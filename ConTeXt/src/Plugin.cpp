#include "StdAfx.h"
#include "PluginInterface.h"
#include "Version.h"
#include "Menumanager.h"
#include "ContextMenu.h"
#include "ConTeXtEditU.h"
#include "Tag.h"
#include "Scintilla.h"
#include "AutoCompletion.h"

//Debug 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <iostream>


static FuncItem	*funcItem = NULL;
static NppData nppData;
HWND g_NppWindow = nullptr;
HWND g_SCI = nullptr;
TCHAR lexer_name[MaxLexerNameLen];

int usrmacro_window_width = -1;
int usrmacro_window_height = -1;

void About()
{
	MessageBox(
		g_NppWindow,
		L"This plugin aims to support the editing of ConTeXt documents by integrating the files "
		L"included in the standalone distribution. For more information on ConTeXt see\n"
		L"http ://wiki.contextgarden.net/Main_Page\nand\nhttp://www.pragma-ade.nl/index.htm",
		ConTeXt_TITLE,
		MB_OK);
}

/// Notepad++ exports
BOOL isUnicode()
{
	return TRUE;
}

/// Notepad++ exports, return the name of this plugin
const TCHAR * getName()
{
	return L"&ConTeXt";
}

void replace()
{
	ConTeXtEditU::getInstance()->replaceTag();
}

void insert_ctx_macro()
{
	ConTeXtEditU::getInstance()->InsertCtxMacro();
}

void delete_ctx_macro()
{
	ConTeXtEditU::getInstance()->DeleteCtxMacro();
}


void editConfig()
{
	ConTeXtEditU::getInstance()->EditConfig();
}

void loadConfig()
{
	// Reset stored configuration
	// ContextMacroListView:
	usrmacro_window_width = -1;
	usrmacro_window_height = -1;

	MenuManager::getInstance()->loadConfig();
}

void editConTeXtUserMacro()
{
	ConTeXtEditU::getInstance()->EditConTeXtUserMacro();
}

void loadConTeXtUserMacro()
{
	AutoCompletion::getinstance()->readUserMacro();
}



void updateContextMenu()
{
	MenuManager::getInstance()->updateContextMenu();
	MessageBox(nppData._nppHandle, L"Please restart Notepad++", L"ConTeXt", MB_OK);
}

void removeContextMenu()
{
	MenuManager::getInstance()->removeContextMenu();
	MessageBox(nppData._nppHandle, L"Please restart Notepad++", L"ConTeXt", MB_OK);
}

void editContextXML()
{
	modifyContextMenu mc(nppData._nppHandle);
	mc.editXML();
}

// TODO:Attributes of Environment element of APIs/context.xml
void editEnvAttributes()
{
	//MessageBox(nppData._nppHandle, L"To do: get/set attributes of Env. in APIs/context.xml", L"ConTeXt", MB_OK);
}


