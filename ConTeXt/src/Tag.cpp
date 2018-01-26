#include "Tag.h"



/// Return TRUE if 'key' consists of valid tag keys only.
bool Tag::ValidKey(char* key)
{
	
	long int i = 0;
	while ((i <= MaxKeyLen) && (key[i] != '\0') && isalnum(key[i]))
		++i;
	return (i <= MaxKeyLen) && (key[i] == '\0');
}

void Tag::ShowMsg(const char* msg) {
	::SendMessage(hSci, SCI_CALLTIPSHOW, pos, reinterpret_cast<LPARAM>(msg));
}

/// Get the key around caret from editor
bool Tag::GetKey()
{
	long int stop;
	bool res;
	posLeft = pos;
	stop = pos - MaxKeyLen - 1;
	if (stop < 0)
		stop = 0;
	while ((posLeft > stop) && isalnum(::SendMessage(hSci, SCI_GETCHARAT, posLeft - 1, 0)))
		--posLeft;
	posRight = pos;
	stop = ::SendMessage(hSci, SCI_GETTEXTLENGTH,0,0);
	if (stop > pos + MaxKeyLen + 1)
		stop = pos + MaxKeyLen + 1;
	
	while ((posRight < stop) && isalnum(::SendMessage(hSci, SCI_GETCHARAT, posRight, 0)))
		++posRight;
	res = FALSE;

	if (posRight - posLeft <= 0)
		ShowMsg(NoKeyMsg);
	else if (posRight - posLeft <= MaxKeyLen)
	{
		stop = GetTextRange(&posLeft, &posRight, key);
		res = stop == posRight - posLeft;
		if (res)
			key[stop] = '\0';
	}
	else
		ShowMsg(MaxKeyMsg);
	return res;
}

