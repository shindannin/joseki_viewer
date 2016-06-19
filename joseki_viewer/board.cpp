#include <cassert>
#include <algorithm>

#include "board.h"

//先手の玉：K、後手の玉：k （Kingの頭文字）
//先手の飛車：R、後手の飛車：r （Rookの頭文字）
//先手の角：B、後手の角：b （Bishopの頭文字）
//先手の金：G、後手の金：g （Goldの頭文字）
//先手の銀：S、後手の銀：s （Silverの頭文字）
//先手の桂馬：N、後手の桂馬：n （kNightより）
//先手の香車：L、後手の香車：l （Lanceの頭文字）
//先手の歩：P、後手の歩：p （Pawnの頭文字）




Board::Board()
{
	SetState("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
	mScore = 0;
}

bool Board::IsTekijin(int y, ESengo teban) const
{
	if (teban == E_SEN && INRANGE(y, 0, 2)) return true;
	if (teban == E_GO  && INRANGE(y, BOARD_SIZE - 3, BOARD_SIZE - 1)) return true;

	return false;
}

bool Board::IsBanjyo(const GridPos& gp) const
{
	return IsBanjyo(gp.y, gp.x);
}

bool Board::IsBanjyo(int y, int x) const
{
	return INRANGE(y, 0, BOARD_SIZE - 1) && INRANGE(x, 0, BOARD_SIZE - 1);
}



bool Board::IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const
{
	const EKomaType k = mGrid[from.y][from.x].type;

	// 盤上じゃないとダメ
	if (!IsBanjyo(from)) return false;
	if (!IsBanjyo(to)) return false;

	// つかんでいる駒がなれる駒でないとダメ
	if (mKoma[k].narigoma == E_EMPTY)
	{
		return false;
	}

	// 移動元か移動先が敵陣でないとダメ
	if (!IsTekijin(from.y, teban) && !IsTekijin(to.y, teban))
	{
		return false;
	}

	// TODO : 移動可能

	return true;
}

void Board::DecideMove()
{
	if (mNextMove.utsuKomaType != E_EMPTY)
	{
		// 駒をはる
		mMochigoma[mTeban][GetUtsuKomaType()]--;
		mGrid[mNextMove.to.y][mNextMove.to.x].sengo = mTeban;
		mGrid[mNextMove.to.y][mNextMove.to.x].type = mNextMove.utsuKomaType;
	}
	else
	{
		// 移動先に駒がある場合は取る
		if (mGrid[mNextMove.to.y][mNextMove.to.x].type != E_EMPTY)
		{
			const int s = 1 - mGrid[mNextMove.to.y][mNextMove.to.x].sengo;
			const int k = mKoma[mGrid[mNextMove.to.y][mNextMove.to.x].type].motogoma;
			mMochigoma[s][k]++;
		}

		// 移動する
		{
			mGrid[mNextMove.to.y][mNextMove.to.x] = mGrid[mNextMove.from.y][mNextMove.from.x];
			if (mNextMove.naru)
			{
				mGrid[mNextMove.to.y][mNextMove.to.x].type = mKoma[mGrid[mNextMove.to.y][mNextMove.to.x].type].narigoma;
			}

			mGrid[mNextMove.from.y][mNextMove.from.x].type = E_EMPTY;
			mGrid[mNextMove.from.y][mNextMove.from.x].sengo = E_NO_SENGO;
		}
	}


	// 手番の交代
	if (mTeban == E_SEN)
	{
		mTeban = E_GO;
	}
	else
	{
		mTeban = E_SEN;
	}

	mMoves.push_back(mNextMove);
	mNextMove.Init();
//	mScore = GetScore();
}

// PSN形式の手を、SFEN形式の手に変換。
// PSN形式は、先頭に駒の表記と、駒をとる取らない"-x"がついている。

string Board::GetTeFromPSN(const string& tePSN) const
{
	// 駒を打ったときの、表記法は一緒。、
	if (tePSN.find('*') != string::npos)
	{
		return tePSN;
	}

	// 駒を移動するとき
	string ret(tePSN);
	RemoveCharsFromString(ret, "-x");


	for (int i = 0; i < SZ(ret); ++i)
	{
		if (INRANGE(ret[i], '1', '9'))
		{
			ret = ret.substr(i);
			break;
		}
	}
	return ret;
}

string Board::GetTejunFromPSN(const string& tejunPSN) const
{
	vector <string> vs;
	Split1(tejunPSN, vs);

	string ret;
	for (const string& tePSN : vs)
	{
		ret += GetTeFromPSN(tePSN);
		ret += " ";
	}
	ret.pop_back();
	return ret;
}

void Board::MoveByTejun(const string& tejun)
{
	vector <string> vs;
	Split1(tejun, vs);
	for (const string& te : vs)
	{
		MoveByTe(te);
	}
}

void Board::MoveByTe(const string& te)
{
	InitNextMove();

	// http://www.geocities.jp/shogidokoro/usi.html より
	// 次に、指し手の表記について解説します。
	// 筋に関しては１から９までの数字で表記され、
	// 段に関してはaからiまでのアルファベット（１段目がa、２段目がb、・・・、９段目がi）というように表記されます。
	// 位置の表記は、この２つを組み合わせます。５一なら5a、１九なら1iとなります。
	// そして、指し手に関しては、駒の移動元の位置と移動先の位置を並べて書きます。
	// ７七の駒が７六に移動したのであれば、7g7fと表記します。（駒の種類を表記する必要はありません。）
	// 
	// 駒が成るときは、最後に + を追加します。８八の駒が２二に移動して成るなら8h2b + と表記します。
	// 持ち駒を打つときは、最初に駒の種類を大文字で書き、それに*を追加し、さらに打った場所を追加します。金を５二に打つ場合はG * 5bとなります


	if (te.find('*') == string::npos)
	{
		// 通常の移動
		assert(INRANGE(SZ(te), 4, 5));
		mNextMove.from.Set(te.substr(0, 2));
		mNextMove.to.Set(te.substr(2, 2));
		if (SZ(te)==5 && te[4]=='+')
		{
			mNextMove.naru = true;
		}
	}
	else
	{
		// 持ち駒を打つとき
		assert(SZ(te)==4);

		bool isFound = false;
		for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
		{
			if (mKoma[k].notation[0] == string(1,te[0]))
			{
				mNextMove.utsuKomaType = static_cast<EKomaType>(k);
				isFound = true;
				break;
			}
		}
		assert(isFound);

		mNextMove.to.Set(te.substr(2, 2));
	}

	DecideMove();
}

wstring Board::GetTejunJap(const string& state, const string& tejun) const
{
	Board* board = new Board;

	board->SetState(state);

	vector <string> yomi;
	Split1(tejun, yomi);



	delete board;

	return L"▲１一玉"; // てきと～
}

void Board::SetState(const string& state)
{
	vector <string> splitted;
	Split1(state, splitted);

	// 盤面
	{
		const string& banmen = splitted[0];

		int y = 0;
		int x = 0;
		int i = 0;

		mGrid = vector < vector <Masu> >(BOARD_SIZE, vector <Masu>(BOARD_SIZE));
		while (i < SZ(banmen))
		{
			if (banmen[i] == '/')
			{
				y++;
				x = 0;
				i++;
			}
			else if (INRANGE(banmen[i], '0', '9'))
			{
				x += banmen[i] - '0';
				i++;
			}
			else
			{
				for (int s = 0; s < NUM_SEN_GO; ++s)
				{
					for (int k = 0; k < NUM_KOMA_TYPE; ++k)
					{
						const string& nt = mKoma[k].notation[s];
						if (banmen.substr(i, SZ(nt)) == nt)
						{
							mGrid[y][BOARD_SIZE - 1 - x].type = static_cast<EKomaType>(k);
							mGrid[y][BOARD_SIZE - 1 - x].sengo = static_cast<ESengo>(s);
							x += 1;
							i += SZ(nt);
							goto NUKE;
						}
					}
				}
			}

		NUKE:;
		}
	}

	// 手番
	// 次の手番については、先手番ならb、後手番ならwと表記します。（Black、Whiteの頭文字）
	mTeban = E_SEN;
	if (splitted[1] == "b")
	{
		mTeban = E_SEN;
	}
	else if (splitted[1] == "w")
	{
		mTeban = E_GO;
	}
	else
	{
		assert(0);
	}

	// 持ち駒
	{
		mMochigoma = vector < vector <int> > (NUM_SEN_GO, vector <int> (NUM_NARAZU_KOMA_TYPE) );

		const string& line = splitted[2];

		// 持ち駒については、先手後手のそれぞれの持ち駒の種類と、その枚数を表記します。
		// 枚数は、２枚以上であれば、駒の種類の前にその数字を表記します。
		// 先手側が銀１枚歩２枚、後手側が角１枚歩３枚であれば、S2Pb3pと表記されます。
		// どちらも持ち駒がないときは - （半角ハイフン）を表記します。
		if (line != "-")
		{
			int maisu = 1;
			for (int i = 0; i < SZ(line); ++i)
			{
				const char c = line[i];
				if (isupper(c) || islower(c))
				{
					ESengo s = E_SEN;
					if (islower(c))
					{
						s = E_GO;
					}

					for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
					{
						const string& nt = mKoma[k].notation[s];
						if (c == nt[0])
						{
							mMochigoma[s][k] = maisu;
							break;
						}
					}
					maisu = 1;
				}
				else if (isdigit(c))
				{
					maisu = c - '0';
				}
				else
				{
					assert(0);
				}
			}
		}
	}

	// 履歴のクリア
	mMoves.clear();
}

string Board::GetState() const
{
	string ret;

	// "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"

	// 盤面
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		int numEmpties = 0;

		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			const Masu& masu = mGrid[y][BOARD_SIZE-1-x];
			if (masu.type == E_EMPTY)
			{
				numEmpties++;
			}
			else
			{
				if (numEmpties >= 1)
				{
					ret += string(1, static_cast<char>('0' + numEmpties));
					numEmpties = 0;
				}

				ret += mKoma[masu.type].notation[masu.sengo];
			}
		}

		if (numEmpties >= 1)
		{
			ret += string(1, static_cast<char>('0' + numEmpties));
		}

		if (y < BOARD_SIZE - 1)
		{
			ret += "/";
		}
	}

	ret += " ";

	// 手番
	if (mTeban==E_SEN)
	{
		ret += "b";
	}
	else if (mTeban == E_GO)
	{
		ret += "w";
	}
	else
	{
		assert(0);
	}

	ret += " ";

	// 持ち駒
	{
		string str;

		// 持ち駒については、先手後手のそれぞれの持ち駒の種類と、その枚数を表記します。
		// 枚数は、２枚以上であれば、駒の種類の前にその数字を表記します。
		// 先手側が銀１枚歩２枚、後手側が角１枚歩３枚であれば、S2Pb3pと表記されます。
		// どちらも持ち駒がないときは - （半角ハイフン）を表記します。

		for (int s = 0; s < NUM_SEN_GO; ++s)
		{
			for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
			{
				if (mMochigoma[s][k] >= 2)
				{
					str += string(1, static_cast<char>('0' + mMochigoma[s][k]));
				}

				if (mMochigoma[s][k] >= 1)
				{
					str += mKoma[k].notation[s];
				}
			}
		}

		if (str == "")
		{
			str = "-";
		}

		ret += str;
	}

	ret += " 1";

	return ret;
}

void Board::Split1(const string& str, vector<string>& out, const char splitter) const
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

void Board::RemoveCharsFromString(string &str, char* charsToRemove) const
{
	for (unsigned int i = 0; i < strlen(charsToRemove); ++i) {
		str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
	}
}
