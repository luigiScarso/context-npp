#include "ContextMenu.h"

modifyContextMenu::modifyContextMenu(HWND hNotePad):
	hNpp(hNotePad),
	_pXmlFile(NULL)
{
	//find contextMenu.xml
	//search appdata first
	TCHAR tmpDir[MAX_PATH];
	::SendMessage(hNpp, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(tmpDir));
	TCHAR* pdest = _tcsstr(tmpDir, _T("plugins"));
	int pre_n = (int)(pdest - tmpDir);
	
	_tcsncpy(xmlDir, tmpDir, pre_n);

	
	_tcscat_s(xmlDir, MAX_PATH, menuXmlName);
	//MessageBox(0, xmlDir, 0, MB_OK);

	if (!PathFileExists(xmlDir))
	{
		::SendMessage(hNpp, NPPM_GETNPPDIRECTORY, MAX_PATH, reinterpret_cast<LPARAM>(xmlDir));
		_tcscat_s(xmlDir, MAX_PATH, _T("\\"));
		_tcscat_s(xmlDir, MAX_PATH, menuXmlName);
	}
}

modifyContextMenu::~modifyContextMenu()
{ 
	if (_pXmlFile)
		delete _pXmlFile;
}

/// Load the right-click menu xml file
bool modifyContextMenu::loadContextXml()
{
	if (_pXmlFile)
		delete _pXmlFile;
	userChildCount = 0;
	_pXmlFile = new TiXmlDocument(xmlDir);
	if (_pXmlFile->LoadFile())
	{
		parent = _pXmlFile->FirstChild(TEXT("NotepadPlus"));
		if (!parent)
		{
			MessageBox(hNpp, L"Context XML file has a wrong format!", L"ConTeXt", MB_OK);
			return FALSE;
		}
		parent = parent->FirstChildElement(TEXT("ScintillaContextMenu"));
		if (!parent)
		{
			MessageBox(hNpp, L"Context XML file has a wrong format!", L"ConTeXt", MB_OK);
			return FALSE;
		}
	}
	else
	{
		MessageBox(hNpp, L"Cannot find Context XML file!", L"ConTeXt", MB_OK);
		return FALSE;
	}
	countUserItems();
	return TRUE;
}

/// Count the number of menu items added by this plugin
void modifyContextMenu::countUserItems()
{
	firstNonUserChild = parent->FirstChildElement(TEXT("Item"));
	TiXmlElement* pElem;
	bool foundFristNonUser = FALSE;
	bool foundFristUser = FALSE;
	for (pElem = reinterpret_cast<TiXmlElement *>(firstNonUserChild); pElem; pElem = pElem->NextSiblingElement())
	{
		const TCHAR *isUser = pElem->Attribute(TEXT("User"));
		if (isUser)
		{
			if (!foundFristUser)
			{
				firstUserChild = pElem;
				foundFristUser = TRUE;
			}
			++userChildCount;
		}
		else
		{
			if (!foundFristNonUser)
			{
				firstNonUserChild = pElem;
				foundFristNonUser = TRUE;
			}
			break;
		}
	}
}

/// Remove all context menus added by this plugin
void modifyContextMenu::clear()
{
	for (size_t i = 0; i != userChildCount; ++i)
	{
		TiXmlNode* to_delete = firstUserChild;
		firstUserChild = firstUserChild->NextSibling();
		parent->RemoveChild(to_delete);
	}
	firstUserChild = nullptr;
	userChildCount = 0;
}

/// Update context menu xml by adding menu name and the command ID
bool modifyContextMenu::update(const TCHAR* folderName, idx_t id, const TCHAR* itemName)
{
	//<Item FolderName="" id="" ItemNameAs="" User=""/>
	if (!parent)
		return FALSE;
	TiXmlElement insert(TEXT("Item"));
	insert.SetAttribute(TEXT("FolderName"), folderName);
	insert.SetAttribute(TEXT("id"), id);
	insert.SetAttribute(TEXT("ItemNameAs"), itemName);
	insert.SetAttribute(TEXT("User"), TEXT(""));
	TiXmlNode* success; 
	
	//If there is no Item elements, insert at end
	if (!parent->FirstChildElement())
	{
		firstUserChild = parent->InsertEndChild(insert);
		success = firstUserChild;
	}
	else if (firstNonUserChild)
	{
		TiXmlNode* tmp = parent->InsertBeforeChild(firstNonUserChild, insert);
		if (!firstUserChild)
			firstUserChild = tmp;
		success = tmp;
	}
	else
	{
		TiXmlNode* tmp = parent->InsertEndChild(insert);
		if (!firstUserChild)
			firstUserChild = tmp;
		success = tmp;
	}
	if (success)
	{
		++userChildCount;
		return TRUE;
	}
	else
		return FALSE;
}

/// Open context xml for editing
void modifyContextMenu::editXML()
{
	::SendMessage(hNpp, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(xmlDir));
}