long int Tag::GetTextRange(long int* posLeft, long int* posRight, char *key)
{
	TextRange tr;
	tr.chrg.cpMin = static_cast<long>(*posLeft);
	tr.chrg.cpMax = static_cast<long>(*posRight);
	tr.lpstrText = key;
	return ::SendMessage(hSci, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

/// Main replace function
void Tag::replace()
{
	//(** Either replace the current tag or jump to the next hotspot. *)
	pos = ::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);

	if (tags.empty())
		ShowMsg(NoTagsMsg);
	else if (GetKey())
	{
		tag = tags.find(std::string(key)); // tag is an iterator

		if (tag != tags.end()) {
			::SendMessage(hSci, SCI_BEGINUNDOACTION, 0, 0);
			line = ::SendMessage(hSci, SCI_LINEFROMPOSITION, posLeft, 0);
			indentBeg = ::SendMessage(hSci, SCI_POSITIONFROMLINE, line, 0);
			indentEnd = ::SendMessage(hSci, SCI_GETLINEINDENTPOSITION, line, 0);
			strcpy(msg, ""); //(*msg is just a temp variable here *)
			::SendMessage(hSci, SCI_SETSEL, posLeft, posRight);
			::SendMessage(hSci, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(msg));

			PasteValue(&posLeft, tag->second, &indentBeg, &indentEnd);
			::SendMessage(hSci, SCI_ENDUNDOACTION, 0, 0);
		}
		else
		{
			strcpy_s(msg, KeyNotFoundMsg);
			strcat_s(msg, key);
			ShowMsg(msg);
		}
	}
}

/*!	Same as InsertText, but only insert text[from..to - 1].
	Pre: 0 <= from <= to < LEN(text) 
	pos: insertion position
	text: the target text that needs to be inserted
	from/to: determine which part of text needs to be inserted. */
void Tag::InsertBuff(long int* pos, std::string & text, long int* from, long int* to)
{
	if (*to - *from > 0)
	{
		char * insertion = new char[*to - *from + 1];
#pragma warning( push )  
#pragma warning(suppress : 4996)
		size_t len = text.copy(insertion, *to - *from, *from);
#pragma warning( pop )  
		insertion[len] = '\0';
		::SendMessage(hSci, SCI_INSERTTEXT, *pos, reinterpret_cast<LPARAM>(insertion));
	}
}


/*! Insert EOL at 'pos' according to the current GetEolMode setting.
	Return the number of characters inserted : 0..2 (0 means error). */
int Tag::InsertEol (long int* pos)
{
	const char* CRLF = "\r\n";
	const char* CR = "\r";
	const char* LF = "\n";
	char eol[3];
	int res;
	switch (::SendMessage(hSci, SCI_GETEOLMODE,0 ,0))
	{
	case SC_EOL_CRLF:
		strcpy(eol, CRLF);
		res = 2;
		break;
	case SC_EOL_CR: 
		strcpy(eol, CR); 
		res = 1;
		break;
	case SC_EOL_LF: 
		strcpy(eol, LF); 
		res = 1;
		break;
	default:
		eol[0] = '\0';
		res = 0;
		break;
	}
	::SendMessage(hSci, SCI_INSERTTEXT, *pos, reinterpret_cast<LPARAM>(eol));
	return  res;
}

/// Paste target text, one character each. Also handles escaped character. 
void Tag::PasteValue(long int* pastePos, std::string& value, long int* indentBeg, long int* indentEnd)
{
	char TabChar = '\t';
	long int i = 0, c = 0; //(*value[c] is the next character to be pasted to 'sci' *)
	long int pos; //(*Current text insertion position. *)
	long int caretPos; //(*Caret position after the operation. *)

	caretPos = -1;
	indentLen = *indentEnd - *indentBeg;
	if (indentLen < MaxIndent)//(*init indent once and for all *)
		indent[GetTextRange(indentBeg, indentEnd, indent)] = '\0'; 

	pos = *pastePos;
	while (i != value.size())
	{
		if (value[i] == '\\' && (value[i+1]=='|' || value[i+1]=='\0') ) //(* paste value [c..i - 1] to 'sci' *)
		{
			InsertBuff(&pos, value, &c, &i);
			pos += i - c;
			++i;
			//(*handle escaped characters *)
			switch (value[i])
			{
			//case 'n'://(*eol *)
			//	pos += InsertEol(&pos);
			//	PasteIndent(&pos);
			//	break;
			//case 'i'://(*indentation *)
			//	PasteByTab(&pos);
			//	break;
			//case '\\':      //(* backslash *)
			//	PasteChar(&pos, '\\');
			//	break;
			//case 't'://(*tab *)
			//	PasteChar(&pos, TabChar);
			//	break;
			case '|'://(*pipe *)
				PasteChar(&pos, '|');
				break;
			//case 'c'://(*clipboard *)
			//	PasteByPaste(&pos);
			//	break;
			case '\0'://(*null ? yes, it can happen *)
				--i; //(*step back to terminate the outer loop *)
				break;
			default:
				break;
			}
			c = i + 1;
		} 
		else if (value[i] == '%' && value[i+1] =='n') // "%n" is '\n', "% n" is "%n" 
		{
			InsertBuff(&pos, value, &c, &i);
			pos += i - c;
			++i;
			pos += InsertEol(&pos);
			PasteIndent(&pos);
			c = i + 1;
		}
		else if (value[i] == '|')
		{
			InsertBuff(&pos, value, &c, &i);
			pos += i - c;
			caretPos = pos;
			c = i + 1;
		}
		++i;
	}
	InsertBuff(&pos, value, &c, &i); //(*c = i is no problem *)

	pos += i - c;
	if (caretPos == -1)
		caretPos = pos;
	::SendMessage(hSci, SCI_GOTOPOS, caretPos, 0);
	//(*move the caret about the target position to avoid invalid caret
	//*placement on subsequent moves with up and down keys(wrong column) *)
	::SendMessage(hSci, SCI_CHARLEFT, 0 ,0);
	::SendMessage(hSci, SCI_CHARRIGHT, 0, 0);
	if (caretPos == 0)
		::SendMessage(hSci, SCI_CHARLEFT, 0, 0);

}

void Tag::PasteChar(long int* to, char ch)
{
	char str[2];
	str[0] = ch;
	str[1] = '\0';
	::SendMessage(hSci, SCI_INSERTTEXT, *to, reinterpret_cast<LPARAM>(str));
	++*to;
}

/*! Paste indentation characters. 'to' is increased by the pasted amount. 
The indentation was kept the same with the indentation from the previous line. */
void Tag::PasteIndent(long int* to)
{
	long int from, endPos;
	if (indentLen < MaxIndent)
	{
		::SendMessage(hSci, SCI_INSERTTEXT, *to, reinterpret_cast<LPARAM>(indent));
		*to += indentLen;
	}
	else
	{
		endPos = *to + indentLen;
		from = indentBeg;
		while (!(indentEnd - from < MaxIndent))
		{
			indent[GetTextRange(&from, &from + MaxIndent, indent)] = '\0';
			::SendMessage(hSci, SCI_INSERTTEXT, *to, reinterpret_cast<LPARAM>(indent));
			from += MaxIndent;
			*to += MaxIndent;
		}
		indent[GetTextRange(&from, &indentEnd, indent)] = '\0';
		::SendMessage(hSci, SCI_INSERTTEXT, *to, reinterpret_cast<LPARAM>(indent));
		*to += indentEnd - from;
	}
}

void Tag::PasteByTab(long int *to)
{
	//(*Paste something using 'cmd'. 'to' is increased by the pasted amount. *)
	long int prevLen;
	::SendMessage(hSci, SCI_SETCURRENTPOS, *to, 0);
	::SendMessage(hSci, SCI_SETANCHOR, *to, 0);
	prevLen = ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0);
	Tab();
	to += ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0) - prevLen;
}

void Tag::PasteByPaste(long int *to)
{
	//(*Paste something using 'cmd'. 'to' is increased by the pasted amount. *)
	long int prevLen;
	::SendMessage(hSci, SCI_SETCURRENTPOS, *to, 0);
	::SendMessage(hSci, SCI_SETANCHOR, *to, 0);
	prevLen = ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0);
	Paste();
	to += ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0) - prevLen;
}