#include  <cassert>

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
	SetSFEN("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
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

void Board::DecideMove(bool isNaru)
{
	if (mNextMove.haruKomaType != E_EMPTY)
	{
		// 駒をはる
		mMochigoma[mTeban][GetHaruKomaType()]--;
		mGrid[mNextMove.to.y][mNextMove.to.x].sengo = mTeban;
		mGrid[mNextMove.to.y][mNextMove.to.x].type = mNextMove.haruKomaType;
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
			if (isNaru)
			{
				mGrid[mNextMove.to.y][mNextMove.to.x].type = mKoma[mGrid[mNextMove.to.y][mNextMove.to.x].type].narigoma;
			}

			mGrid[mNextMove.from.y][mNextMove.from.x].type = E_EMPTY;
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
}


void Board::SetSFEN(const string& sfen)
{
	vector <string> splitted;
	Split1(sfen, splitted);

	// 盤面の設定

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

	// 手番の設定
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

	// 持ち駒の設定
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