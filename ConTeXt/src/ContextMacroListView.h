#ifndef HEADER_ContextMacroListView
#define HEADER_ContextMacroListView


#include <array>
#include <ctime>
#include <iostream>
#include <vector>
#include <map>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h> 


#include "Scintilla.h"
#include "ConTeXtEditU.h"

enum enum_COLUMNNR { COLUMNNR = 2, MAXCOLUMNNR = 5 };
typedef std::basic_string<TCHAR> generic_string;
typedef std::vector<std::array<generic_string, MAXCOLUMNNR>> ContextMacro;



class ContextMacroListView
{
public:
	ContextMacroListView();
	ContextMacroListView(ContextMacro* ctxmacro);
	~ContextMacroListView();

	static ContextMacroListView* CreateStaticInstance();
	static ContextMacroListView* getInstance();
	static void Register();
	static void UnRegister();
	static HWND ComposeAndShowWindow(const TCHAR* header, HWND hOwner);
	static bool DestroyStaticInstance();

	BOOL LoadMacro(ContextMacro* ctxmacro);
	BOOL SetSetupValue(std::string Key, std::string Value);
	BOOL SetBackgroundColor(COLORREF bgcolor);



	HWND CreateListView(HINSTANCE hInstance, HWND hwndParent);
	void ResizeListView(HWND hwndListView, HWND hwndParent);
	void NotifyListViewChangeTo(HWND notifier);
	void PositionHeader(HWND hwndListView);
	BOOL InitListView(HWND hwndListView);
	BOOL InsertListViewItems(HWND hwndListView);
	LRESULT ListViewNotify(HWND hWnd, LPARAM lParam);


	generic_string getUserMacroSelected();

	void setItemClass(int itemclass, int itemnr);
	std::vector<int>* getItemClass(int itemnr);
	void setDisplayMaxRows(int r) { DisplayMaxRows = std::abs(r); }
	int getDisplayMaxRows() { return DisplayMaxRows; }
	void setWindowDimensions(int w, int h) { window_width = w; window_height = h; }
	void getWindowDimensions(int *w, int *h) { *w = window_width; *h = window_height; }
	int getNrOfColumns()  { return NrOfColumns; }
	UINT getTimerElapse() { return elapse; }
	UINT setTimerElapse(UINT newval) { UINT oldval = elapse;  elapse = newval;  return oldval; }

private:
	int NrOfColumns = MAXCOLUMNNR;
	ContextMacro  UserMacro;
	std::map< generic_string, UINT > ShortcutPrefixCnt;
	int shortcut_currentState = 0;
	generic_string shortcut_currentstring;
	bool parent_resizing = false;
	int  window_width = -1;
	int  window_height = -1;
	int  window_width_ini = -1;
	int  window_height_ini = -1;

#define CTXMACRO_TIMER_ID 10001
	UINT elapse=250;
	UINT elapse_shift = 300;
	UINT elapse_limit = 100*1000; // 100 seconds
	// std::vector<std::array<generic_string, MAXCOLUMNNR>> UserMacro;
	std::map< int, std::map< int, bool > > ItemClass;
	generic_string const KeyRange =
		TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	size_t UserMacroIndexSelected = -1;
	int DisplayMaxRows = 40;
	int DisplayMaxRowsLimit = 400;

	static const TCHAR  cClassName[];
	static ContextMacroListView* CMLVW;
	static LRESULT APIENTRY wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static HINSTANCE    HMod1;

	COLORREF backgdText = 0x00fafafa; 
	TCHAR  ColumnLabels[COLUMNNR][20] = { TEXT("Key"),TEXT("User Macro") };

#define ID_LISTVIEW 2001
	HWND hwndCurrentParentParent;
	HWND hwndCurrentParent;
	HWND hwndCurrentListView;
	HWND hwndNotifier;
	HINSTANCE hCurrentInstance;
	int controlID;

	int getItemSelectedandFocused(HWND hwndListView);
	unsigned int getHeight(HWND hwndListView, const TCHAR* st);
	unsigned int getMaxWidth(HWND hwndListView);
	unsigned int getMaxItemWidth(HWND hwndListView, unsigned int column);
	
