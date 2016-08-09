#pragma once

#include <vector>
#include <string>

#define WINDOW_W	(1280)
#define WINDOW_H	 (720)

using namespace std;

extern void Split1(const string& str, vector<string>& out, const char splitter = ' ');
extern void GetLineTrim(wfstream& wfs, wstring& ws);