static FuncItem* getGeneratedFuncItemArray(int *nbF)
{

	MenuManager* menuManager = MenuManager::getInstance();
	MenuManager::ItemVectorTD items;
	unsigned int Replace_tags_index = 0;
	unsigned int Insert_ctx_macro = 0;
	unsigned int index = 0;

	items.push_back(pair<tstring, void(*)()>(_T("--"), reinterpret_cast<void(*)()>(NULL)));
	index++;
	items.push_back(pair<tstring, void(*)()>(_T("Replace template"), replace));
	Replace_tags_index = index++;

	items.push_back(pair<tstring, void(*)()>(_T("Insert ConTeXt macro"), insert_ctx_macro));
	Insert_ctx_macro = index++;


	items.push_back(pair<tstring, void(*)()>(_T("--"), reinterpret_cast<void(*)()>(NULL)));
	items.push_back(pair<tstring, void(*)()>(_T("Edit config"), editConfig));
	items.push_back(pair<tstring, void(*)()>(_T("Load config"), loadConfig));
	items.push_back(pair<tstring, void(*)()>(_T("--"), reinterpret_cast<void(*)()>(NULL)));
	items.push_back(pair<tstring, void(*)()>(_T("Edit ConTeXt user macros"), editConTeXtUserMacro));
	items.push_back(pair<tstring, void(*)()>(_T("Load ConTeXt user macros"), loadConTeXtUserMacro));
	items.push_back(pair<tstring, void(*)()>(_T("--"), reinterpret_cast<void(*)()>(NULL)));

	items.push_back(pair<tstring, void(*)()>(_T("Update right-click menu"), updateContextMenu));
	items.push_back(pair<tstring, void(*)()>(_T("Remove right-click menu"), removeContextMenu));
	items.push_back(pair<tstring, void(*)()>(_T("Edit right-click menu"), editContextXML));
	// TODO
	//items.push_back(pair<tstring, void(*)()>(_T("Edit Env. attributes"), editEnvAttributes));


	items.push_back(pair<tstring, void(*)()>(_T("--"), reinterpret_cast<void(*)()>(NULL)));
	items.push_back(pair<tstring, void(*)()>(_T("About"), About));

	FuncItem* funcItems = menuManager->getFuncItemArray(nbF, items);

	if (!funcItems[Replace_tags_index]._pShKey)
	{
		funcItems[Replace_tags_index]._pShKey = new ShortcutKey;
		funcItems[Replace_tags_index]._pShKey->_isAlt = true;
		funcItems[Replace_tags_index]._pShKey->_isCtrl = false;
		funcItems[Replace_tags_index]._pShKey->_isShift = false;
		funcItems[Replace_tags_index]._pShKey->_key = VK_OEM_5; /* \\ */  
	}
	if (!funcItems[Insert_ctx_macro]._pShKey)
	{
		funcItems[Insert_ctx_macro]._pShKey = new ShortcutKey;
		funcItems[Insert_ctx_macro]._pShKey->_isAlt = false;
		funcItems[Insert_ctx_macro]._pShKey->_isCtrl = true;
		funcItems[Insert_ctx_macro]._pShKey->_isShift = false;
		funcItems[Insert_ctx_macro]._pShKey->_key = VK_OEM_MINUS; 
	}


	return funcItems;
}


/// Notepad++ exports, return the static function arrays
FuncItem * getFuncsArray(int *count) 
{
	funcItem = getGeneratedFuncItemArray(count);
	return funcItem;
}

/// Notepad++ exports, create several class static instances
void setInfo(NppData data)
{
	nppData = data;
	g_NppWindow = data._nppHandle;
	
	// Get the current scintilla
	// Copied from notepad++ source code
	int which = -1;
	::SendMessage(g_NppWindow, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return;
	g_SCI = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Create instances of ConTeXtEditU and MenuManager
	ConTeXtEditU::create(g_NppWindow, g_SCI);
	MenuManager::create(g_NppWindow);
	AutoCompletion::create(g_NppWindow, g_SCI);
}

/// Get the name of current lexer
void getLexerName()
{
	int langtype;
	::SendMessage(g_NppWindow, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&langtype));
	::SendMessage(g_NppWindow, NPPM_GETLANGUAGENAME, langtype, reinterpret_cast<LPARAM>(lexer_name));
}

///Set Scintilla word characters by adding a backslash if the lexer is ConTeXt
void setWordChar()
{
	auto defaultCharListLen = ::SendMessage(g_SCI, SCI_GETWORDCHARS, 0, 0);
	char *defaultCharList = new char[defaultCharListLen + 2];
	::SendMessage(g_SCI, SCI_GETWORDCHARS, 0, reinterpret_cast<LPARAM>(defaultCharList));
	defaultCharList[defaultCharListLen] = '\\';
	defaultCharList[defaultCharListLen + 1] = '\0';
	::SendMessage(g_SCI, SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(defaultCharList));
	delete[] defaultCharList;
}

