#include <algorithm>
#include <locale>
#include <shlwapi.h>
#include "AutoCompletion.h"
#include "Notepad_plus_msgs.h"
#include "Scintilla.h"
#include <cwctype>
#include <algorithm>
#include <deque>
#include <cmath>
#include <sstream>


//Debug defau
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <iostream>



using namespace std;

static bool isInList(generic_string word, const vector<generic_string> & wordArray)
{
	for (size_t i = 0, len = wordArray.size(); i < len; ++i)
		if (wordArray[i] == word)
			return true;
	return false;
}

static bool isAllDigits(const generic_string &str)
{
	return std::all_of(str.begin(), str.end(), ::isdigit);
}

AutoCompletion* AutoCompletion::s_autoc = NULL;

AutoCompletion::~AutoCompletion() {
	if (_pXmlFile)
		delete _pXmlFile;
	if (_pXmlConfig)
		delete _pXmlConfig;
	if (calltipFontName)
		delete[] calltipFontName;
};

AutoCompletion* AutoCompletion::create(HWND npp, HWND sci)
{
	if (s_autoc == NULL)
		s_autoc = new AutoCompletion(npp, sci);
	return s_autoc;
}

LRESULT AutoCompletion::execute(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
	return _pScintillaFunc(_pScintillaPtr, Msg, wParam, lParam);
}

/// Find the arguments of a macro
bool AutoCompletion::findParam(const TCHAR* keyTarget)
{
	// key never seen before, parse the APIs/context.xml files for Overload/Param elements
	//generic_string keyTarget = key;
	//const TCHAR* keyTarget = key;
	bool res = false;
	TiXmlNode *autocompleteNode;
	TiXmlNode *pNode = _pXmlFile->FirstChild(TEXT("NotepadPlus"));
	if (!pNode)
		return false;
	pNode = pNode->FirstChildElement(TEXT("AutoComplete"));
	if (!pNode)
		return false;
	autocompleteNode = pNode;
	pNode = pNode->FirstChildElement(TEXT("KeyWord"));
	if (!pNode)
		return false;
	_pXmlKeyword = reinterpret_cast<TiXmlElement *>(pNode);
	if (!_pXmlKeyword)
		return false;
	TiXmlElement *funcNode = _pXmlKeyword;
	for (; funcNode; funcNode = funcNode->NextSiblingElement(TEXT("KeyWord")))
	{
		generic_string keyName = funcNode->Attribute(TEXT("ctxname"));
		if (keyName == keyTarget)
		{
			TiXmlElement *overloadNode = funcNode->FirstChildElement(TEXT("Overload"));
			TiXmlElement *paramNode = NULL;
			//_overload[key] = {};
			res = true; // Found a key, eventually empty
			_overload[keyTarget] = {};
			if (overloadNode)
			{
				const TCHAR *paramSets = overloadNode->Attribute(TEXT("paramsets"));
				if (paramSets)
				{
					unsigned long size = wcstol(paramSets, NULL, 10);
					for (unsigned i = 0; i < size; i++)
						_overload[keyTarget].push_back({});
				}
			}
			for (; overloadNode; overloadNode = overloadNode->NextSiblingElement(TEXT("Overload")))
			{
				paramNode = overloadNode->FirstChildElement(TEXT("Param"));
				for (; paramNode; paramNode = paramNode->NextSiblingElement(TEXT("Param")))
				{
					const TCHAR *paramValue = paramNode->Attribute(TEXT("ctxname"));
					const TCHAR *paramSet = paramNode->Attribute(TEXT("paramset"));
					if (paramValue && paramSet) {
						unsigned long j = wcstol(paramSet, NULL, 10);
						//_overload[keyTarget].push_back(paramValue);
						if ((j - 1) < _overload[keyTarget].size())
							_overload[keyTarget][j - 1].push_back(paramValue);
						else
							res = false; // something wrong
					}
					else
					{
						res = false; // something wrong
					}
				}
			}
			// 	0 == _overload[keyTarget].size() means that the key exists without paramName
			//  (macro without args) 
			if (sortMacroValues)
				for (auto& v : _overload[keyTarget])
					std::sort(v.begin(), v.end());
		}
	}
	return res;
}

/// Retrieve text from Scintilla, from start position to end position
void AutoCompletion::getGenericText(TCHAR *dest, size_t destlen, size_t start, size_t end)
{
	char *destA = new char[end - start + 1];

	TextRange tr;
	tr.chrg.cpMin = static_cast<long>(start);
	tr.chrg.cpMax = static_cast<long>(end);
	tr.lpstrText = destA;
	::SendMessage(hSci, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));

	UINT cp = static_cast<UINT>(::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
	int lenWc = MultiByteToWideChar(cp, 0, destA, -1, NULL, 0);
	TCHAR* destW = new TCHAR[lenWc];
	MultiByteToWideChar(cp, 0, destA, -1, destW, lenWc);

	_tcsncpy_s(dest, destlen, destW, _TRUNCATE);
	delete[] destW;
	delete[] destA;
}

