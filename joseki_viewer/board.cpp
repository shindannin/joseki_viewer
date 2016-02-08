#include  <cassert>

#include "board.h"

//���̋ʁFK�A���̋ʁFk �iKing�̓������j
//���̔�ԁFR�A���̔�ԁFr �iRook�̓������j
//���̊p�FB�A���̊p�Fb �iBishop�̓������j
//���̋��FG�A���̋��Fg �iGold�̓������j
//���̋�FS�A���̋�Fs �iSilver�̓������j
//���̌j�n�FN�A���̌j�n�Fn �ikNight���j
//���̍��ԁFL�A���̍��ԁFl �iLance�̓������j
//���̕��FP�A���̕��Fp �iPawn�̓������j




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

void Board::DecideMove(bool isNaru)
{
	if (mNextMove.haruKomaType != E_EMPTY)
	{
		// ����͂�
		mMochigoma[mTeban][GetHaruKomaType()]--;
		mGrid[mNextMove.to.y][mNextMove.to.x].sengo = mTeban;
		mGrid[mNextMove.to.y][mNextMove.to.x].type = mNextMove.haruKomaType;
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
			if (isNaru)
			{
				mGrid[mNextMove.to.y][mNextMove.to.x].type = mKoma[mGrid[mNextMove.to.y][mNextMove.to.x].type].narigoma;
			}

			mGrid[mNextMove.from.y][mNextMove.from.x].type = E_EMPTY;
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
}


void Board::SetSFEN(const string& sfen)
{
	vector <string> splitted;
	Split1(sfen, splitted);

	// �Ֆʂ̐ݒ�

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

	// ��Ԃ̐ݒ�
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

	// ������̐ݒ�
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

void Board::Split1(const string& str, vector<string>& out, const char splitter) const
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