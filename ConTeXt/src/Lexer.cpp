#include "StdAfx.h"
#include "Lexer.h"
#include <iostream>
#include <iomanip>
#include <fstream>

namespace ConTeXt {

ILexer* ConTeXt::LexerFactory()
{
	return new ConTeXt();
}

/// Determine if a character is considered as a ConTeXt operator
inline bool ConTeXt::isOperator(char ch) {
	if (OPERATORS.find(std::string(1, ch)) != OPERATORS.end())
		return true;
	else
		return false;
/*	if (ch >= 0x80 || isalnum(ch)) {
		return false;
	}
	if (ch == '*' || ch == '/' || ch == '-' ||
		ch == '+' || ch == '(' || ch == ')' ||
		ch == '=' || ch == '{' || ch == '}' ||
		ch == '~' || ch == '[' || ch == ']' ||
		ch == ';' || ch == '<' || ch == '>' ||
		ch == '.' ||
		ch == '^' || ch == ':' || ch == '#' ||
		ch == '&' || ch == '|' )
		return true;
	return false;
	*/
}

inline bool ConTeXt::isDigit(char ch) {
	if (NUMBERS.find(std::string(1, ch)) != NUMBERS.end())
		return true;
	else
		return false;
}



/// Once a backslash is met, continue reading the following word
int ConTeXt::ParseTeXCommand(unsigned int pos, Accessor &styler, char *command)
{
	int length = 0;
	char ch = styler.SafeGetCharAt(pos + 1);

	if (ch == ',' || ch == ':' || ch == ';' || ch == '%') {
		command[0] = '\\';
		command[1] = ch;
		command[2] = 0;
		return 1;
	}
	command[length] = '\\';
	length++;
	// find end
	while (isalpha(ch) && !IsADigit(ch) && ch != '_' && ch != '.' && length<100) {
		command[length] = ch;
		length++;
		ch = styler.SafeGetCharAt(pos + length);
	}

	command[length] = '\0';
	length--;
	if (!length) return 0;
	return length + 1;
}

bool ConTeXt::IsTeXCommentLine(int line, Accessor &styler) {
	int pos = styler.LineStart(line);
	int eol_pos = styler.LineStart(line + 1) - 1;

	int startpos = pos;

	while (startpos<eol_pos) {
		char ch = styler[startpos];
		if (ch != '%' && ch != ' ') return false;
		else if (ch == '%') return true;
		startpos++;
	}

	return false;
}

//
// ILexer
//

void SCI_METHOD ConTeXt::Release() {
	delete this;
}

int SCI_METHOD ConTeXt::Version() const {
	return lvOriginal;
}

const char* SCI_METHOD ConTeXt::PropertyNames() {
	return "";
}

int SCI_METHOD ConTeXt::PropertyType(const char* name) {
	return SC_TYPE_BOOLEAN;
}

const char* SCI_METHOD ConTeXt::DescribeProperty(const char* name) {
	return "";
}

int SCI_METHOD ConTeXt::PropertySet(const char* key, const char* val) {
	return -1;
}

const char* SCI_METHOD ConTeXt::DescribeWordListSets() {
	return "";
}

int SCI_METHOD ConTeXt::WordListSet(int n, const char *wl) {
	if (n < _countof(m_WordLists)) {
		WordList wlNew;
		wlNew.Set(wl);
		if (m_WordLists[n] != wlNew) {
			m_WordLists[n].Set(wl);
			return 0;
		}
	}
	return -1;
}

void SCI_METHOD ConTeXt::Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess)
{
	Accessor styler(pAccess, nullptr);

	const WordList& keywords0 = m_WordLists[0];
	const WordList& keywords1 = m_WordLists[1]; //folder in code 2, open
	const WordList& keywords2 = m_WordLists[2]; //folder in code 2, close
	const WordList& keywords3 = m_WordLists[3];
	const WordList& keywords4 = m_WordLists[4];
	const WordList& keywords5 = m_WordLists[5];
	const WordList& keywords6 = m_WordLists[6];
	const WordList& keywords7 = m_WordLists[7];
	const WordList& keywords8 = m_WordLists[8];


	length += startPos;
	styler.StartAt(startPos);
	styler.StartSegment(startPos);

	int state = TS_DEFAULT;
	char* name = nullptr;
	int count = 0;
	int arabic_digit_count = 0;
	int extended_arabic_digit_count = 0;
	int punctuation_001_count = 0;
	int punctuation_002_count = 0;
	int sm_plus_minus_sign_count = 0;
	char buffer[128];
	int i = startPos;
//	int digits = 0;
	for (; i <= length ; ++i)
	{
		// Make ch 0 if at EOF.
		char ch = (i == length) ? '\0' : styler[i];
		// Amount of EOL chars is 2 (\r\n) with the Windows format and 1 (\n) with Unix format.
		int chEOL = (styler[i] == '\n' && styler[i - 1] == '\r') ? 2 : 1;

		switch(state)
		{
		case TS_DEFAULT:
			switch (ch)
			{
			//case EM_EN_DASH_BEGIN:
			//	styler.ColourTo(i - 1, TC_DEFAULT);
			//	state = TS_EM_EN_DASH_OPERATOR;
			//	em_dash_count = 1;
			//	em_dash_count = 1;
			//	break;
			case ARABIC_INDIC_DIGIT_START:
				styler.ColourTo(i - 1, TC_DEFAULT); 
				state = TS_ARABIC_INDIC_NUMBERS;
				arabic_digit_count = 0;
				break;
			case PUNCTUATION_001: // also E_Sm_PLUS_MINUS_SIGN
				styler.ColourTo(i - 1, TC_DEFAULT);
				state = TS_PUNCTUATION_001;
				punctuation_001_count = 0;
				sm_plus_minus_sign_count = 0;
				break;
			case  PUNCTUATION_002_BEGIN:
				styler.ColourTo(i - 1, TC_DEFAULT);
				state = TS_PUNCTUATION_002;
				punctuation_002_count = 0;
				break;
			case EXTENDED_ARABIC_INDIC_DIGIT_START:
				styler.ColourTo(i - 1, TC_DEFAULT);
				//{std::fstream s("C:\\data\\lex.txt", std::ios::app); s << "EXT-ARABIC ch:0x" << std::hex << (unsigned int)((unsigned char)ch) << "; length=" << std::dec << length << std::endl; s.close(); }
				state = TS_EXTENDED_ARABIC_INDIC_NUMBERS;
				extended_arabic_digit_count = 0;
				break;
			case '\0':
			case '\n':
				styler.ColourTo(i, TC_DEFAULT);
				break;

			case '%':
				styler.ColourTo(i-1, TC_DEFAULT);
				state = TS_COMMENT;
				break;

			case '\\':
				styler.ColourTo(i - 1, TC_DEFAULT);
				buffer[0] = '\\';
				count = 1;
				state = TS_KEYWORD;
				break;
				
			default:
				if (isOperator(ch)) {
					styler.ColourTo(i - 1, TC_DEFAULT); 
					state = TS_VALUE_OPERATOR;
				}
				if (isDigit(ch)) {
					styler.ColourTo(i -1, TC_DEFAULT);
					state = TS_VALUE_NUMBER;
				}
			}
			break;

		case TS_COMMENT:
			// Style as comment when EOL (or EOF) is reached
			switch (ch)
			{
			case '\0':
			case '\n':
				state = TS_DEFAULT;
				styler.ColourTo(i - chEOL, TC_COMMENT);
				styler.ColourTo(i, TC_DEFAULT);
			}
			break;

		case TS_KEYWORD:
			// Read max. 32 chars into buffer until the equals sign (or EOF/EOL) is met.
			if (!isalpha(ch)) {
				buffer[count] = '\0';
				
				//Hardcoded TeX and eTeX macros
				//if(TeX_eTeX_Macros.find(buffer) != TeX_eTeX_Macros.end())
				//{
				//	//std::fstream s("C:\\data\\lex.txt", std::ios::app);	s << "TeX/eTeX macro:" << buffer << std::endl;
				//	styler.ColourTo(i - 1, TC_ETEX);
				//}
				if (TeX_Macros.find(buffer) != TeX_Macros.end())
				{
					styler.ColourTo(i - 1, TC_TEX);
				}
				if (eTeX_Macros.find(buffer) != eTeX_Macros.end())
				{
					styler.ColourTo(i - 1, TC_ETEX);
				}

				if (keywords0.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD0);
				else if (keywords1.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD1);
				else if (keywords2.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD2);
				else if (keywords3.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD3);
				else if (keywords4.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD4);
				else if (keywords5.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD5);
				else if (keywords6.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD6);
				else if (keywords7.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD7);
				else if (keywords8.InList(buffer))
					styler.ColourTo(i - 1, TC_KEYWORD8);
				else
					styler.ColourTo(i - 1, TC_DEFAULT);
				--i;
				state = TS_DEFAULT;
			}
			else if (count < 128)
					buffer[count++] = ch;
			else
					state = TS_DEFAULT;
			break;

		
		case TS_VALUE_OPERATOR:
			if (!isOperator(ch) ) {
				styler.ColourTo(i - 1, TC_OPERATOR); 
				--i;
				state = TS_DEFAULT;
			}
			break;

		case TS_VALUE_NUMBER:
			if (!isDigit(ch)) {
				styler.ColourTo(i - 1, TC_NUMBER);
				--i;
				state = TS_DEFAULT;
			}
			break;

		case TS_LINEEND:
			// Apply default style when EOL (or EOF) is reached
			switch (ch)
			{
			case '\0':
			case '\n':
				state = TS_DEFAULT;
				styler.ColourTo(i, TC_DEFAULT);
			}
			break;
		case TS_PUNCTUATION_001:
		{
			bool foundPunct = (SET_PUNCTUATION_001.find(ch) != SET_PUNCTUATION_001.end());
			bool foundSmMathPlusMinus = (SET_Sm_PLUS_MINUS_SIGN.find(ch) != SET_Sm_PLUS_MINUS_SIGN.end());
			if (!foundPunct && !foundSmMathPlusMinus) {
				if (punctuation_001_count == 1) 
				{// found a punctuation char
					styler.ColourTo(i - 1, TC_OPERATOR);
					--i;
					state = TS_DEFAULT;
					punctuation_001_count = 0;
				}
				else if (sm_plus_minus_sign_count == 1)
				{
					styler.ColourTo(i - 1, TC_NUMBER);
					--i;
					state = TS_DEFAULT;
					sm_plus_minus_sign_count = 0;
				}
				else
				{// not a punctuation char
					state = TS_DEFAULT;
					punctuation_001_count = 0;
					sm_plus_minus_sign_count = 0;
				}
			}
			else { // found a byte of a punctuation char or plus_minus_sign
				if (foundPunct)
					punctuation_001_count++;
				else if (foundSmMathPlusMinus)
					sm_plus_minus_sign_count++;
				else
				{
					state = TS_DEFAULT;
					punctuation_001_count = 0;
					sm_plus_minus_sign_count = 0;
				}
			}
		}
			break;

		case TS_PUNCTUATION_002:
			if ((ch == PUNCTUATION_002_MIDDLE_01) || (ch == PUNCTUATION_002_MIDDLE_02)
			 || (ch == PUNCTUATION_002_MIDDLE_03) || (ch == PUNCTUATION_002_MIDDLE_04)) {
				punctuation_002_count++;
			}
			else if ((SET_PUNCTUATION_002.find(ch) != SET_PUNCTUATION_002.end()) && punctuation_002_count==1) {
					unsigned int key = ((unsigned int)((unsigned char)styler[i - 2]) << 16) + ((unsigned int)((unsigned char)styler[i - 1]) << 8) + (unsigned int)((unsigned char)styler[i]);
					if (SET_PUNCTUATION_002_EXCLUDED.find(key)!=SET_PUNCTUATION_002_EXCLUDED.end())
					{
						//{// DEBUG begin
						//	std::time_t result = std::time(nullptr);
						//	std::fstream s("C:\\data\\lex.txt", std::ios::app);
						//	s << result << " found  ch in SET_PUNCTUATION_002" << std::endl;
						//	s << "i:" << i;
						//	s << " styler[i]:0x" << std::hex << (unsigned int)((unsigned char)styler[i]);
						//	s << " 0x" << std::hex << (unsigned int)((unsigned char)styler[i - 1]);
						//	s << " 0x" << std::hex << (unsigned int)((unsigned char)styler[i - 2]);
						//	s << " key: 0x" << std::hex << key;
						//	s << std::endl;
						//	s << " key: 0x" << std::hex << key << "FOUND !";
						//	s << std::endl;
						//}// DEBUG end
						punctuation_002_count=0;
						state = TS_DEFAULT;
					}
					else
					{
						punctuation_002_count++;
					}
			}
			else  if (punctuation_002_count == 2) {
				styler.ColourTo(i - 1, TC_OPERATOR);
				--i;
				punctuation_002_count = 0;
				state = TS_DEFAULT;
			}
			else {
				punctuation_002_count = 0;
				state = TS_DEFAULT;
			}
			break;

		case TS_ARABIC_INDIC_NUMBERS:
			if (!(SET_ARABIC_INDIC_DIGIT.find(ch) != SET_ARABIC_INDIC_DIGIT.end())) {
				if (arabic_digit_count == 1) {// We are at the end of an arabi-indic digit bytestream 
					//{std::fstream s("C:\\data\\lex.txt", std::ios::app); s << "TS_ARABIC not found ch:0x" << std::hex << (unsigned int)((unsigned char)ch) << "; length=" << std::dec << length << std::endl; s.close(); }
					styler.ColourTo(i - 1, TC_NUMBER);
					--i;
					state = TS_DEFAULT;
					arabic_digit_count = 0;
				}
				else // not an arabic-indic digit bytestream
				{
					state = TS_DEFAULT;
					arabic_digit_count = 0;
				}
			} else  // found a byte of arabic digit 
			{
				arabic_digit_count++;
			}
			break;
		
		case TS_EXTENDED_ARABIC_INDIC_NUMBERS:
			if (!(SET_EXTENDED_ARABIC_INDIC_DIGIT.find(ch) != SET_EXTENDED_ARABIC_INDIC_DIGIT.end())) {
				if (extended_arabic_digit_count == 1) {
					styler.ColourTo(i - 1, TC_NUMBER);
					--i;
					state = TS_DEFAULT;
					extended_arabic_digit_count = 0;
				} else // not an extended arabic digit
				{
					state = TS_DEFAULT;
					extended_arabic_digit_count = 0;
				}
			} else // found an extended digit 
			{
				extended_arabic_digit_count++;
			}
			break;

		}
	}
	styler.Flush();
}

