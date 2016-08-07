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
		// http://www.geocities.jp/shogidokoro/usi.html より
		// 次に、指し手の表記について解説します。
		// 筋に関しては１から９までの数字で表記され、
		// 段に関してはaからiまでのアルファベット（１段目がa、２段目がb、・・・、９段目がi）というように表記されます。
		// 位置の表記は、この２つを組み合わせます。５一なら5a、１九なら1iとなります。
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
		{ { "K", "k" },{ L"玉", L"玉" }, E_EMPTY, E_OU }, // 他のソフトにあわせて両方玉にしてます
		{ { "R", "r" },{ L"飛", L"飛" }, E_RYU  , E_HI },
		{ { "B", "b" },{ L"角", L"角" }, E_UMA, E_KAKU },
		{ { "G", "g" },{ L"金", L"金" }, E_EMPTY, E_KIN },
		{ { "S", "s" },{ L"銀", L"銀" }, E_NGIN, E_GIN },
		{ { "N", "n" },{ L"桂", L"桂" }, E_NKEI, E_KEI },
		{ { "L", "l" },{ L"香", L"香" }, E_NKYO, E_KYO },
		{ { "P", "p" },{ L"歩", L"歩" }, E_TO, E_FU },

		{ { "+R", "+r" },{ L"龍", L"龍" }, E_EMPTY, E_HI },
		{ { "+B", "+b" },{ L"馬", L"馬" }, E_EMPTY, E_KAKU },
		{ { "+S", "+s" },{ L"成銀", L"成銀" }, E_EMPTY, E_GIN },
		{ { "+N", "+n" },{ L"成桂", L"成桂" }, E_EMPTY, E_KEI },
		{ { "+L", "+l" },{ L"成香", L"成香" },  E_EMPTY, E_KYO },
		{ { "+P", "+p" },{ L"と", L"と" }, E_EMPTY, E_FU },
	};

	const wstring mKomaMark[NUM_SEN_GO] = { L"▲", L"△" };
	const wstring mJapX[BOARD_SIZE] = { L"１", L"２", L"３", L"４", L"５", L"６", L"７", L"８", L"９" };
	const wstring mJapY[BOARD_SIZE] = { L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九" };
	const wstring mJapSemiNumber[BOARD_SIZE] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
	const wstring mJapDou  = { L"同" };
	const wstring mJapUtsu = { L"打" };
	const wstring mJapNaru = { L"成" };

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