/// Check if calltip visible
bool AutoCompletion::isVisibleCalltip()
{
	return hSci ? execute(SCI_CALLTIPACTIVE) == TRUE : false;

};

/// Take a TCHAR array and display it using scintilla autocompletion
void AutoCompletion::showAutoComletion(size_t lenEntered, const TCHAR* list)
{
	UINT codepage = static_cast<UINT>(::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
	int lenMbcs = WideCharToMultiByte(codepage, 0, list, -1, NULL, 0, NULL, NULL);
	char* dest = new char[lenMbcs];
	WideCharToMultiByte(codepage, 0, list, -1, dest, lenMbcs, NULL, NULL);
	::SendMessage(hSci, SCI_AUTOCSHOW, lenEntered, reinterpret_cast<LPARAM>(dest));
	delete[] dest;
}

/// Find the target text using scintilla, return the inex
int AutoCompletion::searchInTarget(const TCHAR * text2Find, size_t lenOfText2Find, size_t fromPos, size_t toPos)
{
	::SendMessage(hSci, SCI_SETTARGETRANGE, fromPos, toPos);

	UINT cp = static_cast<UINT>(::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
	int lenMbcs = WideCharToMultiByte(cp, 0, text2Find, -1, NULL, 0, NULL, NULL);
	char* text2FindA = new char[lenMbcs];
	WideCharToMultiByte(cp, 0, text2Find, -1, text2FindA, lenMbcs, NULL, NULL);
	size_t text2FindALen = strlen(text2FindA);
	size_t len = (lenOfText2Find > text2FindALen) ? lenOfText2Find : text2FindALen;

	int posFind = static_cast<int32_t>(::SendMessage(hSci, SCI_SEARCHINTARGET, len, reinterpret_cast<LPARAM>(text2FindA)));
	delete[] text2FindA;
	return posFind;
}

/// Parameter display from list 
bool AutoCompletion::showFunctionComplete(int character, const char* keyword)
{

	UINT codepage = static_cast<UINT>(::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
	int lenWcs = MultiByteToWideChar(codepage, 0, keyword, -1, NULL, 0);
	TCHAR* key = new TCHAR[lenWcs];
	MultiByteToWideChar(codepage, 0, keyword, -1, key, lenWcs);
	bool res = true;
	auto val = _overload.find(key);
	if (val == _overload.end()) // key *not* found
	{
		res = findParam(key);
	}
	::SendMessage(hSci, SCI_CALLTIPCANCEL, NULL, NULL);
	// The arguments of the macro <key> are now stored into a vector;
	// if the vector has size 0 then the macro <key> has no arguments;
	// if _overload has no <key> then the macro is not a standard one (perhaps a user defined one)
	// res can be false, signaling that something goes wrong	
	val = _overload.find(key);
	latestKeyWord.clear();
	if (res && val->second.size())
	{
		for (auto &v : val->second)
			if (v.size()) // assign a key iff exists a not-empty  set of paramenters
			{
				latestKeyWord.assign(key);
				latestKeyWordNrParams = val->second.size();
				latestKeyWordParamSetCompleted = 0;
				break;
			}
	}
	delete[] key;
	return true;
}
/// Show function completion
bool AutoCompletion::showApiComplete()
{

	if (!_funcCompletionActive)
		return false;

	// calculate entered word's length
	int curPos = int(::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0));
	int startPos = int(::SendMessage(hSci, SCI_WORDSTARTPOSITION, curPos, true));

	if (curPos == startPos)
		return false;

	size_t len = (curPos > startPos) ? (curPos - startPos) : (startPos - curPos);
	if (len >= _keyWordMaxLen)
		return false;

	::SendMessage(hSci, SCI_AUTOCSETSEPARATOR, WPARAM(' '), 0);
	::SendMessage(hSci, SCI_AUTOCSETIGNORECASE, _ignoreCase, 0);
	showAutoComletion(curPos - startPos, _keyWords.c_str());

	return true;
}


/// Show function and word completion
bool AutoCompletion::showApiAndWordComplete()
{
	// calculate entered word's length
	int curPos = int(::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0));
	int startPos = int(::SendMessage(hSci, SCI_WORDSTARTPOSITION, curPos, true));

	if (curPos == startPos)
		return false;

	const size_t bufSize = 256;
	TCHAR beginChars[bufSize];

	size_t len = (curPos > startPos) ? (curPos - startPos) : (startPos - curPos);
	if (len >= bufSize)
		return false;

	// Get word array
	// set the text into beginChars
	vector<generic_string> wordArray;
	getGenericText(beginChars, bufSize, startPos, curPos);

	// only one macro:
	// in streams like \foo\boo
	// only \boo is selected
	{
		generic_string s = getlastmacro(beginChars);
		if (s.size())
		{
			s.copy(beginChars, s.size());
			beginChars[s.size()] = L'\0';
		}
	}

	getWordArray(wordArray, beginChars);
		
	if (beginChars[0] = startMacro)
	{
		len = std::wcslen(beginChars);
	}
	if (len > 1)
	{
		bool canStop = false;
		for (size_t i = 0, kwlen = _keyWordArray.size(); i < kwlen; ++i)
		{
			if (_keyWordArray[i].compare(0, len, beginChars) == 0)
			{
				if (!isInList(_keyWordArray[i], wordArray))
					wordArray.push_back(_keyWordArray[i]);
				canStop = true;
			}
			else if (canStop) {
				// Early out since no more strings will match
				break;
			}
		}
	}
	std::sort(wordArray.begin(), wordArray.end());

	// Get word list
	generic_string words;

	for (size_t i = 0, wordArrayLen = wordArray.size(); i < wordArrayLen; ++i)
	{
		words += wordArray[i];
		if (i != wordArrayLen - 1)
			words += TEXT(" ");
	}

	::SendMessage(hSci, SCI_AUTOCSETSEPARATOR, WPARAM(' '), 0);
	::SendMessage(hSci, SCI_AUTOCSETIGNORECASE, _ignoreCase, 0);
	//showAutoComletion(curPos - startPos, words.c_str());
	showAutoComletion(len, words.c_str());
	return true;
}

/// Extract matched words from editor
void AutoCompletion::getWordArray(vector<generic_string> & wordArray, TCHAR *beginChars)
{
	const size_t bufSize = 256;
	bool startWithBackslash = (beginChars[0] == '\\');

	if (autocIgnoreNumbers && isAllDigits(beginChars))
		return;

	generic_string expr(TEXT(""));
	//The regex expression needs to be modified for ConTeXt lexers
	if (startWithBackslash)
		expr += TEXT("\\");
	else
		//expr += TEXT("(?<!\\\\)"); // find word start, look behind to rule out a backslash
		//expr += TEXT("[ %\\{\\}\\t\\n\\r.,;:\"()=<>\'+!\\[\\]]");
		expr += TEXT("\\<");
	expr += beginChars;
	expr += TEXT("[^ \\\\%\\{\\}\\t\\n\\r.,;:\"()=<>'+!\\[\\]]+"); // word ends with non-alphanumerical char

	int docLength = ::SendMessage(hSci, SCI_GETLENGTH, 0, 0);

	int flags = SCFIND_WORDSTART | SCFIND_MATCHCASE | SCFIND_REGEXP | SCFIND_POSIX;
	//int flags = SCFIND_WORDSTART | SCFIND_MATCHCASE | SCFIND_REGEXP | SCFIND_CXX11REGEX;
	::SendMessage(hSci, SCI_SETSEARCHFLAGS, flags, 0);

	int posFind = searchInTarget(expr.c_str(), int(expr.length()), 0, docLength);

	while (posFind != -1 && posFind != -2)
	{
		int wordStart = int(::SendMessage(hSci, SCI_GETTARGETSTART, 0, 0));
		int wordEnd = int(::SendMessage(hSci, SCI_GETTARGETEND, 0, 0));
		int charBack = static_cast<char>(::SendMessage(hSci, SCI_GETCHARAT, wordStart - 1, 0));

		//If word-to-search does not start with backslash
		//Only process if the character behind the found word is not a backslash
		//Regex does not provide searching behind, so have to do this way
		if (!startWithBackslash && charBack != 92) //92 is backslash
		{
			size_t foundTextLen = wordEnd - wordStart;
			if (foundTextLen < bufSize)
			{
				TCHAR w[bufSize];
				getGenericText(w, bufSize, wordStart, wordEnd);

				if (!isInList(w, wordArray))
					wordArray.push_back(w);
			}
		}
		posFind = searchInTarget(expr.c_str(), static_cast<int32_t>(expr.length()), wordEnd, docLength);
	}
}

/// Show autocompletion for word completion
bool AutoCompletion::showWordComplete(bool autoInsert)
{
	// calculate entered word's length
	int curPos = int(::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0));
	int startPos = int(::SendMessage(hSci, SCI_WORDSTARTPOSITION, curPos, true));

	if (curPos == startPos)
		return false;

	const size_t bufSize = 256;
	TCHAR beginChars[bufSize];

	size_t len = (curPos > startPos) ? (curPos - startPos) : (startPos - curPos);
	if (len >= bufSize)
		return false;

	// Get word array
	vector<generic_string> wordArray;
	getGenericText(beginChars, bufSize, startPos, curPos);

	getWordArray(wordArray, beginChars);

	if (wordArray.size() == 0) return false;

	std::sort(wordArray.begin(), wordArray.end());

	// Get word list
	generic_string words(TEXT(""));

	for (size_t i = 0, wordArrayLen = wordArray.size(); i < wordArrayLen; ++i)
	{
		words += wordArray[i];
		if (i != wordArrayLen - 1)
			words += TEXT(" ");
	}

	::SendMessage(hSci, SCI_AUTOCSETSEPARATOR, WPARAM(' '), 0);
	::SendMessage(hSci, SCI_AUTOCSETIGNORECASE, _ignoreCase, 0);
	showAutoComletion(curPos - startPos, words.c_str());
	return true;
}