void SCI_METHOD ConTeXt::Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess)
{
	
	Accessor styler(pAccess, nullptr);

	const WordList& keywords1 = m_WordLists[7]; //folder in code 2, open
	const WordList& keywords2 = m_WordLists[8]; //folder in code 2, close

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	char buffer[101] = "";

	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (ch == '%')
		{
			i = styler.LineStart(lineCurrent + 1) - 1;
			++visibleChars;
			atEOL = TRUE;
			ch = '\n';
			chNext = styler.SafeGetCharAt(i + 1);
		}

		if (ch == '\\') {
			ParseTeXCommand(i, styler, buffer);
			if (keywords1.InList(buffer))
				levelCurrent++; 
			else if (keywords2.InList(buffer))
				levelCurrent--;
		}

		bool foldComment = TRUE;

		if (foldComment && atEOL && IsTeXCommentLine(lineCurrent, styler))
		{
			if (lineCurrent == 0 && IsTeXCommentLine(lineCurrent + 1, styler)
				)
				levelCurrent++;
			else if (lineCurrent != 0 && !IsTeXCommentLine(lineCurrent - 1, styler)
				&& IsTeXCommentLine(lineCurrent + 1, styler)
				)
				levelCurrent++;
			else if (lineCurrent != 0 && IsTeXCommentLine(lineCurrent - 1, styler) &&
				!IsTeXCommentLine(lineCurrent + 1, styler))
				levelCurrent--;
		}
		
		//---------------------------------------------------------------------------------------------

		if (atEOL) {
			int lev = levelPrev;
			if ((levelCurrent > levelPrev) && (visibleChars > 0))
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			visibleChars = 0;
		}

		if (isalnum(ch))
			visibleChars++;
	}

	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);

}

void* SCI_METHOD ConTeXt::PrivateCall(int operation, void* pointer) {
	return nullptr;
}

//
// Scintilla exports
//

int SCI_METHOD GetLexerCount()
{
	return 1;
}

void SCI_METHOD GetLexerName(unsigned int index, char* name, int buflength)
{
	strncpy(name, "ConTeXt", buflength);
	name[buflength - 1] = '\0';
}

void SCI_METHOD GetLexerStatusText(unsigned int index, WCHAR* desc, int buflength)
{
	//wcsncpy(desc, L"ConTeXt skin file", buflength);
	wcsncpy(desc, L"ConTeXt file", buflength);
	desc[buflength - 1] = L'\0';
}

LexerFactoryFunction SCI_METHOD GetLexerFactory(unsigned int index)
{
	return (index == 0) ? ConTeXt::LexerFactory : nullptr;
}

}	// namespace ConTeXt
