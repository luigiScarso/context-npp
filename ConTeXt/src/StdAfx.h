#ifndef ConTeXt_STDAFX_H_
#define ConTeXt_STDAFX_H_

#include <Windows.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <list>
#include <memory>
#include <atlstr.h>
using namespace std;
typedef  std::basic_string<TCHAR>	tstring;
typedef size_t idx_t;
typedef unsigned int UINT;
struct Constants
{
	static const string SEP;
};


#define IDX_MAX SIZE_MAX
#define MaxKeyLen 50
#define MaxValueLen 1024
#define MaxBuffLen 1024
#define MaxLineLen 2049
#define MaxFuncs 1000
#define MaxIndent 80
#define KeyNotFoundMsgLen 40
#define MaxLexerNameLen 100


#endif
