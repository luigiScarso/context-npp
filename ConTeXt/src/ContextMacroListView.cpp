//Debug
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <iostream>
#include <string>

#pragma comment (lib, "comctl32")
#include "ContextMacroListView.h"

const TCHAR ContextMacroListView::cClassName[] = TEXT("ContextMacroListView");
ContextMacroListView* ContextMacroListView::CMLVW = NULL;
HINSTANCE    ContextMacroListView::HMod1 = (HINSTANCE)GetModuleHandle(TEXT("ConTeXt.dll"));

ContextMacroListView::ContextMacroListView()
{
	UserMacroIndexSelected = -1;
	loademptymacro();
	backgdText = 0x00fafafa;
	shortcut_currentState = 0;
	shortcut_currentstring.clear();
}

ContextMacroListView::ContextMacroListView(ContextMacro* ctxmacro)
{
	UserMacroIndexSelected = -1;
	backgdText = 0x00fafafa;
	shortcut_currentState = 0;
	shortcut_currentstring.clear();
	LoadMacro(ctxmacro);
}

ContextMacroListView::~ContextMacroListView()
{
	UserMacro.clear();
}

BOOL ContextMacroListView::LoadMacro(ContextMacro* ctxmacro)
{
	UserMacroIndexSelected = -1;
	UserMacro.clear();

	UserMacro.reserve(ctxmacro->size());
	UserMacro.insert(UserMacro.begin(),
		ctxmacro->size(),
		{ TEXT(""),TEXT(""),TEXT(""), TEXT("") });

	for (size_t i = 0; i < ctxmacro->size(); i++)
	{
		UserMacro[i] = (*ctxmacro)[i];
	}
	makeshortcutprefix();
	return TRUE;
}

BOOL ContextMacroListView::SetSetupValue(std::string Key, std::string Value)
{
	//
	bool res = false;
	if ((Key == "um:elapse") || (Key == "usermacro:elapse"))
	{
		int v = std::atoi(Value.c_str());
		if ((v < 0) || (v > elapse_limit))
		{
			res = false;
		}
		else
		{
			res = true;
			elapse = v;
		}
	}
	else if ((Key == "um:elapse_shift") || (Key == "usermacro:elapse_shift"))
	{
		int v = std::atoi(Value.c_str());
		if ((v < 0) || (v > elapse_limit))
		{
			res = false;
		}
		else
		{
			res = true;
			elapse_shift = v;
		}
	}
	else if ((Key == "um:display_rows") || (Key == "usermacro:display_rows"))
	{
		int v = std::atoi(Value.c_str());
		if ((v < 0) || (v > DisplayMaxRowsLimit))
		{
			res = false;
		}
		else
		{
			res = true;
			DisplayMaxRows = v;
		}
	}

	return res;
}

BOOL ContextMacroListView::SetBackgroundColor(COLORREF bgcolor)
{
	backgdText = bgcolor;
	return TRUE;
}

void ContextMacroListView::Register()
{
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.hInstance = HMod1;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = cClassName;

	if (CMLVW)
		wc.hbrBackground = GetSysColorBrush(CMLVW->backgdText);

	RegisterClass(&wc);

	INITCOMMONCONTROLSEX icex = { 0 };
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;

	InitCommonControlsEx(&icex);
}

void ContextMacroListView::UnRegister()
{
	UnregisterClass(cClassName, HMod1);
}

ContextMacroListView* ContextMacroListView::CreateStaticInstance()
{
	if (CMLVW)
	{
		//Only one instance of  CMLVW;
		DestroyWindow(CMLVW->hwndCurrentListView);
		DestroyWindow(CMLVW->hwndCurrentParent);
		ContextMacroListView::UnRegister();
	}

	CMLVW = new ContextMacroListView();
	CMLVW->hCurrentInstance = HMod1;
	return CMLVW;
}

bool ContextMacroListView::DestroyStaticInstance()
{
	if (CMLVW)
	{
		//Only one instance of  CMLVW;
		DestroyWindow(CMLVW->hwndCurrentListView);
		DestroyWindow(CMLVW->hwndCurrentParent);
		ContextMacroListView::UnRegister();
		delete CMLVW;
		CMLVW = NULL;
		return true;
	}
	return false;
}

