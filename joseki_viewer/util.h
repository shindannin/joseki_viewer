#pragma once

#include <vector>
#include <string>

#define WINDOW_W	(1280)
#define WINDOW_H	 (720)

#define RIGHT_CENTER_X (WINDOW_W*0.75f)
#define RIGHT_CENTER_Y (WINDOW_H*0.5f)

#define VERSION_TITLE L"Siv3D 定跡ビューア Ver 0.02 (2016/11/27)"

using namespace std;

extern void Split1(const string& str, vector<string>& out, const char splitter = ' ');
extern wstring EasyEscape(const wstring& ws);
extern wstring EasyUnescape(const wstring& ws);
extern void GetLineTrim(wfstream& wfs, wstring& ws);
extern void Trim(wstring& ws);
extern void Trim(string& s);
extern wstring AddNewLine(const wstring& ws, int perNumTe);
extern void RemoveCharsFromString(string &str, char* charsToRemove);
extern wstring GetVersionTitle(const wstring& jsvPathStr);