//#define CTX_USRMACRO 0xff000000
/// Notepad++ exports, initializes menu when NPP is ready; handles auto-completion
void beNotified(SCNotification *scn) 
{
	switch (scn->nmhdr.code)
	{
		case CTX_USRMACRO:
		{
			HWND ctxusrmacroListView = ::FindWindow(TEXT("ContextMacroListView"), NULL);
			HWND w = ::GetWindow(ctxusrmacroListView, GW_CHILD);
			int ListViewID= ::GetDlgCtrlID(w);
			switch (scn->message)
			{
				case CTX_USRMACRO_SETRESIZE:
				{	
					usrmacro_window_width = static_cast<int>(scn->wParam);
					usrmacro_window_height = static_cast<int>(scn->lParam);
				}
				break;
				case CTX_USRMACRO_GETRESIZE:
				{
					ConTeXtEditU::getInstance()->SetWindowDimenCtxMacro(usrmacro_window_width, usrmacro_window_height);
				}
				break;

				default:
					break;

			}
		}
		break; 
		
		case NPPN_READY:
		{
			MenuManager* menuManager = MenuManager::getInstance();
			menuManager->idsInitialised();
			menuManager->populatePairMenu();
			getLexerName();
			setWordChar();
		}
		break;
		case SCN_PAINTED:
		{
			getLexerName();
		}
		break;

		case SCN_UPDATEUI:
			getLexerName();
			if (_tcscmp(lexer_name, _T("ConTeXt")) == 0)
			{
				// if Context Macro Window is up, kill it
				delete_ctx_macro();
			}
			break;

		case SCN_CHARADDED:
			// Place for in-plugin autocompletion
			getLexerName();
			if (_tcscmp(lexer_name, _T("ConTeXt")) == 0)
			{
				setWordChar();
				AutoCompletion::getinstance()->update(scn->ch);
			}
			break;
		//When the user makes a selection from the list the container is sent a SCN_AUTOCSELECTION notification message.
		case SCN_AUTOCSELECTION:
		{
			getLexerName();
			if (_tcscmp(lexer_name, _T("ConTeXt")) == 0)
			{
				AutoCompletion::getinstance()->showFunctionComplete(scn->ch,scn->text);
			}
		}
		break;
				
		case NPPN_LANGCHANGED:
			getLexerName();
			break;

		case NPPN_SHUTDOWN:
		{
			ConTeXtEditU::deleteInstance();
			MenuManager::deleteInstance();
		}
		break;
				
		/*case SCN_DOUBLECLICK:
		{
		*/	//{//DEBUG
			//	std::time_t result = std::time(nullptr); 
			//	std::fstream s("C:\\data\\lex.txt", std::ios::app); 
			//	s << result << " SCN_DOUBLECLICK event ######################" << std::endl; 
			//	s << "  position:" << scn->position << std::endl;
			//	s << "  modifiers:" << scn->modifiers << std::endl;
			//	s << "  line:" << scn->line << "+1" << std::endl;
			//	s << "  g_SCI:" << static_cast<const void*>(g_SCI) << std::endl;
			//	s << "  g_NppWindow:" << static_cast<const void*>(g_NppWindow) << std::endl;
			//	s.close(); 
			//}//	end DEBUG
			//if (g_NppWindow && g_SCI)
			//{
			//	    /* NPPM_GETCURRENTLINE 	wParam : 0 			lParam : 0
			//		Returns the caret current position 0 - based line
			//		NPPM_GETCURRENTCOLUMN 	wParam : 0 	lParam : 0
			//		Returns the caret current position 0 - based column
			//		*/
			//		int currentLine = ::SendMessage(g_NppWindow, NPPM_GETCURRENTLINE, 0, 0);
			//		int currentColumn = ::SendMessage(g_NppWindow, NPPM_GETCURRENTCOLUMN, 0, 0);
			//		int currentAnchor = ::SendMessage(g_SCI, SCI_GETANCHOR, 0, 0);
			//		int currentSelectionStart = ::SendMessage(g_SCI, SCI_GETSELECTIONSTART, 0, 0);
			//		int currentSelectionEnd = ::SendMessage(g_SCI, SCI_GETSELECTIONEND, 0, 0);
			//		int currentPos = ::SendMessage(g_SCI, SCI_GETSELECTIONEND, 0, 0);
			//		int wordStart = ::SendMessage(g_SCI, SCI_WORDSTARTPOSITION, currentPos, true);
			//		int wordEnd= ::SendMessage(g_SCI, SCI_WORDENDPOSITION, currentPos, true);
			//		int charAt = ::SendMessage(g_SCI, SCI_GETCHARAT , static_cast<int>((wordStart+wordEnd)/2),0);
			//		int lineLength = ::SendMessage(g_SCI, SCI_LINELENGTH, currentLine,0);
			//		int m_wordStart = ::SendMessage(g_SCI,SCI_WORDSTARTPOSITION, lineLength - currentColumn, true);
			//		int m_wordEnd = ::SendMessage(g_SCI, SCI_WORDENDPOSITION, lineLength - currentColumn, true);
			//		{// DEBUG,
			//			std::fstream s("C:\\data\\lex.txt", std::ios::app);
			//			s << "  currentLine:" << currentLine << std::endl;
			//			s << "  line length:" << lineLength << std::endl;
			//			s << "  currentColumn:" << currentColumn << std::endl;
			//			s << "  currentAnchor:" << currentAnchor << std::endl;	
			//			s << "  currentSelectionStart:" << currentSelectionStart << std::endl;
			//			s << "  currentSelectionEnd:" << currentSelectionEnd << std::endl;
			//			s << "  currentPos:" << currentPos << std::endl;
			//			s << "  wordStart:" << wordStart << std::endl;
			//			s << "  wordEnd:" << wordEnd << std::endl;
			//			s << "  m_pos:" << (lineLength - currentColumn) << std::endl;
			//			s << "  m_wordStart:" << m_wordStart << std::endl;
			//			s << "  m_wordEnd:" << m_wordEnd << std::endl;
			//			s << "  byte(s) from wordStart:" ;
			//			for (int i=wordStart; i< wordEnd; i++)
			//			{
			//				charAt = ::SendMessage(g_SCI, SCI_GETCHARAT, static_cast<int>(i), 0);
			//				s <<  std::hex << (0xff & static_cast<unsigned int>(charAt)) << " ";
			//			}
			//			s << std::endl;
			//			/*HWND hWnd = static_cast<HWND>(scn->nmhdr.hwndFrom);
			//			switch (::GetTextAlign(::GetDC(hWnd)))
			//			{
			//			case TA_LEFT:
			//				s << "  TexAlign: left";
			//				::SetTextAlign(::GetDC(hWnd), TA_RTLREADING);
			//				break;
			//			case TA_RIGHT:
			//				s << "  TexAlign: right";
			//				break;
			//			case TA_RTLREADING:
			//				s << "  TexAlign: rtl";
			//				break;
			//			default:
			//				s << "  TexAlign: other";
			//			}
			//			s << std::endl;
			//			*/
			//			/*long exStyle = static_cast<long>(::GetWindowLongPtr(g_SCI, GWL_EXSTYLE));
			//			s << "  WS_EX_LAYOUTRTL:" << ((exStyle & WS_EX_LAYOUTRTL)!=0) << std::endl;
			//			s << "  WS_EX_RTLREADING:" << ((exStyle & WS_EX_RTLREADING) != 0) << std::endl;
			//			if (exStyle & WS_EX_LAYOUTRTL)
			//			{
			//				::SetWindowLongPtr(g_SCI, GWL_EXSTYLE, exStyle | WS_EX_RIGHT);
			//			}
			//			else
			//			{
			//				::SetWindowLongPtr(g_SCI, GWL_EXSTYLE, exStyle&(~WS_EX_RIGHT));
			//			}*/
			//		}// end DEBUG
					/*::SendMessage(g_SCI, SCI_SETEMPTYSELECTION, m_wordStart, 0);
					::SendMessage(g_SCI, SCI_SETSELECTIONSTART, m_wordStart, 0);
					::SendMessage(g_SCI, SCI_SETSELECTIONEND, m_wordEnd, 0);
					*/
					/*::SendMessage(g_SCI, SCI_SETEMPTYSELECTION, m_wordStart, 0);
					::SendMessage(g_SCI, SCI_SETSELECTIONSTART, 54, 0);
					::SendMessage(g_SCI, SCI_SETSELECTIONEND, 71, 0);
					*/
					//::SendMessage(g_SCI, SCI_SETSELECTIONSTART, currentSelectionStart, 0);
				   //::SendMessage(g_SCI, SCI_SETSELECTIONEND, currentSelectionStart, 0);
		   //}
		/*}
		break;*/

	}
}

/// Notepad++ exports, trigged when a user clicks a submenu
LRESULT messageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg)
	{
	case WM_COMMAND:
		// When user clicks a submenu, WM_COMMAND is processed here
		MenuManager::getInstance()->processWmCommand(wParam, lParam);
		break;
	
	default:
		// Other messages can just be ignored
		break;

	}
	return TRUE;
}


