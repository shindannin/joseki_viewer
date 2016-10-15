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
		next = tmp.find(splitter, st); // tmp.find_first_of("+-",st); 複数の文字で分けたいとき
		string word = tmp.substr(st, next - st);
		if (word.length() >= 1) // 空文字列ありのときは消す
		{
			out.push_back(word);
		}
		st = next + 1;
	} while (next != string::npos);
}

// 改行エスケープを追加し、コメントをセーブ可能にする。
wstring EasyEscape(const wstring& ws)
{
	wstring ret;
	for (const auto& wc : ws)
	{
		if (wc == '\n')
		{
			ret.push_back('\\n');
		}
		else
		{
			ret.push_back(wc);
		}
	}
	return ret;
}

wstring EasyUnescape(const wstring& ws)
{
	wstring ret;
	for (int i=0;i<ws.size();++i)
	{
		if( ws[i]=='\\n')
		{
			ret.push_back('\n');
		}
		else
		{
			ret.push_back(ws[i]);
		}
	}
	return ret;
}


void GetLineTrim(wfstream& wfs, wstring& ws)
{
	getline(wfs, ws);
	if (!ws.empty() && ws[ws.size() - 1] == '\r')
	{
		ws.erase(ws.size() - 1);
	}
}

void Trim(wstring& ws)
{
	if (!ws.empty() && ws[ws.size() - 1] == '\r')
	{
		ws.erase(ws.size() - 1);
	}
}

void Trim(string& s)
{
	if (!s.empty() && s[s.size() - 1] == '\r')
	{
		s.erase(s.size() - 1);
	}
}

wstring AddNewLine(const wstring& ws, int perNumTe)
{
	wstring ret;
	int numTe = 0;
	for (int i = 0; i < ws.size(); ++i)
	{
		ret += ws[i];
		if (i + 1 < ws.size() && (ws[i + 1] == L'▲' || ws[i + 1] == L'△'))
		{
			numTe++;
			if (numTe%perNumTe == 0)
			{
				ret += L'\n';
			}
		}

	}

	return ret;
}