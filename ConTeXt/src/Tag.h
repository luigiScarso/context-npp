#pragma once
#ifndef TAG_H
#define TAG_H

#include "StdAfx.h"
#include "StdAfx.h"
//#include "PluginInterface.h"
#include "Scintilla.h"

///This was adapted from the WebEdit plugin. This module deals with tag replacements.
struct Tag
{
	Tag(HWND SCI, HWND hNotepad) : hSci(SCI), hNpp(hNotepad) {}
	bool ValidKey(char *);
	void ShowMsg(const char* );
	void replace();
	bool GetKey();
	void PasteValue(long int* , string &, long int* , long int* );
	long int GetTextRange(long int*, long int*, char*);
	void InsertBuff(long int*, string &, long int*, long int*);
	int InsertEol(long int*);
	void PasteIndent(long int*);
	void PasteChar(long int*, char);
	void Tag::PasteByTab(long int *to);
	void Tag::PasteByPaste(long int *to);
	map<string, string> tags; ///< mapping from key to values
private:
	HWND hSci;
	HWND hNpp;
	const char*	MaxKeyMsg = "Maximum tag length is 32 characters.";
	const char* KeyNotFoundMsg = "Undefined tag: ";
	const char* NoKeyMsg = "No tag here.";
	const char* NoTagsMsg = "No tags defined.";
	long int pos; ///< current caret position
	char key[MaxKeyLen];
	map<string, string>::iterator tag; ///< Store tags
	long int posLeft; ///< the left position of the found key
	long int posRight; ///< the left position of the found key
	long int line;
	long int indentBeg; ///< the position of indentation start
	long int indentEnd; ///< the position of indentation stop
	char msg[KeyNotFoundMsgLen + MaxKeyLen]; ///< error message

	long int indentLen; ///< Really required length.Can be > MaxIndent.
	char indent[MaxIndent + 1];///< Indentation at line start. 

	void Tab() { ::SendMessage(hSci, SCI_TAB, 0, 0); }
	void Paste() { ::SendMessage(hSci, SCI_PASTE, 0, 0); }
};

#endif
