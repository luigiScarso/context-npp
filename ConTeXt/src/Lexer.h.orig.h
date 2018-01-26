#ifndef ConTeXt_LEXER_H_
#define ConTeXt_LEXER_H_
#define MAX_MENU_SCRIPTS 50

#include "Scintilla.h"
#include "ILexer.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "LexerModule.h"
#include "StyleContext.h"
#include "CharacterSet.h"

namespace ConTeXt {

/// ConTeXt script lexing and folding.
class ConTeXt final :
	public ILexer
{
public:
	ConTeXt() {}
	virtual ~ConTeXt() {}

	static ILexer* LexerFactory();

	inline bool isOperator(const char ch);
	int ParseTeXCommand(unsigned int pos, Accessor &styler, char *command);
	int classifyFoldPointTeXPaired(const char* s);
	int classifyFoldPointTeXUnpaired(const char* s);
	bool IsTeXCommentLine(int line, Accessor &styler);

	// ILexer
	void SCI_METHOD Release() override;
	int SCI_METHOD Version() const override;
	const char* SCI_METHOD PropertyNames() override;
	int SCI_METHOD PropertyType(const char* name) override;
	const char* SCI_METHOD DescribeProperty(const char* name) override;
	int SCI_METHOD PropertySet(const char* key, const char* val) override;
	const char* SCI_METHOD DescribeWordListSets() override;
	int SCI_METHOD WordListSet(int n, const char* wl) override;
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument* pAccess) override;
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess) override;
	void* SCI_METHOD PrivateCall(int operation, void* pointer) override;	

private:
	enum TextState
	{
		TS_DEFAULT,
		TS_COMMENT,
		TS_KEYWORD,
		TS_VALUE,
		TS_LINEEND
	};
	
	enum TextColor
	{
		TC_DEFAULT,
		TC_COMMENT,
		TC_KEYWORD0,
		TC_KEYWORD1,
		TC_KEYWORD2,
		TC_KEYWORD3,
		TC_KEYWORD4,
		TC_OPERATOR,
		TC_NUMBER,
		TC_FOLDER_OPEN,
		TC_FOLDER_CLOSE
	};

	WordList m_WordLists[7];
};

}	// namespace ConTeXt

#endif
