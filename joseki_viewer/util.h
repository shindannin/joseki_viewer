#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

template<class T> bool INRANGE(T x, T a, T b) { return a <= x&&x <= b; }
#define SZ(a) ((int)a.size())
const int NG = -1;
#define WINDOW_W	(1280)
#define WINDOW_H	 (720)

#define RIGHT_CENTER_X (WINDOW_W*0.75f)
#define RIGHT_CENTER_Y (WINDOW_H*0.5f)

const int mShogibanWidth = 613;
const int mShogibanHeight = 523;


#define VERSION_TITLE L"Siv3D ’èÕƒrƒ…[ƒA Ver 0.04 (2019/04/01)  "
#define VERSION_DATA (2)

using namespace std;

extern void Split1(const string& str, vector<string>& out, const char splitter = ' ');
extern void Split1(const wstring& str, vector<wstring>& out, const wchar_t splitter = L' ');
extern wstring EasyEscape(const wstring& ws);
extern wstring EasyUnescape(const wstring& ws);
extern void GetLineTrim(wfstream& wfs, wstring& ws);
extern void Trim(wstring& ws);
extern void Trim(string& s);
extern wstring AddNewLine(const wstring& ws, int perNumTe);
extern void RemoveCharsFromString(string &str, char* charsToRemove);
extern wstring GetVersionTitle(const wstring& jsvPathStr);