/// Manage a SCN_CHARADDED event
void AutoCompletion::update(int character)
{

	if (!character)
	{
		return;
	}

	if (!_funcCompletionActive && autocStatus == CompletionFunc)
	{
		return;
	}

	//If autocomplete already active, let Scintilla handle it
	if (::SendMessage(hSci, SCI_AUTOCACTIVE, 0, 0) != 0)
	{
		return;
	}

	updateCalltip(character, latestKeyWord.size() > 0);

	if (isVisibleCalltip())
	{
		return;
	}

	/// get word to current position
	const int wordSize = 256; // hm  why ?
	TCHAR s[wordSize];

	size_t caretPos = ::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);
	size_t startPos = ::SendMessage(hSci, SCI_WORDSTARTPOSITION, caretPos, true);

	s[0] = L'\0';
	if ((caretPos - startPos) < wordSize)
		getGenericText(s, wordSize, startPos, caretPos);

	// s can have more than 1 macro
	// i.e. \foo\goo
	// show* function have to manage this
	if (lstrlen(s) >= autocFromLen)
	{
		if (autocStatus == CompletionWord)
			showWordComplete(false);
		else if (autocStatus == CompletionFunc)
			showApiComplete();
		else if (autocStatus == CompletionBoth)
			showApiAndWordComplete();
	}
}

