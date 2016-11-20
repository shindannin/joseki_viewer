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

// �Ԃ�l true : �w�肵��y���W�́Ateban�i���or���j���ɂƂ��āA�G�w�ł���B
bool Board::IsTekijin(int y, ESengo teban) const
{
	if (teban == E_SEN && INRANGE(y, 0, 2)) return true;
	if (teban == E_GO  && INRANGE(y, BOARD_SIZE - 3, BOARD_SIZE - 1)) return true;

	return false;
}

// �Ԃ�l true : �w�肵��GridPos gp�́A�����Տ�ɂ���
bool Board::IsBanjyo(const GridPos& gp) const
{
	return IsBanjyo(gp.y, gp.x);
}

// �Ԃ�l true : �w�肵�����W(y,x)�́A�����Տ�ɂ���
bool Board::IsBanjyo(int y, int x) const
{
	return INRANGE(y, 0, BOARD_SIZE - 1) && INRANGE(x, 0, BOARD_SIZE - 1);
}

// �Ԃ�l true : ��΂ɐ���Ȃ���΂Ȃ�Ȃ�
bool Board::IsNarubeki(const GridPos& from, const GridPos& to, ESengo teban) const
{
	return (IsNareru(from, to, teban) && IsIkidomari(to.y, GetMasu(from).type, teban));
}

// �Ԃ�l true : ���邱�Ƃ��ł���
bool Board::IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const
{
	const EKomaType k = mGrid[from.y][from.x].type;

	// �Տザ��Ȃ��ƃ_��
	if (!IsBanjyo(from)) return false;
	if (!IsBanjyo(to)) return false;

	// ����ł����Ȃ���łȂ��ƃ_��
	if (mKoma[k].narigoma == E_EMPTY)
	{
		return false;
	}

	// �ړ������ړ��悪�G�w�łȂ��ƃ_��
	if (!IsTekijin(from.y, teban) && !IsTekijin(to.y, teban))
	{
		return false;
	}

	// TODO : �ړ��\

	return true;
}

