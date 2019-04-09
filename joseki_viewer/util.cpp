#include "util.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> 

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

void Split1(const wstring& str, vector<wstring>& out, const wchar_t splitter)
{
	out.clear();
	wstring::size_type st = 0;
	wstring::size_type next = 0;
	wstring tmp = str;
	do
	{
		next = tmp.find(splitter, st); // tmp.find_first_of("+-",st); �����̕����ŕ��������Ƃ�
		wstring word = tmp.substr(st, next - st);
		if (word.length() >= 1) // �󕶎��񂠂�̂Ƃ��͏���
		{
			out.push_back(word);
		}
		st = next + 1;
	} while (next != wstring::npos);
}


// �ȉ��̃G�X�P�[�v�V�[�P���X���Ă��Ɓ`�B�Ⴆ�Ε�����Ƀo�b�N�X���b�V�����������玸�s�������Ȃ̂Œ����B
// ���s�G�X�P�[�v�A�R�����g���Z�[�u�\�ɂ���B
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

// ���s�G�X�P�[�v�����ɖ߂��A�R�����g�����[�h�\�ɂ���B
wstring EasyUnescape(const wstring& ws)
{
	wstring ret;
	for (int i=0;i<static_cast<int>(ws.size());++i)
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

int CalcOrder(float x, float start_x, float w)
{
	return static_cast<int>((x - start_x + w * 0.5) / w);
}

wstring AddNewLine(const wstring& ws, int perNumTe)
{
	wstring ret;
	int numTe = 0;
	for (int i = 0; i < static_cast<int>(ws.size()); ++i)
	{
		ret += ws[i];
		if (i + 1 < static_cast<int>(ws.size()) && (ws[i + 1] == L'��' || ws[i + 1] == L'��'))
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

void RemoveCharsFromString(string &str, char* charsToRemove)
{
	for (unsigned int i = 0; i < strlen(charsToRemove); ++i) {
		str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
	}
}

wstring GetVersionTitle(const wstring& jsvPathStr)
{
	wstring ret = VERSION_TITLE + jsvPathStr;
	return ret;
}
