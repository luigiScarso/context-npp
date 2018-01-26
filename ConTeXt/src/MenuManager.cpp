#include "StdAfx.h"
#include "MenuManager.h"
#include "PluginInterface.h"
#include "NppAllocator.h"
#include "Notepad_plus_msgs.h"
#include "DynamicIDManager.h"
#include "ConTeXtEditU.h"

// Static instance
MenuManager* MenuManager::s_menuManager = NULL;

/// Create a static instance of MenuManager
MenuManager* MenuManager::create(HWND hNotepad)
{
	if (NULL == s_menuManager)
	{
		s_menuManager = new MenuManager(hNotepad);
	}
	return s_menuManager;
}

MenuManager::~MenuManager()
{
	if (m_GroupMenuManager)
		delete m_GroupMenuManager;

	if (m_contextMenu)
	{
		::DestroyMenu(m_contextMenu);
		m_contextMenu = NULL;
	}
	for (size_t gp_no = 0; gp_no != m_GroupMenu.size(); ++gp_no)
	{
		if (m_GroupMenu[gp_no])
		{
			::DestroyMenu(m_GroupMenu[gp_no]);
			m_GroupMenu[gp_no] = NULL;
		}
	}
	if (m_funcItems)
	{
		delete [] m_funcItems;
		m_funcItems = NULL;
	}
	m_hNotepad = NULL;
}

MenuManager* MenuManager::getInstance()
{
	return s_menuManager;
}

void MenuManager::deleteInstance()
{
	if (s_menuManager)
	{
		delete s_menuManager;
		s_menuManager = NULL;
	}
}

MenuManager::MenuManager(HWND hNotepad) :
	m_hNotepad (hNotepad),	
	m_contextMenu(NULL),
	m_funcItems(NULL),
	m_idAllocator(NULL),
	m_GroupMenuManager(NULL)
{
}

/*! This code was shamefully robbed from NppExec from Dovgan Vitaliy*/
HMENU MenuManager::getOurMenu()
{
	assert(m_funcItems != NULL);
	if (m_funcItems && NULL == m_contextMenu)
	{
		HMENU hPluginMenu = (HMENU)::SendMessage(m_hNotepad, NPPM_GETMENUHANDLE, 0, 0);

		int iMenuItems = GetMenuItemCount(hPluginMenu);
		for ( int i = 0; i < iMenuItems; i++ )
		{
			HMENU hSubMenu = ::GetSubMenu(hPluginMenu, i);
			if ( ::GetMenuState(hSubMenu, (UINT)m_funcItems[1]._cmdID, MF_BYCOMMAND) != -1 )
			{
				// this is our ConTeXt plugin menu
				m_contextMenu = hSubMenu;
				break;
			}
		}
	}

	return m_contextMenu;
}

/// Populate the pair submenus
bool MenuManager::populatePairMenu()
{
	const ConTeXtEditU* conEdit = ConTeXtEditU::getInstance();
	m_contextMenu = getOurMenu();
	if (!m_contextMenu)
	{
		return false;
	}
	else
	{
		assert(m_GroupMenuManager != NULL);
		if (m_GroupMenuManager)
		{
			m_GroupMenuManager->begin();
		}
		// delete every existing submenu
		for (size_t gp_no = 0; gp_no != m_GroupMenu.size(); ++gp_no)
			DeleteMenu(m_contextMenu, 0, MF_BYPOSITION);
		m_GroupMenu.clear();
		funcs_map.clear();
		
		// add new group menus
		for (size_t gp_no = 0; gp_no != conEdit->numGroups; ++gp_no)
		{
			m_GroupMenu.push_back(CreateMenu());
			InsertMenu(m_contextMenu, gp_no,
				MF_BYPOSITION | MF_POPUP,
				reinterpret_cast<UINT_PTR>(m_GroupMenu[gp_no]),
				CA2T(conEdit->groupNames[gp_no].c_str()));
			refreshPairMenu(gp_no);
		}
	}
	//Create a reversed map from pair index to menu ID
	for (map<idx_t, size_t>::iterator iter = funcs_map.begin(); iter != funcs_map.end(); ++iter)
		funcs_map_reversed.insert(pair<size_t, idx_t>(iter->second, iter->first));
	return true;
}

