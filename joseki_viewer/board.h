#pragma once

#include <string>
#include <vector>
#include <cassert>

using namespace std;

template<class T> bool INRANGE(T x, T a, T b) { return a <= x&&x <= b; }
#define SZ(a) ((int)a.size())
const int NG = -1;

enum ESengo
{
	E_SEN,
	E_GO,
	NUM_SEN_GO,
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
	E_HARU,
	E_CHOICE,
};

struct Masu
{
	Masu() : type(E_EMPTY), sengo(E_SEN) {}
	EKomaType type;
	ESengo sengo;
};


const int BOARD_SIZE = 9;

struct Koma
{
	string notation[NUM_SEN_GO];
	EKomaType narigoma;
	EKomaType motogoma;
};


struct GridPos
{
	GridPos() : x(0), y(0) {}
	GridPos(int ay, int ax) : x(ax), y(ay) {}

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
};


class Board
{
public:
	Board();
	virtual ~Board() {};
	virtual void Draw() = 0;
	virtual void Update() = 0;

	bool IsTekijin(int y, ESengo teban) const;
	bool IsBanjyo(const GridPos& gp) const;
	bool IsBanjyo(int y, int x) const;
	bool IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const;
	void DecideMove(bool isNaru);
	void SetSFEN(const string& sfen);
	void Split1(const string& str, vector<string>& out, const char splitter = ' ') const;

	// setter, getter
	void SetMoveFromPos(const GridPos& gp) { mMoveFromPos = gp; }
	const GridPos& GetMoveFromPos() const { return mMoveFromPos; }

	void SetMoveToPos(const GridPos& gp) { mMoveToPos = gp; }
	const GridPos& GetMoveToPos() const { return mMoveToPos; }

	void SetHaruKomaType(EKomaType haruKomaType ) { mHaruKomaType = haruKomaType; }
	EKomaType GetHaruKomaType() const { return mHaruKomaType; }

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

protected:

	// 文字列分割
	// 空文字列ありのときも考慮すること。



	const vector <Koma> mKoma =
	{
		{ { "K", "k" }, E_EMPTY, E_OU },
		{ { "R", "r" }, E_RYU  , E_HI },
		{ { "B", "b" }, E_UMA, E_KAKU },
		{ { "G", "g" }, E_EMPTY, E_KIN },
		{ { "S", "s" }, E_NGIN, E_GIN },
		{ { "N", "n" }, E_NKEI, E_KEI },
		{ { "L", "l" }, E_NKYO, E_KYO },
		{ { "P", "p" }, E_TO, E_FU },

		{ { "+R", "+r" }, E_EMPTY, E_HI },
		{ { "+B", "+b" }, E_EMPTY, E_KAKU },
		{ { "+S", "+s" }, E_EMPTY, E_GIN },
		{ { "+N", "+n" }, E_EMPTY, E_KEI },
		{ { "+L", "+l" }, E_EMPTY, E_KYO },
		{ { "+P", "+p" }, E_EMPTY, E_FU },
	};


private:
	GridPos mMoveFromPos;
	GridPos mMoveToPos;
	EKomaType mHaruKomaType = E_EMPTY;

	vector < vector <Masu> > mGrid;
	vector <int> mMochigoma[NUM_SEN_GO];
	ESengo mTeban;
	vector <Move> mMoves;
};
