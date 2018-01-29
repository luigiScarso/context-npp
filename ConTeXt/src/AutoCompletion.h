#include <string>
#include <unordered_map>
#include <vector>
#include "StdAfx.h"
#include "tinyxml.h"

#include "Scintilla.h"

#define TIXMLA_USE_STL
//#include "ScintillaEditView.h"

//const size_t tagMaxLen = 256;

class AutoCompletion {
public:
	enum ActiveCompletion {CompletionNone = 0, CompletionFunc, CompletionWord, CompletionBoth};

	AutoCompletion(HWND npp, HWND sci) : hNpp(npp), hSci(sci) { readApi(); }
	~AutoCompletion();
	static AutoCompletion* create(HWND, HWND);
	static AutoCompletion* getinstance() { return s_autoc; }
	static void deleteinstance(){
		if (s_autoc)
		{
			delete s_autoc;
			s_autoc = NULL;
		}
	}
	void getGenericText(TCHAR *dest, size_t destlen, size_t start, size_t end);
	void showAutoComletion(size_t lenEntered, const TCHAR* list);
	int AutoCompletion::searchInTarget(const TCHAR * text2Find, size_t lenOfText2Find, size_t fromPos, size_t toPos);
	bool readApi();
	bool readConfig();

	///AutoComplete from the list
	bool showApiComplete();
	///WordCompletion from the current file
	bool showWordComplete(bool autoInsert);	//autoInsert true if completion should fill in the word on a single match
	/// AutoComplete from both the list and the current file
	bool showApiAndWordComplete();
	/// Parameter display from list
	bool showFunctionComplete(int character,const char* keyword);
	/// Main updating function, entry for SCN_CHARADDED
	void update(int character);
	/// calltuip management
	bool isVisibleCalltip();
	int updateCalltip(int ch, bool needShow);

private:
	static AutoCompletion * s_autoc; 
	HWND hSci;
	HWND hNpp;
	// calltip management
	std::unordered_map < generic_string, std::vector<  std::vector<generic_string>   > > _overload; // _overeload[key]={ {param1},{param2},{param3}}

	typedef sptr_t (*SCINTILLA_FUNC)(void*, unsigned int, uptr_t, sptr_t);
	typedef void * SCINTILLA_PTR;
	typedef LRESULT(WINAPI *CallWindowProcFunc) (WNDPROC, HWND, UINT, WPARAM, LPARAM);
	SCINTILLA_FUNC _pScintillaFunc = nullptr;
	SCINTILLA_PTR  _pScintillaPtr = nullptr;
	int startFuncs[256];
	int stopFuncs[256];
		
	TiXmlDocument *_pXmlConfig = nullptr; //< pointer to config.xml
	int autocStatus = CompletionBoth; //< auto completion status, None/Func/Word/Both
	int autocFromLen = 3; //< Show autocompletion when the input length equals to or exceeds this number
	bool autocIgnoreNumbers = true; //< Whether to ignore numbers
	bool _ignoreCase = false; //< whether to ignore case

	bool _funcCompletionActive = false; //< indicator whether reading keywords from ConTeXt.xml succeeds
	TiXmlDocument *_pXmlFile = nullptr; //< pointer to ConTeXt.xml
	TiXmlElement *_pXmlKeyword = nullptr;

	std::vector<generic_string> _keyWordArray;
	generic_string _keyWords; //< Keywords read from ConTeXt.xml in plugins/API
	size_t _keyWordMaxLen = 0;
	generic_string latestKeyWord; // latest keyword found
	unsigned int latestKeyWordNrParams = 0;
	unsigned int latestKeyWordParamSetCompleted = 0;

	void getWordArray(std::vector<generic_string> & wordArray, TCHAR *beginChars);
	generic_string getlastmacro(const TCHAR* t);
	// calltip management
	bool findParam(const TCHAR* key);
	LRESULT execute(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	unsigned int ctTextSize = 2048;
	char *ctText; // = new char[ctTextSize];
	unsigned int columns = 1; // only 1 or 2 columns 
	unsigned int thresholdNrOfRows=20; // min. nr. of rows, only when 2 columns 
	unsigned int widthColumnSep = 2; // distance in ' ' among columns, only when 2 columns 
	unsigned int maxLineLength = 80;// no more than maxLineLenght chars
	unsigned int toplines = 0; 
	bool macroValueOnSingleLine = true;
	unsigned int tabSpacesParameters = 2; // insert tabSpacesParameters spaces  at the beginning of each new row
	generic_string tab = L"  ";
	unsigned int paramSeparator = L',';    // separator between parameters
	unsigned int spaceSeparator = L' ';    // add a spaceSeparator after paramSeparator
	unsigned int startMacro = L'\\';    // add a spaceSeparator after paramSeparator
	int calltipSizeInPoint = 9;
	bool sortMacroValues = false; 
	char* calltipFontName = nullptr;
	int calltipForeColor = 0x101010;
	int calltipBackColor = 0xFFFFFF;
};

/*  Snippet
{//Debug begin
 	std::time_t result = std::time(nullptr);
 	std::wfstream s("C:\\data\\lex.txt", std::ios::app);
 //	UINT cp = static_cast<UINT>(::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0));
 //	int lenMbcs = WideCharToMultiByte(cp, 0, name, -1, NULL, 0, NULL, NULL);
 //	char* _name = new char[lenMbcs];
 //	WideCharToMultiByte(cp, 0, name, -1, _name, lenMbcs, NULL, NULL);
 	s << result << " showFunctionComplete keyName="<<keyName<<" keyTarget=" << keyTarget<<std::endl;
 //	s << _name << std::endl;
 	s.close();
 //	delete[] _name;
}//Debug end



if (true) {//Debug begin
	std::time_t result = std::time(nullptr);
	std::wfstream ws("C:\\data\\lex.txt", std::ios::app);
	ws << result << L" text" << std::endl;
	ws.close();
}//Debug end




*/