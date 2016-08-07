#pragma once

#include <string>
#include <vector>
#include <cassert>

using namespace std;

template<class T> bool INRANGE(T x, T a, T b) { return a <= x&&x <= b; }
template<class T> T SQ(T x) { return x*x; }
template<class T> T SQSUM(T x, T y) { return x*x + y*y; }


#define SZ(a) ((int)a.size())
const int NG = -1;

enum ESengo
{
	E_SEN,
	E_GO,
	NUM_SEN_GO,

	E_NO_SENGO = -1,
};

enum EKomaType
{
	E_OU,
	E_HI,
	E_KAKU,
	E_KIN,
	E_GIN,
	E_KEI,
	E_KYO,
	E_FU,
	NUM_NARAZU_KOMA_TYPE,

	E_RYU = NUM_NARAZU_KOMA_TYPE,
	E_UMA,
	E_NGIN,
	E_NKEI,
	E_NKYO,
	E_TO,
	NUM_KOMA_TYPE,

	E_EMPTY = -1,
};

enum EInputState
{
	E_IDLE,
	E_GRABBED,
	E_UTSU,
	E_CHOICE,
};

struct Masu
{
	Masu() : type(E_EMPTY), sengo(E_NO_SENGO) {}
	EKomaType type;
	ESengo sengo;
};


const int BOARD_SIZE = 9;

struct Koma
{
	string notation[NUM_SEN_GO];
	wstring jap[NUM_SEN_GO];
	EKomaType narigoma;
	EKomaType motogoma;
};


struct GridPos
{
	GridPos() : x(0), y(0) {}
	GridPos(int ay, int ax) : x(ax), y(ay) {}

	void Set(const string& s) 
	{
		// http://www.geocities.jp/shogidokoro/usi.html ���
		// ���ɁA�w����̕\�L�ɂ��ĉ�����܂��B
		// �؂Ɋւ��Ă͂P����X�܂ł̐����ŕ\�L����A
		// �i�Ɋւ��Ă�a����i�܂ł̃A���t�@�x�b�g�i�P�i�ڂ�a�A�Q�i�ڂ�b�A�E�E�E�A�X�i�ڂ�i�j�Ƃ����悤�ɕ\�L����܂��B
		// �ʒu�̕\�L�́A���̂Q��g�ݍ��킹�܂��B�T��Ȃ�5a�A�P��Ȃ�1i�ƂȂ�܂��B
		assert(SZ(s) == 2);
		x = s[0] - '1';
		y = s[1] - 'a';

		assert(INRANGE(x, 0, BOARD_SIZE - 1));
		assert(INRANGE(y, 0, BOARD_SIZE - 1));
	}

	string Get() const
	{
		string ret;
		ret += string(1, static_cast<char>(x + '1'));
		ret += string(1, static_cast<char>(y + 'a'));
		assert(SZ(ret) == 2);
		return ret;
	}

	bool operator==(const GridPos & n) const
	{
		return this->x == n.x && this->y == n.y;
	}

	bool operator!=(const GridPos & n) const
	{
		return !(n == *this);
	}
	int x;
	int y;
};

struct Move
{
	GridPos from;
	GridPos to;
	bool naru;
	EKomaType utsuKomaType;

	Move()
	{
		Init();
	}

	void Init()
	{
		from.y = 0;
		from.x = 0;
		to.y = 0;
		to.x = 0;
		naru = false;
		utsuKomaType = E_EMPTY;
	}
};


class Board
{
public:
	Board();
	virtual ~Board() {};
	virtual void Draw() {};
	virtual bool Update(string& /* te */, wstring& /* teJap */) { return false; };
	virtual int CalcBestMoveAndScore() { return 0; }

	void InitState();

	wstring MoveByTejun(const string& tejun);
	wstring MoveByTe(const string& te);

	wstring DecideMove();

	void SetState(const string& state);

	string GetState() const;

	string GetTeFromPSN(const string& tePSN) const;
	string GetTejunFromPSN(const string& tejunPSN) const;
	string GetTeFromMove(const Move& mv) const;
	Move GetMoveFromTe(const string& te) const;


