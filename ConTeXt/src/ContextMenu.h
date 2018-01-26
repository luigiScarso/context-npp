#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

// tinyxml.h was copied from Notepad++
#include "StdAfx.h"
#include "tinyxml.h"
#include "Scintilla.h"
#include "PluginInterface.h"

/// This is used to modify the contextMenu.xml file of Npp and add commands to right-click context menu.
class modifyContextMenu 
{
public:
	modifyContextMenu(HWND hNotePad);
	~modifyContextMenu();
	bool loadContextXml();
	void countUserItems();
	void clear();
	bool update(const TCHAR*, idx_t, const TCHAR*);
	void editXML();
	void save() 
	{
		_pXmlFile->SaveFile(xmlDir);
	}

	size_t userChildCount; ///< Number of user-added context menus

private:
	HWND hNpp;
	TiXmlDocument *_pXmlFile;
	TCHAR menuXmlName[100] = TEXT("contextMenu.xml");
	TiXmlNode* parent; ///< <ScintillaContextMenu> entry in xml
	TiXmlNode* firstNonUserChild;
	TiXmlNode* firstUserChild;
	TCHAR xmlDir[MAX_PATH];
};

#endif // !CONTEXTMENU_H

