#include "util.h"

void Split1(const string& str, vector<string>& out, const char splitter)
{
	out.clear();
	string::size_type st = 0;
	string::size_type next = 0;
	string tmp = str;
	do
	{
		next = tmp.find(splitter, st); // tmp.find_first_of("+-",st); •¡”‚Ì•¶š‚Å•ª‚¯‚½‚¢‚Æ‚«
		string word = tmp.substr(st, next - st);
		if (word.length() >= 1) // ‹ó•¶š—ñ‚ ‚è‚Ì‚Æ‚«‚ÍÁ‚·
		{
			out.push_back(word);
		}
		st = next + 1;
	} while (next != string::npos);
}