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
	EKomaType haruKomaType;

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
		haruKomaType = E_EMPTY;
	}
};


class Board
{
public:
	Board();
	virtual ~Board() {};
	virtual void Draw() {};
	virtual void Update() {};

	void DecideMove(bool isNaru);
	void SetSFEN(const string& sfen);
	string GetSFEN() const;
	void InitNextMove() { mNextMove.Init(); }
	void Split1(const string& str, vector<string>& out, const char splitter = ' ') const;

	// setter, getter
	void SetMoveFromPos(const GridPos& gp) { mNextMove.from = gp; }
	const GridPos& GetMoveFromPos() const { return mNextMove.from; }

	void SetMoveToPos(const GridPos& gp) { mNextMove.to = gp; }
	const GridPos& GetMoveToPos() const { return mNextMove.to; }

	void SetHaruKomaType(EKomaType haruKomaType ) { mNextMove.haruKomaType = haruKomaType; }
	EKomaType GetHaruKomaType() const { return mNextMove.haruKomaType; }

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
	bool IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const;

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
	bool IsTekijin(int y, ESengo teban) const;
	bool IsBanjyo(const GridPos& gp) const;
	bool IsBanjyo(int y, int x) const;

	vector < vector <Masu> > mGrid;
	vector < vector <int> > mMochigoma;
	ESengo mTeban;

	Move mNextMove;
	vector <Move> mMoves;
};