HWND ContextMacroListView::ComposeAndShowWindow(const TCHAR* header, HWND hSci)
{

	if (!CMLVW)
		return NULL;
	RECT win;
	GetWindowRect(hSci, &win);
	int X = 100;
	int Y = 100;
	// Point Size for style 0 , usually default
	int pad = SendMessage(hSci, SCI_STYLEGETSIZE, 0, 0);
	long currPos = SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);
	X = SendMessage(hSci, SCI_POINTXFROMPOSITION, 0, currPos);
	Y = SendMessage(hSci, SCI_POINTYFROMPOSITION, 0, currPos);
	pad = (pad > 0 ? pad : 20);

	LONG xpos, ypos;
	xpos = win.left + X + pad;
	ypos = win.top + Y + pad;

	HWND _hWnd = CreateWindow(cClassName,
		TEXT("User's Macros"),
		WS_POPUPWINDOW | WS_CAPTION, // | WS_SIZEBOX , //| WS_HSCROLL | WS_VSCROLL,
		0, 0, //xpos, ypos, later
		0, 0,
		hSci, NULL, HMod1, NULL);
	if (_hWnd == NULL)
		return NULL;

	//HWND _hWndListView =  CMLVW->CreateListView(HMod1,_hWnd);
	if (CMLVW)
	{
		CMLVW->hwndCurrentParentParent = hSci;
		if (CMLVW->hwndCurrentListView)
		{
			RECT lv;
			GetWindowRect(CMLVW->hwndCurrentListView, &lv);
			LONG lv_width = lv.right - lv.left;
			LONG lv_height = lv.bottom - lv.top;
			if ((xpos + lv_width) > win.right)
			{
				xpos = win.left + X - lv_width - pad;

			}
			if ((ypos + lv_height) > win.bottom)
			{
				ypos = win.top + Y - lv_height;
				if (ypos < win.top)
				{
					ypos = win.top + Y - lv_height + (win.top - ypos);
				}
			}
			SetWindowPos(_hWnd, NULL,
				xpos, ypos,
				0, 0,//ignored due SWP_NOSIZE
				SWP_DRAWFRAME | SWP_SHOWWINDOW | SWP_NOSIZE);
		}


	}
	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	return _hWnd;
}

HWND ContextMacroListView::CreateListView(HINSTANCE hInstance, HWND hwndParent)
{
	DWORD       dwStyle;
	DWORD       dwStyleEx;
	HWND        hwndListView;

	dwStyle = WS_TABSTOP |
		WS_CHILD |
		WS_BORDER |
		WS_VISIBLE |
		LVS_AUTOARRANGE |
		LVS_REPORT | LVS_OWNERDATA;

	hwndListView = CreateWindowEx(WS_EX_CLIENTEDGE,    // ex style
		WC_LISTVIEW,         // class name - defined in commctrl.h
		TEXT(""),            // dummy text
		dwStyle,             // style
		0,                   // x position
		0,                   // y position
		0,                   // width
		0,                   // height
		hwndParent,          // parent
		(HMENU)ID_LISTVIEW,  // ID
		hInstance,           // instance
		NULL);               // no extra data

	hwndCurrentParent = hwndParent;
	hwndCurrentListView = hwndListView;
	hCurrentInstance = hInstance;

	if (!hwndListView)
		return NULL;

	dwStyleEx = LVS_EX_AUTOSIZECOLUMNS |
		LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyleEx(hwndListView, dwStyleEx, dwStyleEx);
	ListView_SetBkColor(hwndListView, backgdText);
	ListView_SetTextBkColor(hwndListView, backgdText);
	//ResizeListView(hwndListView, hwndParent);
	UserMacroIndexSelected = 0;
	return hwndListView;

}

