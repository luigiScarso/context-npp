#include "StdAfx.h"
#include "PluginInterface.h"
#include "Scintilla.h"
#include "Tag.h"

#include "ContextMacroListView.h"
struct Tag;

/// Used to store the item name and left and right text
struct Pair
{
	Pair() {}
	string name;
	string left;
	string right;
	string shortcut;
};

/// This class is used for reading and inserting pairs from the ini file
class ConTeXtEditU
{
public:
	ConTeXtEditU(HWND hNotePad, HWND hSCI);
	static ConTeXtEditU* create(HWND hNotepad, HWND hSCI);
	static void SurroundSelection(const char* leftText, const char* rightText);
	static void ApplyPair(Pair pair);
	static ConTeXtEditU* getInstance() { return s_contextEditU; }
	static void deleteInstance()
	{
		if (s_contextEditU)
		{
			delete s_contextEditU;
			s_contextEditU = NULL;
		}
	}
	void EditConfig();
	void ReadConfig();//VAR numRead : INTEGER; initToolbar: BOOLEAN);
	char ReadChar(DWORD &buffPos, DWORD &buffLen, bool &eof, HANDLE &hFile, char* buff, LPDWORD read);
	bool ReadLine(char* line, DWORD &buffPos, DWORD &buffLen, bool &eof,
		HANDLE &hFile, char* buff, LPDWORD read, const char &commentChar, bool &section);
	void UnescapeStr(char* str);
	bool LineToPair(Pair &pair, char* line);
	void LineToToolbar(char* line, DWORD &maxFnameLen);
	void LineToTag(char* line);
	void LineToSetup(char* line);
	void replaceTag() {
		tag_ins.replace(); }
	HWND InsertCtxMacro();
	void DeleteCtxMacro();
	std::string GetSetupValue(const char *key);
	static vector<Pair> pairs; ///< Store pair array
	size_t numRead; ///< Number of read pairs, equals pairs.size()
	vector<size_t> groupStarts; ///< Store the start indices for each group in the pair vector (pairs)
	vector<string> groupNames; ///< Store the name of each group
	size_t numGroups; ///< Number of groups defined by user

private:
	static ConTeXtEditU* s_contextEditU;
	static HWND hSci;
	HWND hNpp;
	bool initToolbar;

	std::string CommandsSetupIniSection = "[CommandsSetup]";
	std::string CommandsIniSection = "[Commands]";
	std::string CommandsEndSection = "[CommandsEnd]";
	std::string ToolbarIniSection = "[Toolbar]";
	std::string TagsIniSection = "[Tags]";
	std::string AboutMsg = "Originally Made by Jason Yu. Current developer Luigi Scarso.";
	
	Tag tag_ins; ///< Tag instance
	std::map< std::string, std::string > setup_ins;

	TCHAR configDir[MAX_PATH];
	TCHAR IniFileDir[MAX_PATH];
	//TCHAR IniFileName[100] = TEXT("ConTeXtEdit.ini");
	TCHAR IniFileName[100] = TEXT("ConTeXt.ini");

	bool isCommandSession;
};