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
		// http://www.geocities.jp/shogidokoro/usi.html ÇÊÇË
		// éüÇ…ÅAéwÇµéËÇÃï\ãLÇ…Ç¬Ç¢Çƒâê‡ÇµÇ‹Ç∑ÅB
		// ãÿÇ…ä÷ÇµÇƒÇÕÇPÇ©ÇÁÇXÇ‹Ç≈ÇÃêîéöÇ≈ï\ãLÇ≥ÇÍÅA
		// íiÇ…ä÷ÇµÇƒÇÕaÇ©ÇÁiÇ‹Ç≈ÇÃÉAÉãÉtÉ@ÉxÉbÉgÅiÇPíiñ⁄Ç™aÅAÇQíiñ⁄Ç™bÅAÅEÅEÅEÅAÇXíiñ⁄Ç™iÅjÇ∆Ç¢Ç§ÇÊÇ§Ç…ï\ãLÇ≥ÇÍÇ‹Ç∑ÅB
		// à íuÇÃï\ãLÇÕÅAÇ±ÇÃÇQÇ¬ÇëgÇ›çáÇÌÇπÇ‹Ç∑ÅBÇTàÍÇ»ÇÁ5aÅAÇPã„Ç»ÇÁ1iÇ∆Ç»ÇËÇ‹Ç∑ÅB
		assert(SZ(s) == 2);
		x = s[0] - '1';
		y = s[1] - 'a';

		assert(INRANGE(x, 0, BOARD_SIZE - 1));
		assert(INRANGE(y, 0, BOARD_SIZE - 1));
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
	virtual void Update() {};
	virtual int CalcBestMoveAndScore() { return 0; }

	wstring MoveByTejun(const string& tejun);
	wstring MoveByTe(const string& te);

	wstring DecideMove();
	void SetState(const string& state);

	string GetState() const;

	string GetTeFromPSN(const string& tePSN) const;
	string GetTejunFromPSN(const string& tejunPSN) const;


	void InitNextMove() { mNextMove.Init(); }
	void Split1(const string& str, vector<string>& out, const char splitter = ' ') const;
	void RemoveCharsFromString(string &str, char* charsToRemove) const;

	// setter, getter
	void SetMoveFromPos(const GridPos& gp) { mNextMove.from = gp; }
	const GridPos& GetMoveFromPos() const { return mNextMove.from; }

	void SetMoveToPos(const GridPos& gp) { mNextMove.to = gp; }
	const GridPos& GetMoveToPos() const { return mNextMove.to; }

	void SetUtsuKomaType(EKomaType utsuKomaType ) { mNextMove.utsuKomaType = utsuKomaType; }
	EKomaType GetUtsuKomaType() const { return mNextMove.utsuKomaType; }

	void SetNaru(bool bNaru) { mNextMove.naru = bNaru; }

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
		{ { "K", "k" },{ L"ã ", L"ã " }, E_EMPTY, E_OU }, // ëºÇÃÉ\ÉtÉgÇ…Ç†ÇÌÇπÇƒóºï˚ã Ç…ÇµÇƒÇ‹Ç∑
		{ { "R", "r" },{ L"îÚ", L"îÚ" }, E_RYU  , E_HI },
		{ { "B", "b" },{ L"äp", L"äp" }, E_UMA, E_KAKU },
		{ { "G", "g" },{ L"ã‡", L"ã‡" }, E_EMPTY, E_KIN },
		{ { "S", "s" },{ L"ã‚", L"ã‚" }, E_NGIN, E_GIN },
		{ { "N", "n" },{ L"åj", L"åj" }, E_NKEI, E_KEI },
		{ { "L", "l" },{ L"çÅ", L"çÅ" }, E_NKYO, E_KYO },
		{ { "P", "p" },{ L"ï‡", L"ï‡" }, E_TO, E_FU },

		{ { "+R", "+r" },{ L"ó¥", L"ó¥" }, E_EMPTY, E_HI },
		{ { "+B", "+b" },{ L"în", L"în" }, E_EMPTY, E_KAKU },
		{ { "+S", "+s" },{ L"ê¨ã‚", L"ê¨ã‚" }, E_EMPTY, E_GIN },
		{ { "+N", "+n" },{ L"ê¨åj", L"ê¨åj" }, E_EMPTY, E_KEI },
		{ { "+L", "+l" },{ L"ê¨çÅ", L"ê¨çÅ" },  E_EMPTY, E_KYO },
		{ { "+P", "+p" },{ L"Ç∆", L"Ç∆" }, E_EMPTY, E_FU },
	};

	const wstring mKomaMark[NUM_SEN_GO] = { L"Å£", L"Å¢" };
	const wstring mJapX[BOARD_SIZE] = { L"ÇP", L"ÇQ", L"ÇR", L"ÇS", L"ÇT", L"ÇU", L"ÇV", L"ÇW", L"ÇX" };
	const wstring mJapY[BOARD_SIZE] = { L"àÍ", L"ìÒ", L"éO", L"él", L"å‹", L"òZ", L"éµ", L"î™", L"ã„" };
	const wstring mJapSemiNumber[BOARD_SIZE] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
	const wstring mJapDou  = { L"ìØ" };
	const wstring mJapUtsu = { L"ë≈" };
	const wstring mJapNaru = { L"ê¨" };

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