/// Show a calltip 
int  AutoCompletion::updateCalltip(int ch, bool needShown)
{
	int res = 0;
	if ((ch == startFuncs[0]) && needShown)
	{
		int startPos = static_cast<int32_t>(execute(SCI_GETCURRENTPOS));
		UINT codepage = static_cast<UINT>(execute(SCI_GETCODEPAGE));
		auto paramArray = _overload.find(latestKeyWord);
		unsigned int paramArraySize = paramArray->second.size();
		latestKeyWordNrParams = paramArraySize;
		tab.clear();
		tab.insert(0, tabSpacesParameters, ' ');

		for (auto &val : paramArray->second)
		{
			// Append macro paramenters to latestKeyWord
			// val is an array of parameters (strings)	
			latestKeyWord.push_back(ch);
			unsigned int lineLength;
			if (macroValueOnSingleLine)
			{
				size_t val_size = val.size();
				for (unsigned j = 0; j < val_size; j++)
				{
					lineLength = val[j].size() + 1;
					if (lineLength <= maxLineLength)
					{
						latestKeyWord.append(val[j]);
						if (val[j].find(L"=") == generic_string::npos)
						{   // = not found
							if (val_size < 10)
							{
								latestKeyWord.push_back(spaceSeparator);
							}
							else
							{
								latestKeyWord.push_back(L'\n');
							}

						}
						else
						{
							latestKeyWord.push_back(L'\n');
						}
					}
					else
					{
						generic_string search_str(L"|");
						size_t pos = 0;
						unsigned int curline = 0;
						unsigned int startpos = 0;
						unsigned int prevpos = 0;
						while (curline++ < val[j].length())
						{
							pos = val[j].find_first_of(search_str, pos);
							if (pos == generic_string::npos)
								break;
							if ((pos - startpos) > maxLineLength)
							{
								latestKeyWord.append(val[j].substr(startpos, prevpos - startpos));
								latestKeyWord.push_back('\n');
								startpos = prevpos;
								prevpos = pos++;
							}
							else
							{
								prevpos = pos;
								pos++;
							}
						}
						// the rest of the text, eventually
						pos = val[j].find_first_of(search_str, startpos + 1);
						if (pos < generic_string::npos)
						{
							latestKeyWord.append(val[j].substr(startpos, pos - startpos));
							startpos = pos++;
						}
						if (startpos < val[j].size())
						{
							val[j].substr(startpos, val[j].size() - startpos);
						}
						latestKeyWord.push_back('\n');
					}
				}
				//remove last \n	
				latestKeyWord.erase(latestKeyWord.end() - 1, latestKeyWord.end());
				latestKeyWord.push_back(stopFuncs[0]);
				latestKeyWord.push_back('\n');

			}
			else {
				lineLength = 2 * (val.size() - 1); // count ',' and ' ' , but not the last ones  
				for (auto &vv : val)
					lineLength += vv.size();
				if (lineLength <= maxLineLength)
				{// One row
					for (unsigned int i = 0; i < val.size() - 1; i++)
					{
						latestKeyWord.append(val[i]);
						latestKeyWord.push_back(',');
						latestKeyWord.push_back(' ');
					}
					latestKeyWord.append(val[val.size() - 1]);
				}
				else
				{// More rows
					lineLength = 0;
					for (unsigned j = 0; j < val.size(); j++)
					{
						lineLength += val[j].size();
						if (lineLength <= maxLineLength)
						{
							latestKeyWord.append(val[j]);
							latestKeyWord.push_back(paramSeparator);
							latestKeyWord.push_back(spaceSeparator);
							lineLength += 2;
						}
						else
						{ // Insert a newline, then re-start
							//latestKeyWord.append(L"◄");
							latestKeyWord.push_back('\n');
							latestKeyWord.append(tab);
							lineLength = tab.size();
							if ((tab.size() + val[j].size()) < maxLineLength)
							{
								latestKeyWord.append(val[j]);
								lineLength += val[j].size();
								latestKeyWord.push_back(paramSeparator);
								latestKeyWord.push_back(spaceSeparator);
								lineLength += 2;
							}
							else
							{// length of line > maxLineLength, try to split it at "|" 
								TCHAR* buffer;
								// This also works, but looks ugly
								//std::wstring _input(val[j].c_str()) ;
								//std::wstring::pointer input = const_cast<TCHAR*>(_input.c_str());
								TCHAR* input = new TCHAR[val[j].size() + 1];
								for (unsigned i = 0; i < val[j].size() + 1; i++)
									input[i] = val[j][i];
								input[val[j].size()] = 0;
								TCHAR* token = std::wcstok(input, L"|", &buffer);
								while (token) {
									if (lineLength < maxLineLength)
									{
										latestKeyWord.append(token);
										lineLength += std::wcslen(token);
										latestKeyWord.append(L"|");
										lineLength++;
									}
									else
									{
										latestKeyWord.push_back('\n');
										latestKeyWord.append(tab);
										lineLength = tab.size();
										latestKeyWord.append(tab);
										lineLength += tab.size();
										latestKeyWord.append(token);
										lineLength += std::wcslen(token);
										latestKeyWord.append(L"|"); lineLength++;
									}
									token = std::wcstok(NULL, L"|", &buffer);
								}
								delete[] input;
								latestKeyWord.erase(latestKeyWord.end() - 1, latestKeyWord.end());
								latestKeyWord.push_back(paramSeparator);
								latestKeyWord.push_back(spaceSeparator);
								lineLength += 2;
							}
						}
					}
					latestKeyWord.erase(latestKeyWord.end() - 1, latestKeyWord.end());
					latestKeyWord.push_back('\n');
				}
				latestKeyWord.push_back(stopFuncs[0]);
				latestKeyWord.push_back('\n');
			}
		}
		if (columns == 2)
		{

			unsigned int rows = 0;
			size_t pos = 0;
			unsigned int rowlen = 0;
			unsigned int buflen = 0;
			bool colBalanced = true;
			std::deque<size_t> leftColumn;
			std::deque<size_t> rightColumn;

			// Nr. of \n in the text
			while (buflen++ < latestKeyWord.length())
			{
				pos = latestKeyWord.find(L'\n', pos);
				if (pos == generic_string::npos)
					break;
				else
					leftColumn.push_back(pos);
				pos++;
			}
			rows = leftColumn.size();
			// Two columns only if enough rows 
			if (rows > thresholdNrOfRows)
			{
				unsigned int lh, rh;     // left & rigth column heights
				unsigned int prevpos;    // previous position of \'n' in left column
				unsigned int prevpos_rc;// previous position of \'n' in right column
				lh = static_cast<unsigned int>(std::round((static_cast<double>(rows)) / 2.0)); //always lh>=rh
				if (!colBalanced)
				{
					if (thresholdNrOfRows > (rows / 2))
						lh = thresholdNrOfRows;
				}
				rh = rows - lh;
				for (unsigned int c = 1; c <= rh; c++)
				{
					rightColumn.push_front(leftColumn.back());
					leftColumn.pop_back();
				}
				prevpos = 0;
				rowlen = 0;
				for (unsigned int c = 0; c < leftColumn.size(); c++)
				{
					if (c > 0)
					{
						prevpos = leftColumn[c - 1] + 1;
					}
					rowlen = rowlen < (leftColumn[c] - prevpos) ? (leftColumn[c] - prevpos) : rowlen;
				}
				rowlen += widthColumnSep; // min 2 spaces
				prevpos = 0;
				prevpos_rc = leftColumn.back() + 1;
				std::basic_stringstream<WCHAR> bufl;
				for (unsigned int c = 0; c < leftColumn.size(); c++)
				{
					if (c > 0)
					{
						prevpos = leftColumn[c - 1] + 1;
						if (c < rightColumn.size())
							prevpos_rc = rightColumn[c - 1] + 1;
					}
					if (c < rightColumn.size())
						bufl << latestKeyWord.substr(prevpos, leftColumn[c] - prevpos);
					else
						bufl << latestKeyWord.substr(prevpos, leftColumn[c] - prevpos + 1);
					if (rowlen > (leftColumn[c] - prevpos) && c < rightColumn.size())
					{
						generic_string pad(rowlen - (leftColumn[c] - prevpos), ' ');
						bufl << pad;
					}
					if (c < rightColumn.size())
						bufl << latestKeyWord.substr(prevpos_rc, rightColumn[c] - prevpos_rc + 1);
				}
				latestKeyWord.clear();
				latestKeyWord.append(bufl.str());

			}

		}

		// insert toplines '\n' at top
		latestKeyWord.insert(0, toplines, L'\n');

		int lenMbcs = WideCharToMultiByte(codepage, 0, latestKeyWord.c_str(), -1, NULL, 0, NULL, NULL);

		if ((lenMbcs > 0) && static_cast<unsigned int>(lenMbcs) > ctTextSize)
		{
			if (ctText)
			{
				delete ctText;
			}
			ctTextSize = lenMbcs;
			ctText = new char[lenMbcs];
		}
		WideCharToMultiByte(codepage, 0, latestKeyWord.c_str(), -1, ctText, lenMbcs, NULL, NULL);
		execute(SCI_CALLTIPUSESTYLE, 0, 0);
		execute(SCI_STYLESETFONT, STYLE_CALLTIP, reinterpret_cast<LPARAM>(calltipFontName));
		execute(SCI_STYLESETSIZE, STYLE_CALLTIP, calltipSizeInPoint);
		execute(SCI_STYLESETFORE, STYLE_CALLTIP, calltipForeColor);
		execute(SCI_STYLESETBACK, STYLE_CALLTIP, calltipBackColor);
		execute(SCI_CALLTIPSHOW, startPos, reinterpret_cast<LPARAM>(ctText));


		latestKeyWord.clear();
	}
	else if (ch == stopFuncs[0])
	{
		latestKeyWordParamSetCompleted++;
		if (latestKeyWordParamSetCompleted >= latestKeyWordNrParams)
		{
			execute(SCI_CALLTIPCANCEL);
			latestKeyWord.clear();
			res = 0;
		}
	}
	else if (ch == startMacro)
	{
		execute(SCI_CALLTIPCANCEL);
		latestKeyWord.clear();
		res = startMacro;
	}
	return res;
}