/******************************************************************************

   PositionHeader

   this needs to be called when the ListView is created, resized, the view is
   changed or a WM_SYSPARAMETERCHANGE message is received

******************************************************************************/
void ContextMacroListView::PositionHeader(HWND hwndListView)
{
	HWND  hwndHeader = GetWindow(hwndListView, GW_CHILD);
	DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);

	/*To ensure that the first item will be visible, create the control without
	the LVS_NOSCROLL style and then add it here*/
	dwStyle |= LVS_NOSCROLL;
	SetWindowLong(hwndListView, GWL_STYLE, dwStyle);

	//only do this if we are in report view and were able to get the header hWnd
	if (((dwStyle & LVS_TYPEMASK) == LVS_REPORT) && hwndHeader)
	{
		RECT        rc;
		HD_LAYOUT   hdLayout;
		WINDOWPOS   wpos;

		GetClientRect(hwndListView, &rc);
		hdLayout.prc = &rc;
		hdLayout.pwpos = &wpos;

		Header_Layout(hwndHeader, &hdLayout);

		SetWindowPos(hwndHeader,
			wpos.hwndInsertAfter,
			wpos.x,
			wpos.y,
			wpos.cx,
			wpos.cy,
			wpos.flags | SWP_SHOWWINDOW);

		ListView_EnsureVisible(hwndListView, 20, FALSE);
	}
}

