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
	static void Register();
	static void UnRegister();
	static HWND ComposeAndShowWindow(const TCHAR* header, HWND hOwner);
	static bool DestroyStaticInstance();

	BOOL LoadMacro(ContextMacro* ctxmacro);
	BOOL SetSetupValue(std::string Key, std::string Value);
	BOOL SetBackgroundColor(COLORREF bgcolor);

	HWND CreateListView(HINSTANCE hInstance, HWND hwndParent);
	void ResizeListView(HWND hwndListView, HWND hwndParent);
	void PositionHeader(HWND hwndListView);
	BOOL InitListView(HWND hwndListView);
	BOOL InsertListViewItems(HWND hwndListView);
	LRESULT ListViewNotify(HWND hWnd, LPARAM lParam);

	generic_string getUserMacroSelected();

	void setItemClass(int itemclass, int itemnr);
	std::vector<int>* getItemClass(int itemnr);
	void setDisplayMaxRows(int r) { DisplayMaxRows = std::abs(r); }
	int getDisplayMaxRows() { return DisplayMaxRows; }
	int getNrOfColumns()  { return NrOfColumns; }
	UINT getTimerElapse() { return elapse; }
	UINT setTimerElapse(UINT newval) { UINT oldval = elapse;  elapse = newval;  return oldval; }

private:
	int NrOfColumns = MAXCOLUMNNR;
	ContextMacro  UserMacro;
	std::map< generic_string, UINT > ShortcutPrefixCnt;
	int shortcut_currentState = 0;
	generic_string shortcut_currentstring;
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
	int DisplayMaxRowsLimit = 100;

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
	HINSTANCE hCurrentInstance;

	int getItemSelectedandFocused(HWND hwndListView);
	unsigned int getHeight(HWND hwndListView, const TCHAR* st);
	unsigned int getMaxWidth(HWND hwndListView);
	unsigned int getMaxItemWidth(HWND hwndListView, unsigned int column);
	
	void makeshortcutprefix();
	BOOL loademptymacro();
	int	getFirstSelected(HWND hwndLV);

	

};