	void InitNextMove() { mNextMove.Init(); }
	void RemoveCharsFromString(string &str, char* charsToRemove) const;

	// setter, getter
	void SetMoveFromPos(const GridPos& gp) { mNextMove.from = gp; }
	const GridPos& GetMoveFromPos() const { return mNextMove.from; }

	void SetMoveToPos(const GridPos& gp) { mNextMove.to = gp; }
	const GridPos& GetMoveToPos() const { return mNextMove.to; }

	void SetUtsuKomaType(EKomaType utsuKomaType ) { mNextMove.utsuKomaType = utsuKomaType; }
	EKomaType GetUtsuKomaType() const { return mNextMove.utsuKomaType; }

	void SetNaru(bool bNaru) { mNextMove.naru = bNaru; }

	const Move& GetNextMove() const { return mNextMove; }


	const Masu& GetMasu(const GridPos& gp) const
	{
		return GetMasu(gp.y, gp.x);
	}

	const Masu& GetMasu(int y, int x) const 
	{
		assert(INRANGE(y, 0, BOARD_SIZE - 1));
		assert(INRANGE(x, 0, BOARD_SIZE - 1));
		return mGrid[y][x];
	}

	int GetNumMochigoma(int teban, int komaType) const
	{
		assert(INRANGE(teban, 0, NUM_SEN_GO-1) );
		assert(INRANGE(komaType, 0, NUM_NARAZU_KOMA_TYPE-1) );
		return mMochigoma[teban][komaType];
	}
	ESengo GetTeban() const { return mTeban; }

	int mScore;
	vector <string> mYomisuji;

protected:
	bool IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const;

	const vector <Koma> mKoma =
	{
		{ { "K", "k" },{ L"��", L"��" }, E_EMPTY, E_OU }, // ���̃\�t�g�ɂ��킹�ė����ʂɂ��Ă܂�
		{ { "R", "r" },{ L"��", L"��" }, E_RYU  , E_HI },
		{ { "B", "b" },{ L"�p", L"�p" }, E_UMA, E_KAKU },
		{ { "G", "g" },{ L"��", L"��" }, E_EMPTY, E_KIN },
		{ { "S", "s" },{ L"��", L"��" }, E_NGIN, E_GIN },
		{ { "N", "n" },{ L"�j", L"�j" }, E_NKEI, E_KEI },
		{ { "L", "l" },{ L"��", L"��" }, E_NKYO, E_KYO },
		{ { "P", "p" },{ L"��", L"��" }, E_TO, E_FU },

		{ { "+R", "+r" },{ L"��", L"��" }, E_EMPTY, E_HI },
		{ { "+B", "+b" },{ L"�n", L"�n" }, E_EMPTY, E_KAKU },
		{ { "+S", "+s" },{ L"����", L"����" }, E_EMPTY, E_GIN },
		{ { "+N", "+n" },{ L"���j", L"���j" }, E_EMPTY, E_KEI },
		{ { "+L", "+l" },{ L"����", L"����" },  E_EMPTY, E_KYO },
		{ { "+P", "+p" },{ L"��", L"��" }, E_EMPTY, E_FU },
	};

	const wstring mKomaMark[NUM_SEN_GO] = { L"��", L"��" };
	const wstring mJapX[BOARD_SIZE] = { L"�P", L"�Q", L"�R", L"�S", L"�T", L"�U", L"�V", L"�W", L"�X" };
	const wstring mJapY[BOARD_SIZE] = { L"��", L"��", L"�O", L"�l", L"��", L"�Z", L"��", L"��", L"��" };
	const wstring mJapSemiNumber[BOARD_SIZE] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
	const wstring mJapDou  = { L"��" };
	const wstring mJapUtsu = { L"��" };
	const wstring mJapNaru = { L"��" };

private:
	bool IsTekijin(int y, ESengo teban) const;
	bool IsBanjyo(const GridPos& gp) const;
	bool IsBanjyo(int y, int x) const;

	vector < vector <Masu> > mGrid;
	vector < vector <int> > mMochigoma;
	ESengo mTeban;

	Move mNextMove;
	vector <Move> mMoves;
};