// �肪���肵����̏����B����Ƃ������Ԃ�i�߂��肷��B
// �Ԃ�l  : ���{��\�L�̎�
wstring Board::DecideMove()
{
	// �w����̓��{�ꖼ�̍쐬
	wstring teJap;

	// ����
	teJap += mKomaMark[mTeban];

	if (SZ(mMoves)>=1 && mNextMove.to == mMoves.back().to)
	{
		// �O�̎�ƈꏏ����
		teJap += mJapDou;
	}
	else
	{
		// �}�X�̕\�L�i�V�Z�Ƃ��j
		teJap += mJapX[mNextMove.to.x] + mJapY[mNextMove.to.y];
	}

	if (mNextMove.utsuKomaType != E_EMPTY) // // ���ł�
	{
		teJap += mKoma[mNextMove.utsuKomaType].jap[mTeban] + mJapUtsu;
	}
	else // �ړ�����
	{
		teJap += mKoma[mGrid[mNextMove.from.y][mNextMove.from.x].type].jap[mTeban];

		// ����
		if (mNextMove.naru)
		{
			teJap += mJapNaru;
		}

		// ��̈ړ���
		teJap += L"(" + mJapSemiNumber[mNextMove.from.x] + mJapSemiNumber[mNextMove.from.y] + L")";
	}


	if (mNextMove.utsuKomaType != E_EMPTY)
	{
		// ���ł�
		mMochigoma[mTeban][mNextMove.utsuKomaType]--;
		mGrid[mNextMove.to.y][mNextMove.to.x].sengo = mTeban;
		mGrid[mNextMove.to.y][mNextMove.to.x].type = mNextMove.utsuKomaType;
	}
	else
	{
		// �ړ���ɋ����ꍇ�͎��
		if (mGrid[mNextMove.to.y][mNextMove.to.x].type != E_EMPTY)
		{
			const int s = 1 - mGrid[mNextMove.to.y][mNextMove.to.x].sengo;
			const int k = mKoma[mGrid[mNextMove.to.y][mNextMove.to.x].type].motogoma;
			mMochigoma[s][k]++;
		}

		// �ړ�����
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



	// ��Ԃ̌��
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

// PSN�`���̎���ASFEN�`���̎�ɕϊ��B
// PSN�`���́A�擪�ɋ�̕\�L�ƁA����Ƃ���Ȃ�"-x"�����Ă���B
string Board::GetTeFromPSN(const string& tePSN) const
{
	// ���ł����Ƃ��́A�\�L�@�͈ꏏ�B�A
	if (tePSN.find('*') != string::npos)
	{
		return tePSN;
	}

	// ����ړ�����Ƃ�
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
		// �ʏ�̈ړ�
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
		// �������łƂ�
		te += mKoma[mv.utsuKomaType].notation[0];
		te += "*";
		te += mv.to.Get();
		assert(SZ(te) == 4);
	}

	return te;
}

Move Board::GetMoveFromTe(const string& te) const
{
	// http://www.geocities.jp/shogidokoro/usi.html ���
	// ���ɁA�w����̕\�L�ɂ��ĉ�����܂��B
	// �؂Ɋւ��Ă͂P����X�܂ł̐����ŕ\�L����A
	// �i�Ɋւ��Ă�a����i�܂ł̃A���t�@�x�b�g�i�P�i�ڂ�a�A�Q�i�ڂ�b�A�E�E�E�A�X�i�ڂ�i�j�Ƃ����悤�ɕ\�L����܂��B
	// �ʒu�̕\�L�́A���̂Q��g�ݍ��킹�܂��B�T��Ȃ�5a�A�P��Ȃ�1i�ƂȂ�܂��B
	// �����āA�w����Ɋւ��ẮA��̈ړ����̈ʒu�ƈړ���̈ʒu����ׂď����܂��B
	// �V���̋�V�Z�Ɉړ������̂ł���΁A7g7f�ƕ\�L���܂��B�i��̎�ނ�\�L����K�v�͂���܂���B�j
	// 
	// �����Ƃ��́A�Ō�� + ��ǉ����܂��B�W���̋�Q��Ɉړ����Đ���Ȃ�8h2b + �ƕ\�L���܂��B
	// �������łƂ��́A�ŏ��ɋ�̎�ނ�啶���ŏ����A�����*��ǉ����A����ɑł����ꏊ��ǉ����܂��B�����T��ɑłꍇ��G * 5b�ƂȂ�܂�

	Move mv;

	if (te.find('*') == string::npos)
	{
		// �ʏ�̈ړ�
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
		// �������łƂ�
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

	// �Ֆ�
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

	// ���
	// ���̎�Ԃɂ��ẮA���ԂȂ�b�A���ԂȂ�w�ƕ\�L���܂��B�iBlack�AWhite�̓������j
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

	// ������
	{
		mMochigoma = vector < vector <int> > (NUM_SEN_GO, vector <int> (NUM_NARAZU_KOMA_TYPE) );

		const string& line = splitted[2];

		// ������ɂ��ẮA�����̂��ꂼ��̎�����̎�ނƁA���̖�����\�L���܂��B
		// �����́A�Q���ȏ�ł���΁A��̎�ނ̑O�ɂ��̐�����\�L���܂��B
		// ��葤����P�����Q���A��葤���p�P�����R���ł���΁AS2Pb3p�ƕ\�L����܂��B
		// �ǂ����������Ȃ��Ƃ��� - �i���p�n�C�t���j��\�L���܂��B
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

	// �����̃N���A
	mMoves.clear();
}

string Board::GetState() const
{
	string ret;

	// "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"

	// �Ֆ�
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

	// ���
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

	// ������
	{
		string str;

		// ������ɂ��ẮA�����̂��ꂼ��̎�����̎�ނƁA���̖�����\�L���܂��B
		// �����́A�Q���ȏ�ł���΁A��̎�ނ̑O�ɂ��̐�����\�L���܂��B
		// ��葤����P�����Q���A��葤���p�P�����R���ł���΁AS2Pb3p�ƕ\�L����܂��B
		// �ǂ����������Ȃ��Ƃ��� - �i���p�n�C�t���j��\�L���܂��B

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


// ��𓮂����ۂɁA��̓�������ꏊ�i�ړ���j���AmValidMoveGrid�ɐ�������B
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
			GetMasu(nextMoveTo).sengo != GetTeban())// �����̋�̂���Ƃ���ɂ͓������Ȃ��B
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

// ���łۂɁA���łĂ�ꏊ���AmValidMoveGrid�ɐ�������B
void Board::GenerateValidMoveGridUtsu()
{
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			mValidMoveGrid[y][x] = true;

			if (GetMasu(y, x).type != E_EMPTY) // �󂫂܂��ȊO�ɂ͑łĂȂ�
			{
				mValidMoveGrid[y][x] = false;
			}

			// ����`�F�b�N
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

			// �ړ��s�\�ȏꏊ�ɂ��łĂȂ�
			if (IsIkidomari(y, GetUtsuKomaType(), GetTeban()))
			{
				mValidMoveGrid[y][x] = false;
			}
		}
	}
}

// �s���~�܂�ŁA�ړ��s�\
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