BOOL ContextMacroListView::InitListView(HWND hwndListView)
{
	LVCOLUMN   lvColumn;
	int         i;
	std::vector<int> wc = { ListView_GetStringWidth(hwndListView,ColumnLabels[0]),
			  ListView_GetStringWidth(hwndListView,ColumnLabels[1]) };
	unsigned int pad = 0;

	//empty the list
	ListView_DeleteAllItems(hwndListView);

	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	pad = ListView_GetStringWidth(hwndListView, L"MM");
	for (i = 0; i < COLUMNNR; i++)
	{
		lvColumn.cx = getMaxItemWidth(hwndListView, i);
		lvColumn.cx = (lvColumn.cx < wc[i]) ? wc[i] : getMaxItemWidth(hwndListView, i);
		lvColumn.cx += pad;
		lvColumn.pszText = ColumnLabels[i];
		ListView_InsertColumn(hwndListView, i, &lvColumn);
	}
	InsertListViewItems(hwndListView);
	UserMacroIndexSelected = 0;
	// select first item
	ListView_SetItemState(hwndListView, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	return TRUE;

}

BOOL ContextMacroListView::InsertListViewItems(HWND hwndListView)
{
	//empty the list
	ListView_DeleteAllItems(hwndListView);
	//set the number of items in the list
	ListView_SetItemCount(hwndListView, UserMacro.size());
	return TRUE;

}

void ContextMacroListView::ResizeListView(HWND hwndListView, HWND hwndParent)
{
	int pad;
	double H = 0;
	double W = 0;
	RECT  rc;
	BOOL res;
	RECT  rclw;
	for (size_t i = 0; i < UserMacro.size(); i++)
	{
		res = ListView_GetItemRect(hwndListView, i, &rclw, LVIR_BOUNDS);
		if (res)
		{
			H += ((rclw.bottom - rclw.top)*1.3);
			W = (W < (rclw.right - rclw.left)) ? (rclw.right - rclw.left) : W;
		}
	}
	pad = ListView_GetStringWidth(hwndListView, L"M");
	if (W == 0)
	{
		W = getMaxWidth(hwndListView);
		H = UserMacro.size() * 2 * pad;
	}

	H = (H > (DisplayMaxRows*pad)) ? (DisplayMaxRows*pad) : H;

	H += (2 * pad);
	W += (4 * pad);

	H = std::ceil(H);

	GetClientRect(hwndParent, &rc);
	MoveWindow(hwndListView,
		rc.left,
		rc.top,
		static_cast<int>(W), static_cast<int>(H),
		TRUE);
	//only call this if we want the LVS_NOSCROLL style
	//PositionHeader(hwndListView);
	//UpdateWindow(hwndListView);

	//ListView_EnsureVisible(hwndListView, UserMacro.size() / 2, FALSE);
	SetWindowPos(hwndParent, NULL,
		0, 0, //ignored due SWP_NOMOVE
		static_cast<int>(W), 25 + static_cast<int>(H), // still not ok, not a multiple of a row height
		SWP_DRAWFRAME | SWP_SHOWWINDOW | SWP_NOMOVE);
}

LRESULT ContextMacroListView::ListViewNotify(HWND hWnd, LPARAM lParam)
{
	LPNMHDR  lpnmh = reinterpret_cast <LPNMHDR>(lParam);
	//HWND     hwndListView = GetDlgItem(hWnd, ID_LISTVIEW);
	HWND hwndListView = hwndCurrentListView;
	NMLVDISPINFO* plvdi;


	switch (lpnmh->code)
	{

	case LVN_GETDISPINFO:
	{
		plvdi = (NMLVDISPINFO*)lParam;
		switch (plvdi->item.iSubItem)
		{
		case 0: //First Column
		case 1: //Second Column   
		{
			if ((0 < plvdi->item.cchTextMax) && (plvdi->item.cchTextMax <= MAX_PATH))
			{
				generic_string s = UserMacro[plvdi->item.iItem][plvdi->item.iSubItem];
				s.push_back(0); // Ensure NULL terminating string
				int l = static_cast<int>(s.size()); //Including NULL
				l = (l > 0) ? (l) : 1; // ensure NULL 
				l = l > (MAX_PATH) ? (MAX_PATH) : l;
				s[l - 1] = 0;
				// Sigh Microsoft doesn't like s.copy
				//s.copy(plvdi->item.pszText, l);
				_tcsncpy_s(plvdi->item.pszText, MAX_PATH,//plvdi->item.cchTextMax,
					s.c_str(), _TRUNCATE);
			}
		}
		break;

		default:
			break;
		}
		break;
	}
	return 0;

	case LVN_KEYDOWN:
	{
		LPNMLVKEYDOWN 	pnkd = reinterpret_cast <LPNMLVKEYDOWN>(lParam);

		if (UserMacroIndexSelected == ((size_t)-1))
			UserMacroIndexSelected = 0;

		// UP & DOWN keys
		if (VK_UP == pnkd->wVKey || VK_DOWN == pnkd->wVKey)
		{
			shortcut_currentstring.clear();
			if (VK_UP == pnkd->wVKey && UserMacroIndexSelected > 0)
				UserMacroIndexSelected--;
			if (VK_DOWN == pnkd->wVKey && UserMacroIndexSelected < (UserMacro.size() - 1))
				UserMacroIndexSelected++;
			return 0;
		}
		// ESCAPE and SPACE exit without selection
		if (VK_ESCAPE == pnkd->wVKey || VK_SPACE == pnkd->wVKey)
		{
			UserMacroIndexSelected = (size_t)-1;
			SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
			return 0;
		}

		// 0-9 A-Z a-z select a possibly multi char item if it exists, then exits
		UINT Elapse = elapse;
		if (((0x30 <= pnkd->wVKey) && (pnkd->wVKey <= 0x39)) || ((0x41 <= pnkd->wVKey) && (pnkd->wVKey <= 0x5A)))
		{
			UserMacroIndexSelected = -1;
			generic_string s;

			if (GetKeyState(VK_SHIFT) >= 0 && ((0x41 <= pnkd->wVKey) && (pnkd->wVKey <= 0x5A)))
			{	//lower case letter
				s.push_back(pnkd->wVKey + 32);
				shortcut_currentstring.push_back(pnkd->wVKey + 32);
			}
			else if (GetKeyState(VK_SHIFT) < 0 && ((0x41 <= pnkd->wVKey) && (pnkd->wVKey <= 0x5A)))
			{   //upper case letter
				Elapse = elapse_shift;
				s.push_back(pnkd->wVKey);
				shortcut_currentstring.push_back(pnkd->wVKey);
			}
			else
			{
				s.push_back(pnkd->wVKey);
				shortcut_currentstring.push_back(pnkd->wVKey);
			}
			size_t i = 0;
			bool found_shortcut = false;
			for (auto v : UserMacro)
			{
				if (v[0] == shortcut_currentstring)//(v[0] == s)
				{
					found_shortcut = true;
					UserMacroIndexSelected = i;
					break;
				}
				i++;
			}
			
			bool is_proper_prefix = (ShortcutPrefixCnt.count(shortcut_currentstring) > 0);
			bool is_improper_prefix = (!is_proper_prefix) && found_shortcut;

			if (shortcut_currentState == 0 && is_improper_prefix)
			{
				// initial state, s is a single char, s is valid Key, s is not a proper prefix:
				//	copy the macro
				getUserMacroSelected();
				SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
				return 0;
			}
			else
			{
				if (shortcut_currentState == 0 && !found_shortcut && (!is_proper_prefix))
				{// it will never be a valid shortcut
				 // kill the timer
					UserMacroIndexSelected = (size_t)-1;
					SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
					//DestroyWindow(hwndListView);
					//DestroyWindow(hwndCurrentParent);
					return 0;
				}
				else if (shortcut_currentState == 0 && is_proper_prefix)
				{// initial state with a proper prefix, waiting the next char
				 //	but also perhaps a valid shortcut:
				 // reset the timer 
					SetTimer(hWnd, CTXMACRO_TIMER_ID, Elapse, NULL);
					shortcut_currentState = 1;
				}
				else if (shortcut_currentState > 0 && found_shortcut && is_proper_prefix)
				{//  another valid shortcut with more than 1 char
					//reset the timer
					KillTimer(hWnd, CTXMACRO_TIMER_ID);
					SetTimer(hWnd, CTXMACRO_TIMER_ID, Elapse, NULL);
					shortcut_currentState++;
				}
				else if (shortcut_currentState > 0 && !found_shortcut && is_proper_prefix)
				{// possible part of a shortcut, go on
				 // reset the timer 
					KillTimer(hWnd, CTXMACRO_TIMER_ID);
					SetTimer(hWnd, CTXMACRO_TIMER_ID, Elapse, NULL);
					shortcut_currentState++;
				}
				else if (shortcut_currentState > 0 && found_shortcut && is_improper_prefix)
				{// seen a multichar Key shortcut
				 // kill the timer
					KillTimer(hWnd, CTXMACRO_TIMER_ID);
					shortcut_currentState = 0;
					getUserMacroSelected();
					SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
					return 0;
				}
				else if (!found_shortcut)
				{// it will never be part of a shortcut
				 // kill the timer
					KillTimer(hWnd, CTXMACRO_TIMER_ID);
					shortcut_currentState = 0;
					shortcut_currentstring.clear();
					UserMacroIndexSelected = (size_t)-1;
					SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
					return 0;
				}
			}
			return 0;
		}

	}
	return 0;
	
	case LVN_ODCACHEHINT:
	{
		/*LPNMLVCACHEHINT   lpCacheHint = (LPNMLVCACHEHINT)lParam;*/
		/*
		  This sample doesn't use this notification, but this is sent when the
		  ListView is about to ask for a range of items. On this notification,
		  you should load the specified items into your local cache. It is still
		  possible to get an LVN_GETDISPINFO for an item that has not been cached,
		  therefore, your application must take into account the chance of this
		  occurring.
		*/
	}
	return 0;

	case LVN_ODFINDITEM:
	{
		/*LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;*/
		/*
		  This sample doesn't use this notification, but this is sent when the
		  ListView needs a particular item. Return -1 if the item is not found.
		*/
	}
	return 0;


	case NM_CUSTOMDRAW:
	{
		LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
		switch (lplvcd->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
		{
			int row = lplvcd->nmcd.dwItemSpec;
			if (ItemClass.count(0) > 0)
			{
				// There is a "First level labels" class (nr 0)
				if (ItemClass[0].count(row) > 0)
				{
					//The row is in "First level labels" class 
					lplvcd->clrText = RGB(255, 255, 255);
					lplvcd->clrTextBk = RGB(0, 0, 0);
					return CDRF_NEWFONT;
				}
			}
			return CDRF_DODEFAULT;
		}
		/* This can be useful to manage a subitem*/
		//case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		//		//SelectObject(lplvcd->nmcd.hdc, GetFontForSubItem(lplvcd->nmcd.dwItemSpec, lplvcd->nmcd.lItemlParam, lplvcd->iSubItem));
		//		//lplvcd->clrText = GetColorForSubItem(lplvcd->nmcd.dwItemSpec, lplvcd->nmcd.lItemlParam, lplvcd->iSubItem));
		//		//lplvcd->clrTextBk = GetBkColorForSubItem(lplvcd->nmcd.dwItemSpec, lplvcd->nmcd.lItemlParam, lplvcd->iSubItem));

		//		// This notification is received only if you are in report mode and
		//		//returned CDRF_NOTIFYSUBITEMDRAW in the previous step. At
		//		//this point, you can change the background colors for the
		//		//subitem and return CDRF_NEWFONT.
		//		
		//		//return CDRF_NEWFONT;
		//default:
			//break;
		}
	}
	return 0;

	case NM_CLICK:
	{
		LPNMITEMACTIVATE pnmitem = (LPNMITEMACTIVATE)lParam;
		UserMacroIndexSelected = pnmitem->iItem;
		getUserMacroSelected();
		SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
		//DestroyWindow(hwndListView);
		//DestroyWindow(hwndCurrentParent);
	}
	return 0;

	case NM_SETFOCUS:
	{

	}
	return 0;

	case NM_RETURN:
	{
		UserMacroIndexSelected = (size_t)(-1);
		int i = getFirstSelected(hwndListView);
		if (i > -1)
		{
			UserMacroIndexSelected = i;
		}
		getUserMacroSelected();
		SendMessage(hwndCurrentParent, WM_CLOSE, NULL, NULL);
	}
	return 0;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Private

LRESULT APIENTRY
ContextMacroListView::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (CMLVW)
	{
		switch (message)
		{
		case WM_CREATE:
		{
			if (CMLVW->hwndCurrentListView)
			{
				SendMessage(CMLVW->hwndCurrentListView, WM_CLOSE, NULL, NULL);
			}
			HWND hwndListView = CMLVW->CreateListView(HMod1, hWnd);
			if (hwndListView)
			{
				CMLVW->InitListView(hwndListView);
				SetFocus(hwndListView);
				CMLVW->ResizeListView(hwndListView, hWnd);
			}
		}
		return 0;

		case WM_TIMER:
		{
			CMLVW->getUserMacroSelected();
			if (CMLVW->hwndCurrentListView)
			{
				SendMessage(CMLVW->hwndCurrentParent, WM_CLOSE, NULL, NULL);
			}
		}
		return 0;

		case WM_SETFOCUS:
			SetFocus(CMLVW->hwndCurrentListView);
			return 0;

		case WM_NOTIFY:
			return CMLVW->ListViewNotify(hWnd, lParam);


		case WM_CLOSE:
			DestroyWindow(CMLVW->hwndCurrentParent);
			break;

		case WM_DESTROY:
			break;

		case WM_NCDESTROY:
		{
			ContextMacroListView::UnRegister();
			delete CMLVW;
			CMLVW = NULL;
		}
		return 0;

		default:
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int
ContextMacroListView::getItemSelectedandFocused(HWND hwndListView)
{
	const int itcnt = ListView_GetItemCount(hwndListView);
	UINT ds = LVIS_SELECTED | LVIS_FOCUSED;
	int res = -1;
	for (int i = 0; i < itcnt; i++)
	{
		if ((ListView_GetItemState(hwndListView, i, ds) & ds) == ds)
		{
			res = i;
			break;
		}
	}
	return res;
}

unsigned int
ContextMacroListView::getHeight(HWND hwndListView, const TCHAR* st = nullptr)
{
	HFONT childFont = GetWindowFont(hwndListView);
	if (!st)
	{
		return 0;
	}
	if (!childFont)
	{
		return 0;
	}
	HDC hdc;
	hdc = GetDC(hwndListView);
	if (!hdc)
	{
		return 0;
	}
	childFont = static_cast<HFONT>(SelectObject(hdc, childFont));
	if (!childFont)
	{
		return 0;
	}
	SIZE s;
	GetTextExtentPoint32(hdc, st, _tcslen(st), &s);
	SelectObject(hdc, childFont);

	return s.cy;
}

unsigned int
ContextMacroListView::getMaxWidth(HWND hwndListView)
{
	int width = 0;
	for (auto v : UserMacro)
	{
		generic_string s;
		for (int column = 0; column < COLUMNNR; column++)
		{
			s.append(v[column]);
			s.append(L"  ");
		}
		s.push_back(0);
		int l = ListView_GetStringWidth(hwndListView, s.c_str());
		width = width > l ? width : l;
	}
	return width;
}

unsigned int
ContextMacroListView::getMaxItemWidth(HWND hwndListView, unsigned int column = 0)
{
	int width = 0;
	for (auto v : UserMacro)
	{
		generic_string s = v[column];
		s.push_back(0);
		int l = ListView_GetStringWidth(hwndListView, s.c_str());
		width = width > l ? width : l;
	}
	return width;
}

generic_string
ContextMacroListView::getUserMacroSelected()
{
	generic_string s;
	if ((0 <= UserMacroIndexSelected) && (UserMacroIndexSelected < UserMacro.size()))
	{
		if (UserMacro[UserMacroIndexSelected].back() == TEXT("1"))
		{
			s = UserMacro[UserMacroIndexSelected][2];
			s.append(UserMacro[UserMacroIndexSelected][3]);
		}
	}
	// To be sure, we kill the timer
	if (hwndCurrentListView)
	{
		KillTimer(hwndCurrentListView, CTXMACRO_TIMER_ID);
	}
	if (CMLVW && CMLVW->hwndCurrentParentParent && (!s.empty()))
	{
		HWND hSci = CMLVW->hwndCurrentParentParent;
		UINT cp = static_cast<UINT>(SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
		int lenMbcs = WideCharToMultiByte(cp, 0, s.c_str(), s.size(), NULL, 0, NULL, NULL);
		if (lenMbcs > 0)
		{
			char* _s = new char[lenMbcs];
			WideCharToMultiByte(cp, 0, s.c_str(), -1, _s, lenMbcs, NULL, NULL);
			auto sstart = SendMessage(hSci, SCI_GETSELECTIONSTART, 0, 0);
			auto send = SendMessage(hSci, SCI_GETSELECTIONEND, 0, 0);
			if (sstart == send)
			{
				SendMessage(hSci, SCI_ADDTEXT, lenMbcs, reinterpret_cast<LPARAM>(_s));
				if (UserMacro[UserMacroIndexSelected][3].size() > 0)
				{
					int pos = SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0) - UserMacro[UserMacroIndexSelected][3].size();
					SendMessage(hSci, SCI_GOTOPOS, pos, 0);

				}
			}
			else
			{
				int e_lenMbcs = WideCharToMultiByte(cp, 0, UserMacro[UserMacroIndexSelected][3].c_str(), UserMacro[UserMacroIndexSelected][3].size(), NULL, 0, NULL, NULL);
				int s_size = SendMessage(hSci, SCI_GETSELTEXT, NULL, NULL) - 1; // drop the final '\0'
				lenMbcs = WideCharToMultiByte(cp, 0, UserMacro[UserMacroIndexSelected][2].c_str(), UserMacro[UserMacroIndexSelected][2].size(), NULL, 0, NULL, NULL);
				int pad = lenMbcs;
				char* _ss = new char[lenMbcs + e_lenMbcs + s_size + 1];
				char* _sstart = _ss;
				char* _ss1 = _ss;
				_ss[lenMbcs + e_lenMbcs + s_size] = '\0';
				WideCharToMultiByte(cp, 0, UserMacro[UserMacroIndexSelected][2].c_str(), -1, _ss, lenMbcs, NULL, NULL);
				_ss += lenMbcs;
				SendMessage(hSci, SCI_GETSELTEXT, NULL, reinterpret_cast<LPARAM>(_ss));
				_ss += (s_size);
				WideCharToMultiByte(cp, 0, UserMacro[UserMacroIndexSelected][3].c_str(), -1, _ss, e_lenMbcs, NULL, NULL);
				SendMessage(hSci, SCI_REPLACESEL, NULL, reinterpret_cast<LPARAM>(_sstart));
				_ss = _sstart;
				delete[] _ss;
				_sstart = NULL;
			}
			delete[] _s;
		}
	}
	UserMacroIndexSelected = (size_t)-1;
	return s;
}

BOOL
ContextMacroListView::loademptymacro()
{
	UserMacroIndexSelected = -1;
	UserMacro.clear();
	/*UserMacro.insert(UserMacro.begin(),20,
			 {TEXT("***"),
			  TEXT("                              "),
			  TEXT("0")});*/
	UserMacro.insert(UserMacro.end(), 1, { TEXT(""), TEXT("Topic 1"),  TEXT(""),TEXT("0") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("0"), TEXT("main text"),TEXT("\\starttext"),TEXT("\\stoptext"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("1"), TEXT("chapter"),TEXT("\\startchapter"),TEXT("\\stopchapter"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("2"), TEXT("section"),TEXT("\\startsection"),TEXT("\\stopsection"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("3"), TEXT("sub.section"),TEXT("\\startsubsection"),TEXT("\\stopsubsection"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("4"), TEXT("sub.sub.section"),TEXT("\\startsubsubsection"),TEXT("\\stopsubsubsection"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("5"), TEXT("sub.sub.sub.section"),TEXT("\\startsubsubsubsection"),TEXT("\\stopsubsubsubsection"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT(""),  TEXT("Topic 2"), TEXT(""), TEXT("0") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("a"), TEXT("itemize"),TEXT("\\startitemize"),TEXT("\\stopitemize"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("b"), TEXT("itemize.n"),TEXT("\\startitemize[n]"),TEXT("\\stopitemize"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("c"), TEXT("item"),TEXT("\\startitem"),TEXT("\\stopitem"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("d"), TEXT("head"),TEXT("\\starthead"),TEXT("\\stophead"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT(""),  TEXT("Topic 3"), TEXT(""), TEXT("0") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("A"), TEXT("framed"),TEXT("\\startframedtext[frame=on]"),TEXT("\\stopframedtext"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("B"), TEXT("inline framed"),TEXT("\\framed{"),TEXT("}"),  TEXT("1") });
	UserMacro.insert(UserMacro.end(), 1, { TEXT("C"), TEXT("typing"),TEXT("\\starttyping"),TEXT("\\stoptyping"),  TEXT("1") });
	makeshortcutprefix();
	return TRUE;
}

int
ContextMacroListView::getFirstSelected(HWND hwndListView)
{
	//HWND hwndListView = hwndLV;
	int selected = -1;
	if (hwndListView)
	{
		for (size_t i = 0; i < UserMacro.size(); i++)
		{
			UINT mask = LVIS_SELECTED;
			if (mask == ListView_GetItemState(hwndListView, i, mask))
			{
				selected = i;
				break;
			}
		}
	}
	return selected;
}

void ContextMacroListView::setItemClass(int itemclass, int itemnr)
{
	auto itemcl = ItemClass.find(itemclass);
	if (itemcl != ItemClass.end())
	{
		// the itemclass exists
		(itemcl->second)[itemnr] = true;
	}
	else
	{
		// the itemclass for this item not found
		std::map< int, bool > newitemcl;
		newitemcl[itemnr] = true;
		ItemClass[itemclass] = newitemcl;
	}
}

std::vector<int>* ContextMacroListView::getItemClass(int itemnr)
{
	std::vector<int>* v = new std::vector<int>;
	for (auto i_cl : ItemClass)
	{
		if ((i_cl.second).count(itemnr))
		{
			v->push_back(i_cl.first);
		}
	}
	return v;
}

void ContextMacroListView::makeshortcutprefix()
{
	for (auto v : UserMacro)
	{
		if (v[0].size() > 1)
		{
			generic_string s = v[0];
			for (size_t l = 1; l < s.size(); l++)
			{
				ShortcutPrefixCnt[s.substr(0, l)] = 1;
			}
		}
	}
}