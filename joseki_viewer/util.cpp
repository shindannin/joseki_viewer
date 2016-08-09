#include "util.h"
#include <fstream>

#define WINDOW_SIZE_W (960.f)

void Split1(const string& str, vector<string>& out, const char splitter)
{
	out.clear();
	string::size_type st = 0;
	string::size_type next = 0;
	string tmp = str;
	do
	{
		next = tmp.find(splitter, st); // tmp.find_first_of("+-",st); �����̕����ŕ��������Ƃ�
		string word = tmp.substr(st, next - st);
		if (word.length() >= 1) // �󕶎��񂠂�̂Ƃ��͏���
		{
			out.push_back(word);
		}
		st = next + 1;
	} while (next != string::npos);
}

void GetLineTrim(wfstream& wfs, wstring& ws)
{
	getline(wfs, ws);
	if (!ws.empty() && ws[ws.size() - 1] == '\r')
	{
		ws.erase(ws.size() - 1);
	}
}