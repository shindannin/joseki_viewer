#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

template<class T> bool INRANGE(T x, T a, T b) { return a <= x&&x <= b; }
template<class T> T CLAMP(T x, T a, T b) { return min(max(x, a), b); }

#define SZ(a) ((int)a.size())
const int NG = -1;
#define WINDOW_W	(1280)
#define WINDOW_H	 (720)

#define RIGHT_CENTER_X (WINDOW_W*0.75f)
#define RIGHT_CENTER_Y (WINDOW_H*0.5f)

const int SHOGIBAN_W = 613;
const int SHOGIBAN_H = 523;

const int SCORE_GRAPH_W = 550;
const int SCORE_GRAPH_H = 200;
const int SCORE_GRAPH_X = WINDOW_W - SCORE_GRAPH_W;
const int SCORE_GRAPH_Y = WINDOW_H - SCORE_GRAPH_H;
const int SCORE_GRAPH_SW = 4;

const int DEFAULT_EVALUATION_SEC = 2;



#define VERSION_TITLE L"Siv3D è´ä˚ÉrÉÖÅ[ÉA Ver 0.05 (2019/04/12)  "
#define VERSION_DATA (2)

using namespace std;

extern void Split1(const string& str, vector<string>& out, const char splitter = ' ');
extern void Split1(const wstring& str, vector<wstring>& out, const wchar_t splitter = L' ');
extern wstring EasyEscape(const wstring& ws);
extern wstring EasyUnescape(const wstring& ws);
extern void GetLineTrim(wfstream& wfs, wstring& ws);
extern void Trim(wstring& ws);
extern void Trim(string& s);
extern int CalcOrder(float x, float start_x, float w);
extern wstring AddNewLine(const wstring& ws, int perNumTe);
extern void RemoveCharsFromString(string &str, char* charsToRemove);
extern wstring GetVersionTitle(const wstring& jsvPathStr);