/// Read API commands (for function completion) from ConTeXt.xml
bool AutoCompletion::readApi()
{
	TCHAR path[MAX_PATH];
	::GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	lstrcat(path, TEXT("\\plugins\\APIs\\ConTeXt.xml"));


	// start & stopFuncts setup
	for (int i = 0; i < 256; i++)
	{
		startFuncs[i] = -1;
		stopFuncs[i] = -1;
	}

	// calltip management
	// Direct access
	_pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(hSci, SCI_GETDIRECTFUNCTION, 0, 0);
	_pScintillaPtr = (SCINTILLA_PTR)::SendMessage(hSci, SCI_GETDIRECTPOINTER, 0, 0);
	if (!_pScintillaFunc)
	{
		throw std::runtime_error("ScintillaEditView::init : SCI_GETDIRECTFUNCTION message failed");
	}
	if (!_pScintillaPtr)
	{
		throw std::runtime_error("ScintillaEditView::init : SCI_GETDIRECTPOINTER message failed");
	}

	if (_pXmlFile)
		delete _pXmlFile;

	_pXmlFile = new TiXmlDocument(path);
	_funcCompletionActive = _pXmlFile->LoadFile();

	TiXmlNode * pAutoNode = NULL;
	if (_funcCompletionActive) {
		_funcCompletionActive = false;	//safety
		TiXmlNode * pNode = _pXmlFile->FirstChild(TEXT("NotepadPlus"));
		if (!pNode)
			return false;
		pAutoNode = pNode = pNode->FirstChildElement(TEXT("AutoComplete"));
		if (!pNode)
			return false;
		// "Environment" need to be parsed as well, done later 	
		pNode = pNode->FirstChildElement(TEXT("KeyWord"));
		if (!pNode)
			return false;
		_pXmlKeyword = reinterpret_cast<TiXmlElement *>(pNode);
		if (!_pXmlKeyword)
			return false;
		_funcCompletionActive = true;
	}

	_keyWords.clear();
	_keyWordArray.clear();

	if (_funcCompletionActive)
	{
		//Cache the keywords
		//Iterate through all keywords
		TiXmlElement *funcNode = _pXmlKeyword;

		for (; funcNode; funcNode = funcNode->NextSiblingElement(TEXT("KeyWord")))
		{
			const TCHAR *name = funcNode->Attribute(TEXT("ctxname"));
			if (name)
			{
				size_t len = lstrlen(name);
				if (len)
				{
					_keyWordArray.push_back(name);
					if (len > _keyWordMaxLen)
						_keyWordMaxLen = len;
				}
			}
		}

		std::sort(_keyWordArray.begin(), _keyWordArray.end());

		for (size_t i = 0, len = _keyWordArray.size(); i < len; ++i)
		{
			_keyWords.append(_keyWordArray[i]);
			_keyWords.append(TEXT(" "));
		}
	}

	// attributes startFunc[i] and stopFunc[i]
	if (pAutoNode)
	{
		TiXmlElement *envNode = pAutoNode->FirstChildElement(TEXT("Environment"));
		const TCHAR *paramValue;
		paramValue = envNode->Attribute(TEXT("startFunc1"));
		if (paramValue) startFuncs[0] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("startFunc2"));
		if (paramValue) startFuncs[1] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("startFunc3"));
		if (paramValue) startFuncs[3] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("startFunc4"));
		if (paramValue) startFuncs[4] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("stopFunc1"));
		if (paramValue) stopFuncs[0] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("stopFunc2"));
		if (paramValue) stopFuncs[1] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("stopFunc3"));
		if (paramValue) stopFuncs[3] = static_cast<int>(paramValue[0]);
		paramValue = envNode->Attribute(TEXT("stopFunc4"));
		if (paramValue) stopFuncs[4] = static_cast<int>(paramValue[0]);
		// calltip Style
		//calltipFontName = envNode->Attribute(TEXT("calltipFontname"));
		paramValue = envNode->Attribute(TEXT("calltipFontName"));
		if (paramValue)
		{
			UINT codepage = static_cast<UINT>(execute(SCI_GETCODEPAGE));
			int lenMbcs = WideCharToMultiByte(codepage, 0, paramValue, -1, NULL, 0, NULL, NULL);
			calltipFontName = new char[lenMbcs];
			WideCharToMultiByte(codepage, 0, paramValue, -1, calltipFontName, lenMbcs, NULL, NULL);
		}
		paramValue = envNode->Attribute(TEXT("calltipFontSize"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val != 0) && (val < LONG_MAX) && (val > LONG_MIN))
				calltipSizeInPoint = static_cast<int>(val);
		}
		paramValue = envNode->Attribute(TEXT("calltipForeColor"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				calltipForeColor = static_cast<int>(val);
		}
		paramValue = envNode->Attribute(TEXT("calltipBackColor"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				calltipBackColor = static_cast<int>(val);
		}
		paramValue = envNode->Attribute(TEXT("sortMacroValues"));
		if (paramValue)
		{
			generic_string v(paramValue);
			bool res = false;
			std::transform(v.begin(), v.end(), v.begin(), ::tolower);
			if (v.compare(L"yes") == 0)
				res = true;
			sortMacroValues = res;
		}
		paramValue = envNode->Attribute(TEXT("maxLineLength"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				maxLineLength = static_cast<int>(val);
			if (maxLineLength < 0)
				maxLineLength = 80;
		}
		paramValue = envNode->Attribute(TEXT("toplines"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				toplines = static_cast<int>(val);
			if (toplines < 0)
				toplines = 0;
		}
		paramValue = envNode->Attribute(TEXT("macroValueOnSingleLine"));
		if (paramValue)
		{
			generic_string v(paramValue);
			bool res = false;
			std::transform(v.begin(), v.end(), v.begin(), ::tolower);
			if (v.compare(L"yes") == 0)
				res = true;
			macroValueOnSingleLine = res;
		}
		paramValue = envNode->Attribute(TEXT("columns"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				columns = static_cast<unsigned int>(val);;
			if ((columns < 0) || (columns > 2))
				columns = 1;
		}
		paramValue = envNode->Attribute(TEXT("widthColumnSep"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				widthColumnSep = static_cast<unsigned int>(val);;
			if (widthColumnSep < 0)
				widthColumnSep = 2;
		}
		paramValue = envNode->Attribute(TEXT("thresholdNrOfRows"));
		if (paramValue)
		{
			long val = std::wcstol(paramValue, NULL, 0);
			if ((val < LONG_MAX) && (val > LONG_MIN))
				thresholdNrOfRows = static_cast<unsigned int>(val);;
			if (thresholdNrOfRows < 0)
				thresholdNrOfRows = 20;
		}
	}


	if (!calltipFontName)
	{
		char *tmp = nullptr;
		std::size_t len;
		execute(SCI_STYLEGETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>(tmp));
		if (tmp)
		{
			len = std::strlen(tmp);
			if (len)
			{
				calltipFontName = new char[len + 1];
				std::strcpy(calltipFontName, tmp);
				calltipFontName[len] = '\000';
			}
		}


	}
	calltipSizeInPoint = calltipSizeInPoint < 1 ? 8 : calltipSizeInPoint;
	if (calltipForeColor < 0)
		calltipForeColor = execute(SCI_STYLEGETFORE, STYLE_DEFAULT);
	if (calltipBackColor < 0)
		calltipBackColor = execute(SCI_STYLEGETBACK, STYLE_DEFAULT);
	//
	ctText = new char[ctTextSize];

	return _funcCompletionActive;
}

/// This function reads the NPP config file, however it is not used in this plugin
/// You may want to create your own config xml and read it because the NPP one is changed
/// with default auto-completion funciton. If you turn off auto-completion in NPP and read config file
/// from NPP, this auto-completion feature won't work. 
bool AutoCompletion::readConfig()
{
	//find config.xml
	//search appdata first
	TCHAR* config_path = new TCHAR[MAX_PATH];

	TCHAR tmpDir[MAX_PATH];
	::SendMessage(hNpp, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(tmpDir));

	TCHAR* pdest = _tcsstr(tmpDir, _T("plugins"));
	int pre_n = (int)(pdest - tmpDir);
	_tcsncpy(config_path, tmpDir, pre_n);
	config_path[pre_n] = _T('\0');
	_tcscat_s(config_path, MAX_PATH, _T("config.xml\0"));

	if (!PathFileExists(config_path))
	{
		::SendMessage(hNpp, NPPM_GETNPPDIRECTORY, MAX_PATH, reinterpret_cast<LPARAM>(config_path));
		_tcscat_s(config_path, MAX_PATH, _T("\\"));
		_tcscat_s(config_path, MAX_PATH, _T("config.xml"));
	}

	if (_pXmlConfig)
		delete _pXmlConfig;

	_pXmlConfig = new TiXmlDocument(config_path);
	delete[] config_path;
	bool config_read = _pXmlConfig->LoadFile();

	TiXmlNode * pAutoNode = NULL;
	if (config_read) {
		TiXmlNode * pNode = _pXmlConfig->FirstChild(TEXT("NotepadPlus"));
		if (!pNode)
			return false;
		pAutoNode = pNode = pNode->FirstChildElement(TEXT("GUIConfigs"));
		if (!pNode)
			return false;
		pNode = pNode->FirstChildElement(TEXT("GUIConfig"));
		if (!pNode)
			return false;
		_pXmlKeyword = reinterpret_cast<TiXmlElement *>(pNode);

		if (!_pXmlKeyword)
			return false;

		// Iterate nodes to find auto-completion entry
		TiXmlElement *funcNode = _pXmlKeyword;

		for (; funcNode; funcNode = funcNode->NextSiblingElement(TEXT("GUIConfig")))
		{
			const TCHAR *name = funcNode->Attribute(TEXT("name"));
			if (_tcscmp(name, _T("auto-completion")) == 0)
			{
				autocStatus = _ttoi(funcNode->Attribute(_T("autoCAction")));
				autocFromLen = _ttoi(funcNode->Attribute(_T("triggerFromNbChar")));
				const TCHAR *ignore = funcNode->Attribute(_T("autoCIgnoreNumbers"));
				if (_tcscmp(ignore, _T("no")) == 0)
					autocIgnoreNumbers = false;

				return true;
			}
		}
	}
	return false;
}


/// split a stream of text into macros, return the last one
generic_string AutoCompletion::getlastmacro(const TCHAR* t)
{   //TODO: \\ macro
	generic_string s;
	if (t)
	{
		s.append(t);
		auto first = s.find(startMacro);
		auto last = s.rfind(startMacro);
		if (first != last)
		{
			return s.substr(last, s.size() - last);
		}
	}
	return s;
}
