#ifndef _MENUMANAGER_H
#define _MENUMANAGER_H

#include "Lexer.h"
#include "ContextMenu.h"

struct FuncItem;
class IDAllocator;
class DynamicIDManager;

/// A master class for managing static and dynamic menus
class MenuManager
{
public:
	~MenuManager();
	typedef vector<pair<tstring, void (*)()> > ItemVectorTD;
	static MenuManager* create(HWND hNotepad);
	static MenuManager* getInstance();
	static void         deleteInstance();
	FuncItem* getFuncItemArray(int *nbF, ItemVectorTD items);
	bool populatePairMenu();
	void refreshPairMenu(size_t);
	void updateContextMenu();
	void removeContextMenu();
	void idsInitialised();
	void loadConfig();
	BOOL processWmCommand(WPARAM wParam, LPARAM lParam);

private:
	MenuManager(); ///< default constructor disabled
	MenuManager(HWND hNotepad); 
	HMENU getOurMenu();
	map<idx_t, size_t> funcs_map; ///< map menu ID to pair number
	map<size_t, idx_t> funcs_map_reversed; ///< map pair number to menu ID
	static MenuManager* s_menuManager; ///< static instance

	HWND		m_hNotepad; 
	HMENU		m_contextMenu; ///< Store the position of our ConTeXt plugin menu
	vector<HMENU>		m_GroupMenu; ///< store the positions of our submenus (pair groups) under ConTeXt
	HMENU		hPluginMenu; ///< Store the position of the main plugin menu of Notepad++
	FuncItem*   m_funcItems; ///< Store static menus of the ConTeXt plugin
	IDAllocator* m_idAllocator; ///< Used for dynamic menu ID allocation
	DynamicIDManager* m_GroupMenuManager; ///< Used for dymamic menu ID allocation
};

#endif // _MENUMANAGER_H