	void sendNotification(SCNotification scn);
	void makeshortcutprefix();
	BOOL loademptymacro();
	int	getFirstSelected(HWND hwndLV);


/*
#define WM_CAP_START WM_USER
#define WM_CAP_UNICODE_START WM_USER+100
#define WM_CAP_PAL_SAVEW WM_CAP_UNICODE_START+81
#define WM_CAP_UNICODE_END WM_CAP_PAL_SAVEW 
	unsigned long long DBG_WINMSG_CNT = 0;
	std::map<size_t, std::string> DBG_WINMSG = {
		{ (WM_CAP_START + 1), "WM_CAP_GET_CAPSTREAMPTR" },
	{ (WM_CAP_START + 10), "WM_CAP_DRIVER_CONNECT" },
	{ (WM_CAP_START + 11), "WM_CAP_DRIVER_DISCONNECT" },
	{ (WM_CAP_START + 12), "WM_CAP_DRIVER_GET_NAMEA" },
	{ (WM_CAP_START + 13), "WM_CAP_DRIVER_GET_VERSIONA" },
	{ (WM_CAP_START + 14), "WM_CAP_DRIVER_GET_CAPS" },
	{ (WM_CAP_START + 2), "WM_CAP_SET_CALLBACK_ERRORA" },
	{ (WM_CAP_START + 20), "WM_CAP_FILE_SET_CAPTURE_FILEA" },
	{ (WM_CAP_START + 21), "WM_CAP_FILE_GET_CAPTURE_FILEA" },
	{ (WM_CAP_START + 22), "WM_CAP_FILE_ALLOCATE" },
	{ (WM_CAP_START + 23), "WM_CAP_FILE_SAVEASA" },
	{ (WM_CAP_START + 24), "WM_CAP_FILE_SET_INFOCHUNK" },
	{ (WM_CAP_START + 25), "WM_CAP_FILE_SAVEDIBA" },
	{ (WM_CAP_START + 3), "WM_CAP_SET_CALLBACK_STATUSA" },
	{ (WM_CAP_START + 30), "WM_CAP_EDIT_COPY" },
	{ (WM_CAP_START + 35), "WM_CAP_SET_AUDIOFORMAT" },
	{ (WM_CAP_START + 36), "WM_CAP_GET_AUDIOFORMAT" },
	{ (WM_CAP_START + 4), "WM_CAP_SET_CALLBACK_YIELD" },
	{ (WM_CAP_START + 41), "WM_CAP_DLG_VIDEOFORMAT" },
	{ (WM_CAP_START + 42), "WM_CAP_DLG_VIDEOSOURCE" },
	{ (WM_CAP_START + 43), "WM_CAP_DLG_VIDEODISPLAY" },
	{ (WM_CAP_START + 44), "WM_CAP_GET_VIDEOFORMAT" },
	{ (WM_CAP_START + 45), "WM_CAP_SET_VIDEOFORMAT" },
	{ (WM_CAP_START + 46), "WM_CAP_DLG_VIDEOCOMPRESSION" },
	{ (WM_CAP_START + 5), "WM_CAP_SET_CALLBACK_FRAME" },
	{ (WM_CAP_START + 50), "WM_CAP_SET_PREVIEW" },
	{ (WM_CAP_START + 51), "WM_CAP_SET_OVERLAY" },
	{ (WM_CAP_START + 52), "WM_CAP_SET_PREVIEWRATE" },
	{ (WM_CAP_START + 53), "WM_CAP_SET_SCALE" },
	{ (WM_CAP_START + 54), "WM_CAP_GET_STATUS" },
	{ (WM_CAP_START + 55), "WM_CAP_SET_SCROLL" },
	{ (WM_CAP_START + 6), "WM_CAP_SET_CALLBACK_VIDEOSTREAM" },
	{ (WM_CAP_START + 60), "WM_CAP_GRAB_FRAME" },
	{ (WM_CAP_START + 61), "WM_CAP_GRAB_FRAME_NOSTOP" },
	{ (WM_CAP_START + 62), "WM_CAP_SEQUENCE" },
	{ (WM_CAP_START + 63), "WM_CAP_SEQUENCE_NOFILE" },
	{ (WM_CAP_START + 64), "WM_CAP_SET_SEQUENCE_SETUP" },
	{ (WM_CAP_START + 65), "WM_CAP_GET_SEQUENCE_SETUP" },
	{ (WM_CAP_START + 66), "WM_CAP_SET_MCI_DEVICEA" },
	{ (WM_CAP_START + 67), "WM_CAP_GET_MCI_DEVICEA" },
	{ (WM_CAP_START + 68), "WM_CAP_STOP" },
	{ (WM_CAP_START + 69), "WM_CAP_ABORT" },
	{ (WM_CAP_START + 7), "WM_CAP_SET_CALLBACK_WAVESTREAM" },
	{ (WM_CAP_START + 70), "WM_CAP_SINGLE_FRAME_OPEN" },
	{ (WM_CAP_START + 71), "WM_CAP_SINGLE_FRAME_CLOSE" },
	{ (WM_CAP_START + 72), "WM_CAP_SINGLE_FRAME" },
	{ (WM_CAP_START + 8), "WM_CAP_GET_USER_DATA" },
	{ (WM_CAP_START + 80), "WM_CAP_PAL_OPENA" },
	{ (WM_CAP_START + 81), "WM_CAP_PAL_SAVEA" },
	{ (WM_CAP_START + 82), "WM_CAP_PAL_PASTE" },
	{ (WM_CAP_START + 83), "WM_CAP_PAL_AUTOCREATE" },
	{ (WM_CAP_START + 84), "WM_CAP_PAL_MANUALCREATE" },
	{ (WM_CAP_START + 85), "WM_CAP_SET_CALLBACK_CAPCONTROL" },
	{ (WM_CAP_START + 9), "WM_CAP_SET_USER_DATA" },
	{ (WM_CAP_UNICODE_START + 12), "WM_CAP_DRIVER_GET_NAMEW" },
	{ (WM_CAP_UNICODE_START + 13), "WM_CAP_DRIVER_GET_VERSIONW" },
	{ (WM_CAP_UNICODE_START + 2), "WM_CAP_SET_CALLBACK_ERRORW" },
	{ (WM_CAP_UNICODE_START + 20), "WM_CAP_FILE_SET_CAPTURE_FILEW" },
	{ (WM_CAP_UNICODE_START + 21), "WM_CAP_FILE_GET_CAPTURE_FILEW" },
	{ (WM_CAP_UNICODE_START + 23), "WM_CAP_FILE_SAVEASW" },
	{ (WM_CAP_UNICODE_START + 25), "WM_CAP_FILE_SAVEDIBW" },
	{ (WM_CAP_UNICODE_START + 3), "WM_CAP_SET_CALLBACK_STATUSW" },
	{ (WM_CAP_UNICODE_START + 66), "WM_CAP_SET_MCI_DEVICEW" },
	{ (WM_CAP_UNICODE_START + 67), "WM_CAP_GET_MCI_DEVICEW" },
	{ (WM_CAP_UNICODE_START + 80), "WM_CAP_PAL_OPENW" },
	{ (WM_CAP_UNICODE_START + 81), "WM_CAP_PAL_SAVEW" },
	{ (WM_DDE_FIRST), "WM_DDE_INITIATE" },
	{ (WM_DDE_FIRST + 1), "WM_DDE_TERMINATE" },
	{ (WM_DDE_FIRST + 2), "WM_DDE_ADVISE" },
	{ (WM_DDE_FIRST + 3), "WM_DDE_UNADVISE" },
	{ (WM_DDE_FIRST + 4), "WM_DDE_ACK" },
	{ (WM_DDE_FIRST + 5), "WM_DDE_DATA" },
	{ (WM_DDE_FIRST + 6), "WM_DDE_REQUEST" },
	{ (WM_DDE_FIRST + 7), "WM_DDE_POKE" },
	{ (WM_DDE_FIRST + 8), "WM_DDE_EXECUTE" },
	{ (WM_DDE_FIRST + 8), "WM_DDE_LAST" },
	{ (WM_MOUSELAST + 1), "WM_MOUSEWHEEL" },
	{ (WM_PENWINFIRST + 1), "WM_RCRESULT" },
	{ (WM_PENWINFIRST + 2), "WM_HOOKRCRESULT" },
	{ (WM_PENWINFIRST + 3), "WM_GLOBALRCCHANGE" },
	{ (WM_PENWINFIRST + 3), "WM_PENMISCINFO" },
	{ (WM_PENWINFIRST + 4), "WM_SKB" },
	{ (WM_PENWINFIRST + 5), "WM_HEDITCTL" },
	{ (WM_PENWINFIRST + 5), "WM_PENCTL" },
	{ (WM_PENWINFIRST + 6), "WM_PENMISC" },
	{ (WM_PENWINFIRST + 7), "WM_CTLINIT" },
	{ (WM_PENWINFIRST + 8), "WM_PENEVENT" },
	{ (WM_USER + 1), "WM_CHOOSEFONT_GETLOGFONT" },
	{ (WM_USER + 101), "WM_CHOOSEFONT_SETLOGFONT" },
	{ (WM_USER + 102), "WM_CHOOSEFONT_SETFLAGS" },
	{ (WM_USER + 1101), "WM_ADSPROP_NOTIFY_PAGEINIT" },
	{ (WM_USER + 1102), "WM_ADSPROP_NOTIFY_PAGEHWND" },
	{ (WM_USER + 1103), "WM_ADSPROP_NOTIFY_CHANGE" },
	{ (WM_USER + 1104), "WM_ADSPROP_NOTIFY_APPLY" },
	{ (WM_USER + 1105), "WM_ADSPROP_NOTIFY_SETFOCUS" },
	{ (WM_USER + 1106), "WM_ADSPROP_NOTIFY_FOREGROUND" },
	{ (WM_USER + 1107), "WM_ADSPROP_NOTIFY_EXIT" },
	{ (WM_USER + 1110), "WM_ADSPROP_NOTIFY_ERROR" },
	{ (WM_USER), "WM_PSD_PAGESETUPDLG" },
	{ (WM_USER + 1), "WM_PSD_FULLPAGERECT" },
	{ (WM_USER + 1000), "WM_CPL_LAUNCH" },
	{ (WM_USER + 1001), "WM_CPL_LAUNCHED" },
	{ (WM_USER + 2), "WM_PSD_MINMARGINRECT" },
	{ (WM_USER + 3), "WM_PSD_MARGINRECT" },
	{ (WM_USER + 4), "WM_PSD_GREEKTEXTRECT" },
	{ (WM_USER + 5), "WM_PSD_ENVSTAMPRECT" },
	{ (WM_USER + 6), "WM_PSD_YAFULLPAGERECT" },
	{ 0x0000, "WM_NULL" },
	{ 0x0001, "WM_CREATE" },
	{ 0x0002, "WM_DESTROY" },
	{ 0x0003, "WM_MOVE" },
	{ 0x0005, "WM_SIZE" },
	{ 0x0006, "WM_ACTIVATE" },
	{ 0x0007, "WM_SETFOCUS" },
	{ 0x0008, "WM_KILLFOCUS" },
	{ 0x000A, "WM_ENABLE" },
	{ 0x000B, "WM_SETREDRAW" },
	{ 0x000C, "WM_SETTEXT" },
	{ 0x000D, "WM_GETTEXT" },
	{ 0x000E, "WM_GETTEXTLENGTH" },
	{ 0x000F, "WM_PAINT" },
	{ 0x0010, "WM_CLOSE" },
	{ 0x0011, "WM_QUERYENDSESSION" },
	{ 0x0012, "WM_QUIT" },
	{ 0x0013, "WM_QUERYOPEN" },
	{ 0x0014, "WM_ERASEBKGND" },
	{ 0x0015, "WM_SYSCOLORCHANGE" },
	{ 0x0016, "WM_ENDSESSION" },
	{ 0x0018, "WM_SHOWWINDOW" },
	{ 0x0019, "WM_CTLCOLOR" },
	{ 0x001A, "WM_WININICHANGE" },
	{ 0x001B, "WM_DEVMODECHANGE" },
	{ 0x001C, "WM_ACTIVATEAPP" },
	{ 0x001D, "WM_FONTCHANGE" },
	{ 0x001E, "WM_TIMECHANGE" },
	{ 0x001F, "WM_CANCELMODE" },
	{ 0x0020, "WM_SETCURSOR" },
	{ 0x0021, "WM_MOUSEACTIVATE" },
	{ 0x0022, "WM_CHILDACTIVATE" },
	{ 0x0023, "WM_QUEUESYNC" },
	{ 0x0024, "WM_GETMINMAXINFO" },
	{ 0x0026, "WM_PAINTICON" },
	{ 0x0027, "WM_ICONERASEBKGND" },
	{ 0x0028, "WM_NEXTDLGCTL" },
	{ 0x002A, "WM_SPOOLERSTATUS" },
	{ 0x002B, "WM_DRAWITEM" },
	{ 0x002C, "WM_MEASUREITEM" },
	{ 0x002D, "WM_DELETEITEM" },
	{ 0x002E, "WM_VKEYTOITEM" },
	{ 0x002F, "WM_CHARTOITEM" },
	{ 0x0030, "WM_SETFONT" },
	{ 0x0031, "WM_GETFONT" },
	{ 0x0032, "WM_SETHOTKEY" },
	{ 0x0033, "WM_GETHOTKEY" },
	{ 0x0037, "WM_QUERYDRAGICON" },
	{ 0x0039, "WM_COMPAREITEM" },
	{ 0x003D, "WM_GETOBJECT" },
	{ 0x0041, "WM_COMPACTING" },
	{ 0x0044, "WM_COMMNOTIFY" },
	{ 0x0046, "WM_WINDOWPOSCHANGING" },
	{ 0x0047, "WM_WINDOWPOSCHANGED" },
	{ 0x0048, "WM_POWER" },
	{ 0x004A, "WM_COPYDATA" },
	{ 0x004B, "WM_CANCELJOURNAL" },
	{ 0x004E, "WM_NOTIFY" },
	{ 0x004e, "WM_NOTIFY" },
	{ 0x0050, "WM_INPUTLANGCHANGEREQUEST" },
	{ 0x0051, "WM_INPUTLANGCHANGE" },
	{ 0x0052, "WM_TCARD" },
	{ 0x0053, "WM_HELP" },
	{ 0x0054, "WM_USERCHANGED" },
	{ 0x0055, "WM_NOTIFYFORMAT" },
	{ 0x007B, "WM_CONTEXTMENU" },
	{ 0x007C, "WM_STYLECHANGING" },
	{ 0x007D, "WM_STYLECHANGED" },
	{ 0x007E, "WM_DISPLAYCHANGE" },
	{ 0x007F, "WM_GETICON" },
	{ 0x007b, "WM_CONTEXTMENU" },
	{ 0x0080, "WM_SETICON" },
	{ 0x0081, "WM_NCCREATE" },
	{ 0x0082, "WM_NCDESTROY" },
	{ 0x0083, "WM_NCCALCSIZE" },
	{ 0x0084, "WM_NCHITTEST" },
	{ 0x0085, "WM_NCPAINT" },
	{ 0x0086, "WM_NCACTIVATE" },
	{ 0x0087, "WM_GETDLGCODE" },
	{ 0x0088, "WM_SYNCPAINT" },
	{ 0x00A0, "WM_NCMOUSEMOVE" },
	{ 0x00A1, "WM_NCLBUTTONDOWN" },
	{ 0x00A2, "WM_NCLBUTTONUP" },
	{ 0x00A3, "WM_NCLBUTTONDBLCLK" },
	{ 0x00A4, "WM_NCRBUTTONDOWN" },
	{ 0x00A5, "WM_NCRBUTTONUP" },
	{ 0x00A6, "WM_NCRBUTTONDBLCLK" },
	{ 0x00A7, "WM_NCMBUTTONDOWN" },
	{ 0x00A8, "WM_NCMBUTTONUP" },
	{ 0x00A9, "WM_NCMBUTTONDBLCLK" },
	{ 0x00AB, "WM_NCXBUTTONDOWN" },
	{ 0x00AC, "WM_NCXBUTTONUP" },
	{ 0x00AD, "WM_NCXBUTTONDBLCLK" },
	{ 0x00FF, "WM_INPUT" },
	{ 0x00fe, "WM_INPUT_DEVICE_CHANGE" },
	{ 0x0100, "WM_KEYDOWN" },
	{ 0x0100, "WM_KEYFIRST" },
	{ 0x0101, "WM_KEYUP" },
	{ 0x0102, "WM_CHAR" },
	{ 0x0103, "WM_DEADCHAR" },
	{ 0x0104, "WM_SYSKEYDOWN" },
	{ 0x0105, "WM_SYSKEYUP" },
	{ 0x0106, "WM_SYSCHAR" },
	{ 0x0107, "WM_SYSDEADCHAR" },
	{ 0x0109, "WM_KEYLAST" },
	{ 0x0109, "WM_UNICHAR" },
	{ 0x0109, "WM_UNICHAR" },
	{ 0x0109, "WM_WNT_CONVERTREQUESTEX" },
	{ 0x010A, "WM_CONVERTREQUEST" },
	{ 0x010B, "WM_CONVERTRESULT" },
	{ 0x010C, "WM_INTERIM" },
	{ 0x010D, "WM_IME_STARTCOMPOSITION" },
	{ 0x010E, "WM_IME_ENDCOMPOSITION" },
	{ 0x010F, "WM_IME_COMPOSITION" },
	{ 0x010F, "WM_IME_KEYLAST" },
	{ 0x0110, "WM_INITDIALOG" },
	{ 0x0111, "WM_COMMAND" },
	{ 0x0112, "WM_SYSCOMMAND" },
	{ 0x0113, "WM_TIMER" },
	{ 0x0114, "WM_HSCROLL" },
	{ 0x0115, "WM_VSCROLL" },
	{ 0x0116, "WM_INITMENU" },
	{ 0x0117, "WM_INITMENUPOPUP" },
	{ 0x0119, "WM_GESTURE" },
	{ 0x011A, "WM_GESTURENOTIFY" },
	{ 0x011F, "WM_MENUSELECT" },
	{ 0x0120, "WM_MENUCHAR" },
	{ 0x0121, "WM_ENTERIDLE" },
	{ 0x0122, "WM_MENURBUTTONUP" },
	{ 0x0123, "WM_MENUDRAG" },
	{ 0x0124, "WM_MENUGETOBJECT" },
	{ 0x0125, "WM_UNINITMENUPOPUP" },
	{ 0x0126, "WM_MENUCOMMAND" },
	{ 0x0127, "WM_CHANGEUISTATE" },
	{ 0x0128, "WM_UPDATEUISTATE" },
	{ 0x0129, "WM_QUERYUISTATE" },
	{ 0x0132, "WM_CTLCOLORMSGBOX" },
	{ 0x0133, "WM_CTLCOLOREDIT" },
	{ 0x0134, "WM_CTLCOLORLISTBOX" },
	{ 0x0135, "WM_CTLCOLORBTN" },
	{ 0x0136, "WM_CTLCOLORDLG" },
	{ 0x0137, "WM_CTLCOLORSCROLLBAR" },
	{ 0x0138, "WM_CTLCOLORSTATIC" },
	{ 0x0200, "WM_MOUSEFIRST" },
	{ 0x0200, "WM_MOUSEMOVE" },
	{ 0x0201, "WM_LBUTTONDOWN" },
	{ 0x0202, "WM_LBUTTONUP" },
	{ 0x0203, "WM_LBUTTONDBLCLK" },
	{ 0x0204, "WM_RBUTTONDOWN" },
	{ 0x0205, "WM_RBUTTONUP" },
	{ 0x0206, "WM_RBUTTONDBLCLK" },
	{ 0x0207, "WM_MBUTTONDOWN" },
	{ 0x0208, "WM_MBUTTONUP" },
	{ 0x0209, "WM_MBUTTONDBLCLK" },
	{ 0x020A, "WM_MOUSEWHEEL" },
	{ 0x020B, "WM_XBUTTONDOWN" },
	{ 0x020C, "WM_XBUTTONUP" },
	{ 0x020D, "WM_XBUTTONDBLCLK" },
	{ 0x020d, "WM_MOUSELAST" },
	{ 0x020e, "WM_MOUSEHWHEEL" },
	{ 0x020e, "WM_MOUSELAST" },
	{ 0x0210, "WM_PARENTNOTIFY" },
	{ 0x0211, "WM_ENTERMENULOOP" },
	{ 0x0212, "WM_EXITMENULOOP" },
	{ 0x0213, "WM_NEXTMENU" },
	{ 0x0214, "WM_SIZING" },
	{ 0x0215, "WM_CAPTURECHANGED" },
	{ 0x0216, "WM_MOVING" },
	{ 0x0218, "WM_POWERBROADCAST" },
	{ 0x0218, "WM_POWERBROADCAST" },
	{ 0x0219, "WM_DEVICECHANGE" },
	{ 0x0219, "WM_DEVICECHANGE" },
	{ 0x0220, "WM_MDICREATE" },
	{ 0x0221, "WM_MDIDESTROY" },
	{ 0x0222, "WM_MDIACTIVATE" },
	{ 0x0223, "WM_MDIRESTORE" },
	{ 0x0224, "WM_MDINEXT" },
	{ 0x0225, "WM_MDIMAXIMIZE" },
	{ 0x0226, "WM_MDITILE" },
	{ 0x0227, "WM_MDICASCADE" },
	{ 0x0228, "WM_MDIICONARRANGE" },
	{ 0x0229, "WM_MDIGETACTIVE" },
	{ 0x0230, "WM_MDISETMENU" },
	{ 0x0231, "WM_ENTERSIZEMOVE" },
	{ 0x0232, "WM_EXITSIZEMOVE" },
	{ 0x0233, "WM_DROPFILES" },
	{ 0x0234, "WM_MDIREFRESHMENU" },
	{ 0x0240, "WM_TOUCH" },
	{ 0x0241, "WM_NCPOINTERUPDATE" },
	{ 0x0242, "WM_NCPOINTERDOWN" },
	{ 0x0243, "WM_NCPOINTERUP" },
	{ 0x0245, "WM_POINTERUPDATE" },
	{ 0x0246, "WM_POINTERDOWN" },
	{ 0x0247, "WM_POINTERUP" },
	{ 0x0249, "WM_POINTERENTER" },
	{ 0x024a, "WM_POINTERLEAVE" },
	{ 0x024b, "WM_POINTERACTIVATE" },
	{ 0x024c, "WM_POINTERCAPTURECHANGED" },
	{ 0x024d, "WM_TOUCHHITTESTING" },
	{ 0x024e, "WM_POINTERWHEEL" },
	{ 0x024f, "WM_POINTERHWHEEL" },
	{ 0x0280, "WM_IME_REPORT" },
	{ 0x0281, "WM_IME_SETCONTEXT" },
	{ 0x0282, "WM_IME_NOTIFY" },
	{ 0x0283, "WM_IME_CONTROL" },
	{ 0x0284, "WM_IME_COMPOSITIONFULL" },
	{ 0x0285, "WM_IME_SELECT" },
	{ 0x0286, "WM_IME_CHAR" },
	{ 0x0288, "WM_IME_REQUEST" },
	{ 0x0290, "WM_IME_KEYDOWN" },
	{ 0x0291, "WM_IME_KEYUP" },
	{ 0x02A0, "WM_NCMOUSEHOVER" },
	{ 0x02A1, "WM_MOUSEHOVER" },
	{ 0x02A2, "WM_NCMOUSELEAVE" },
	{ 0x02A3, "WM_MOUSELEAVE" },
	{ 0x02B1, "WM_WTSSESSION_CHANGE" },
	{ 0x02CC, "WM_TABLET_QUERYSYSTEMGESTURESTATUS" },
	{ 0x02c0, "WM_TABLET_FIRST" },
	{ 0x02df, "WM_TABLET_LAST" },
	{ 0x0300, "WM_CUT" },
	{ 0x0301, "WM_COPY" },
	{ 0x0302, "WM_PASTE" },
	{ 0x0303, "WM_CLEAR" },
	{ 0x0304, "WM_UNDO" },
	{ 0x0305, "WM_RENDERFORMAT" },
	{ 0x0306, "WM_RENDERALLFORMATS" },
	{ 0x0307, "WM_DESTROYCLIPBOARD" },
	{ 0x0308, "WM_DRAWCLIPBOARD" },
	{ 0x0309, "WM_PAINTCLIPBOARD" },
	{ 0x030A, "WM_VSCROLLCLIPBOARD" },
	{ 0x030B, "WM_SIZECLIPBOARD" },
	{ 0x030C, "WM_ASKCBFORMATNAME" },
	{ 0x030D, "WM_CHANGECBCHAIN" },
	{ 0x030E, "WM_HSCROLLCLIPBOARD" },
	{ 0x030F, "WM_QUERYNEWPALETTE" },
	{ 0x0310, "WM_PALETTEISCHANGING" },
	{ 0x0311, "WM_PALETTECHANGED" },
	{ 0x0312, "WM_HOTKEY" },
	{ 0x0317, "WM_PRINT" },
	{ 0x0318, "WM_PRINTCLIENT" },
	{ 0x0318, "WM_PRINTCLIENT" },
	{ 0x0319, "WM_APPCOMMAND" },
	{ 0x031A, "WM_THEMECHANGED" },
	{ 0x031d, "WM_CLIPBOARDUPDATE" },
	{ 0x031e, "WM_DWMCOMPOSITIONCHANGED" },
	{ 0x031f, "WM_DWMNCRENDERINGCHANGED" },
	{ 0x0320, "WM_DWMCOLORIZATIONCOLORCHANGED" },
	{ 0x0321, "WM_DWMWINDOWMAXIMIZEDCHANGE" },
	{ 0x0323, "WM_DWMSENDICONICTHUMBNAIL" },
	{ 0x0326, "WM_DWMSENDICONICLIVEPREVIEWBITMAP" },
	{ 0x033f, "WM_GETTITLEBARINFOEX" },
	{ 0x0358, "WM_HANDHELDFIRST" },
	{ 0x035F, "WM_HANDHELDLAST" },
	{ 0x0360, "WM_AFXFIRST" },
	{ 0x037F, "WM_AFXLAST" },
	{ 0x0380, "WM_PENWINFIRST" },
	{ 0x038F, "WM_PENWINLAST" },
	{ 0x03E0, "WM_DDE_FIRST" },
	{ 0x0400, "WM_USER" },
	{ 0x238, "WM_POINTERDEVICECHANGE" },
	{ 0x239, "WM_POINTERDEVICEINRANGE" },
	{ 0x23a, "WM_POINTERDEVICEOUTOFRANGE" },
	{ 0x290, "WM_IMEKEYDOWN" },
	{ 0x291, "WM_IMEKEYUP" },
	{ 0x2a1, "WM_MOUSEHOVER" },
	{ 0x2a3, "WM_MOUSELEAVE" },
	{ 0x8000, "WM_APP" },
	{ 0xCCCD, "WM_RASDIALEVENT" },
	{ 900, "WM_FI_FILENAME" },
	{ WM_CAP_PAL_SAVEW, "WM_CAP_UNICODE_END" },
	{ WM_CAP_UNICODE_END, "WM_CAP_END" },
	{ WM_USER + 100, "WM_CAP_UNICODE_START" },
	{ WM_USER, "WM_CAP_START" },
	{ WM_WININICHANGE, "WM_SETTINGCHANGE" }
	};
	*/

};
#endif // !HEADER_ContextMacroListView