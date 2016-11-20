#include <cassert>
#include <algorithm>
#include "util.h"

#include "board.h"

Board::Board()
{
	InitState();
	mValidMoveGrid = vector <vector <bool> > (BOARD_SIZE, vector <bool> (BOARD_SIZE) );
}

void Board::InitState()
{
	SetState("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}

// 返り値 true : 指定したy座標は、teban（先手or後手）側にとって、敵陣である。
bool Board::IsTekijin(int y, ESengo teban) const
{
	if (teban == E_SEN && INRANGE(y, 0, 2)) return true;
	if (teban == E_GO  && INRANGE(y, BOARD_SIZE - 3, BOARD_SIZE - 1)) return true;

	return false;
}

// 返り値 true : 指定したGridPos gpは、将棋盤上にある
bool Board::IsBanjyo(const GridPos& gp) const
{
	return IsBanjyo(gp.y, gp.x);
}

// 返り値 true : 指定した座標(y,x)は、将棋盤上にある
bool Board::IsBanjyo(int y, int x) const
{
	return INRANGE(y, 0, BOARD_SIZE - 1) && INRANGE(x, 0, BOARD_SIZE - 1);
}

// 返り値 true : 絶対に成らなければならない
bool Board::IsNarubeki(const GridPos& from, const GridPos& to, ESengo teban) const
{
	return (IsNareru(from, to, teban) && IsIkidomari(to.y, GetMasu(from).type, teban));
}

// 返り値 true : 成ることができる
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

// 手が決定した後の処理。駒をとったり手番を進めたりする。
// 返り値  : 日本語表記の手
wstring Board::DecideMove()
{
	// 指し手の日本語名の作成
	wstring teJap;

	// ▲△
	teJap += mKomaMark[mTeban];

	if (SZ(mMoves)>=1 && mNextMove.to == mMoves.back().to)
	{
		// 前の手と一緒→同
		teJap += mJapDou;
	}
	else
	{
		// マスの表記（７六とか）
		teJap += mJapX[mNextMove.to.x] + mJapY[mNextMove.to.y];
	}

	if (mNextMove.utsuKomaType != E_EMPTY) // // 駒を打つ
	{
		teJap += mKoma[mNextMove.utsuKomaType].jap[mTeban] + mJapUtsu;
	}
	else // 移動する
	{
		teJap += mKoma[mGrid[mNextMove.from.y][mNextMove.from.x].type].jap[mTeban];

		// 成る
		if (mNextMove.naru)
		{
			teJap += mJapNaru;
		}

		// 駒の移動元
		teJap += L"(" + mJapSemiNumber[mNextMove.from.x] + mJapSemiNumber[mNextMove.from.y] + L")";
	}


	if (mNextMove.utsuKomaType != E_EMPTY)
	{
		// 駒を打つ
		mMochigoma[mTeban][mNextMove.utsuKomaType]--;
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

	return teJap;
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

string Board::GetTeFromMove(const Move& mv) const
{
	string te;

	if (mv.utsuKomaType == E_EMPTY)
	{
		// 通常の移動
		te += mv.from.Get();
		te += mv.to.Get();
		if (mv.naru)
		{
			te += "+";
		}
		assert(INRANGE(SZ(te), 4, 5));
	}
	else
	{
		// 持ち駒を打つとき
		te += mKoma[mv.utsuKomaType].notation[0];
		te += "*";
		te += mv.to.Get();
		assert(SZ(te) == 4);
	}

	return te;
}

Move Board::GetMoveFromTe(const string& te) const
{
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

	Move mv;

	if (te.find('*') == string::npos)
	{
		// 通常の移動
		assert(INRANGE(SZ(te), 4, 5));
		mv.from.Set(te.substr(0, 2));
		mv.to.Set(te.substr(2, 2));
		if (SZ(te) == 5 && te[4] == '+')
		{
			mv.naru = true;
		}
	}
	else
	{
		// 持ち駒を打つとき
		assert(SZ(te) == 4);

		bool isFound = false;
		for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
		{
			if (mKoma[k].notation[0] == string(1, te[0]))
			{
				mv.utsuKomaType = static_cast<EKomaType>(k);
				isFound = true;
				break;
			}
		}
		assert(isFound);

		mv.to.Set(te.substr(2, 2));
	}

	return mv;
}

wstring Board::MoveByTejun(const string& tejun)
{
	wstring tejunJap;
	vector <string> vs;
	Split1(tejun, vs);
	for (const string& te : vs)
	{
		tejunJap += MoveByTe(te);
	}

	return tejunJap;
}

wstring Board::MoveByTe(const string& te)
{
	mNextMove = GetMoveFromTe(te);
	return DecideMove();
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


// 駒を動かす際に、駒の動かせる場所（移動先）を、mValidMoveGridに生成する。
void Board::GenerateValidMoveGridGrabbed()
{
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			mValidMoveGrid[y][x] = false;
		}
	}

	const Masu& masu = GetMasu(mNextMove.from);

	const vector <GridPos>& md = mKoma[masu.type].movableDirections;
	for (GridPos delta : md)
	{
		if (GetTeban() == E_GO)
		{
			delta.y = -delta.y;
		}

		const GridPos nextMoveTo = mNextMove.from + delta;
		if (IsBanjyo(nextMoveTo) &&
			GetMasu(nextMoveTo).sengo != GetTeban())// 自分の駒のあるところには動かせない。
		{
			mValidMoveGrid[nextMoveTo.y][nextMoveTo.x] = true;
		}
	}

	const vector <GridPos>& fd = mKoma[masu.type].flyingDirections;
	for (GridPos delta : fd)
	{
		if (GetTeban() == E_GO)
		{
			delta.y = -delta.y;
		}
		GridPos nextMoveTo = mNextMove.from + delta;

		while (IsBanjyo(nextMoveTo) && GetMasu(nextMoveTo).sengo != GetTeban())
		{
			mValidMoveGrid[nextMoveTo.y][nextMoveTo.x] = true;

			if (GetMasu(nextMoveTo).sengo != GetTeban() && GetMasu(nextMoveTo).sengo != E_NO_SENGO)
			{
				break;
			}
			nextMoveTo += delta;
		}
	}
}

// 駒を打つ際に、駒を打てる場所を、mValidMoveGridに生成する。
void Board::GenerateValidMoveGridUtsu()
{
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			mValidMoveGrid[y][x] = true;

			if (GetMasu(y, x).type != E_EMPTY) // 空きます以外には打てない
			{
				mValidMoveGrid[y][x] = false;
			}

			// 二歩チェック
			if (GetUtsuKomaType()==E_FU)
			{
				for (int yy = 0; yy < BOARD_SIZE; ++yy)
				{
					if (GetMasu(yy, x).sengo == GetTeban() && GetMasu(yy, x).type == E_FU)
					{
						mValidMoveGrid[y][x] = false;
						break;
					}
				}
			}

			// 移動不可能な場所にも打てない
			if (IsIkidomari(y, GetUtsuKomaType(), GetTeban()))
			{
				mValidMoveGrid[y][x] = false;
			}
		}
	}
}

// 行き止まりで、移動不可能
bool Board::IsIkidomari(int y, EKomaType type, ESengo teban) const
{
	if (type == E_FU || type == E_KYO)
	{
		if (teban == E_SEN)
		{
			if (y == 0)
			{
				return true;
			}
		}
		else if (teban == E_GO)
		{
			if (y == BOARD_SIZE - 1)
			{
				return true;
			}
		}
	}
	else if (type == E_KEI)
	{
		if (teban == E_SEN)
		{
			if (y <= 1)
			{
				return true;
			}
		}
		else if (teban == E_GO)
		{
			if (y >= BOARD_SIZE - 2)
			{
				return true;
			}
		}
	}

	return false;
}

bool Board::IsValidMove(const GridPos& gp) const
{
	return mValidMoveGrid[gp.y][gp.x];
}
