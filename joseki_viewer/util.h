#pragma once

#include <vector>
#include <string>

using namespace std;

extern void Split1(const string& str, vector<string>& out, const char splitter = ' ');
extern void GetLineTrim(wfstream& wfs, wstring& ws);