/// Fills the pair submenu by group 
void MenuManager::refreshPairMenu(size_t gp_no)
{
	idx_t position = 0;
	const ConTeXtEditU* conEdit = ConTeXtEditU::getInstance();
	size_t start = conEdit->groupStarts[gp_no];
	size_t end = conEdit->groupStarts[gp_no + 1];
	
	for (size_t func_no = start; func_no != end; ++func_no)
	{
		if (Constants::SEP.compare(conEdit->pairs[func_no].name) == 0)
		{
			//this is a line separator
			InsertMenu(m_GroupMenu[gp_no],
				position++,
				MF_BYCOMMAND | MF_SEPARATOR,
				0, NULL);
		}
		else
		{
			//this is an actual command
			InsertMenuA(m_GroupMenu[gp_no],
				position++,
				MF_BYCOMMAND | MF_STRING | MF_UNCHECKED,
				m_GroupMenuManager->currentID(),
				conEdit->pairs[func_no].name.c_str());

			funcs_map.insert(pair<idx_t, size_t>(m_GroupMenuManager->currentID(), func_no));
			++(*m_GroupMenuManager);
		}
	}
}

/// Function used to update right-click menu
void MenuManager::updateContextMenu()
{
	modifyContextMenu mc(m_hNotepad);
	mc.loadContextXml();
	mc.clear();
	const ConTeXtEditU* conEdit = ConTeXtEditU::getInstance();
	for (size_t gp_no = 0; gp_no != conEdit->numGroups; ++gp_no)
	{
		//folder name for right click menu
		string fName = conEdit->groupNames[gp_no];
		TCHAR* folderName = CA2T(fName.c_str());

		size_t start = conEdit->groupStarts[gp_no];
		size_t end = conEdit->groupStarts[gp_no + 1];
		for (size_t func_no = start; func_no != end; ++func_no)
		{
			string iName = conEdit->pairs[func_no].name;
			TCHAR* itemName = CA2T(iName.c_str());
			map<idx_t, size_t>::iterator id_found = funcs_map_reversed.find(func_no);
			if (id_found != funcs_map_reversed.end())
				mc.update(folderName, id_found->second, itemName);
			else if (Constants::SEP.compare(conEdit->pairs[func_no].name) == 0)
				mc.update(folderName, 0, TEXT(""));
		}
	}
	mc.save();
}

/// Remove user-added right-click menus
void MenuManager::removeContextMenu()
{
	modifyContextMenu mc(m_hNotepad);
	mc.loadContextXml();
	mc.clear();
	mc.save();
}

/// Load configuration from ini file
void MenuManager::loadConfig()
{
	ConTeXtEditU* contextEdit_ins = ConTeXtEditU::getInstance();
	contextEdit_ins->ReadConfig();
	populatePairMenu();
}

/// Process command when a submenu is clicked, wParam is the menu ID
BOOL MenuManager::processWmCommand(WPARAM wParam, LPARAM /* lParam */)
{
	map<idx_t, size_t>::iterator func_found = funcs_map.find(LOWORD(wParam));
	if (func_found != funcs_map.end())
	{
		size_t func_no = func_found->second;
		const ConTeXtEditU* contextEdit_ins = ConTeXtEditU::getInstance();
		contextEdit_ins->ApplyPair(contextEdit_ins->pairs[func_no]);
		return TRUE;
	}
	return FALSE;
}

/// Construct a FuncItem array containing all the static menus
FuncItem* MenuManager::getFuncItemArray(int *nbF, ItemVectorTD items)
{
	*nbF = items.size();
	size_t m_funcItemCount = (size_t)*nbF;

	// WARNING: If getFuncItemArray is called twice, we'll leak memory!
	m_funcItems = new FuncItem[m_funcItemCount];

	// Add all the static items passed in
	idx_t position = 0;
	
	for(ItemVectorTD::iterator it = items.begin(); it != items.end(); ++it)
	{
		_tcscpy_s(m_funcItems[position]._itemName, 64, it->first.c_str());
		m_funcItems[position]._init2Check = false;
		m_funcItems[position]._pShKey = NULL;
		m_funcItems[position]._pFunc = it->second;
		++position;
	}
	return m_funcItems;
}

/// Initialize by acquiring dynamic menu ID from Notepad++
void MenuManager::idsInitialised()
{
	if (::SendMessage(m_hNotepad, NPPM_ALLOCATESUPPORTED, 0, 0) == TRUE)
	{
		m_idAllocator = new NppAllocator(m_hNotepad);
	}
	//assert(m_dynamicStartIndex != IDX_MAX);
	m_GroupMenuManager = new DynamicIDManager(m_idAllocator);
}