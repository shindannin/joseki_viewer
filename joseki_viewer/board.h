// board.h : 将棋盤についてのファイル。ここに書いてあるクラスは、Siv3Dには依存せず、単体テストも可能(unittest1.cpp)。

#pragma once

#include "util.h"
#include <cassert>



// 先・後
enum ESengo
{
	E_SEN,			// 先手
	E_GO,			// 後手
	NUM_SEN_GO,

	E_NO_SENGO = -1,	// どちらでもない（マスの状態で、使われる）
};

constexpr int NUM_JAP_DESCRIPTIONS = 2;

// 駒の種類。成駒は別の種類として扱われるので注意
enum EKomaType
{
	E_OU,	// 玉
	E_HI,	// 飛
	E_KAKU,	// 角
	E_KIN,	// 金
	E_GIN,	// 銀
	E_KEI,	// 桂
	E_KYO,	// 香
	E_FU,	// 歩
	NUM_NARAZU_KOMA_TYPE,

	E_RYU = NUM_NARAZU_KOMA_TYPE,	// 龍
	E_UMA,	// 馬
	E_NGIN,	// 成銀
	E_NKEI,	// 成桂
	E_NKYO,	// 成香
	E_TO,	// と
	NUM_KOMA_TYPE,

	E_EMPTY = -1,
};


// 将棋盤1マスごとの状態
struct Masu
{
	Masu() : type(E_EMPTY), sengo(E_NO_SENGO) {}
	EKomaType type;	// 駒の種類
	ESengo sengo;	// 駒がある場合は先手後手どちらの駒か、駒がない場合は、E_NO_SENGOになる。
};

// 将棋盤のサイズ。普通の将棋では当然 9
const int BOARD_SIZE = 9;

// 将棋盤の座標。0-indexなので、(y,x)は(0,0)-(8,8)の範囲の値をとる
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

	const GridPos operator+(const GridPos& rhs) const
	{
		GridPos tmp;
		tmp.y = y + rhs.y;
		tmp.x = x + rhs.x;
		return tmp;
	}

	inline GridPos& operator+=(const GridPos& rhs)
	{
		y += rhs.y;
		x += rhs.x;
		return *this;
	}

	int x;
	int y;
};

// 駒
struct Koma
{
	string notation[NUM_SEN_GO];		// SFEN表記による駒の英語表記
	wstring jap[NUM_JAP_DESCRIPTIONS];	// 駒の日本語表記
	EKomaType narigoma;					// 成った後の、駒の種類
	EKomaType motogoma;					// 成る前の、駒の種類
	vector <GridPos> movableDirections;	// 無条件に移動可能な方向
	vector <GridPos> flyingDirections;	// 途中に駒がない限り、移動可能な方向
};

// 1手分
struct Move
{
	GridPos from;	// 移動元の座標
	GridPos to;		// 移動先の座標
	bool naru;		// 駒がなる場合はtrue
	EKomaType utsuKomaType;	// 駒を打つ場合の駒の種類

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

// 将棋盤＋駒。手番や状態も含んでいるので、このクラスだけで将棋を指すことは可能。
class Board
{
public:
	Board();
	virtual ~Board() {};
	virtual void Draw() {};
	virtual bool Update(string& /* te */, wstring& /* teJap */) { return false; };
	virtual int CalcBestMoveAndScore() { return 0; }

	virtual void Init();
	void InitState();

	wstring MoveByTejun(const string& tejun);
	wstring MoveByTe(const string& te);


	wstring DecideMove();

	void SetState(const string& state);

	string GetState() const;

	string GetTeFromPSN(const string& tePSN) const;
	string GetTejunFromPSN(const string& tejunPSN) const;
	string GetTejunFromKif(const vector <wstring>& kifStrings) const;
	string GetTeFromMove(const Move& mv) const;
	string GetFirstTeFromTejun(const string& tejun) const;
	Move GetMoveFromTe(const string& te) const;


	void InitNextMove() { mNextMove.Init(); }

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

protected:
	bool IsNarubeki(const GridPos& from, const GridPos& to, ESengo teban) const;
	bool IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const;

	void GenerateValidMoveGridGrabbed();
	void GenerateValidMoveGridUtsu();
	bool IsValidMove(const GridPos& gp) const;


	const vector <Koma> mKoma =
	{
		{ { "K", "k" },{ L"玉", L"玉" }, E_EMPTY, E_OU,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, {} }, // 他のソフトにあわせて両方玉にしてます
		{ { "R", "r" },{ L"飛", L"飛" }, E_RYU  , E_HI,  {}, { {-1,  0}, { 0, -1}, { 0, +1},{+1,  0} } },
		{ { "B", "b" },{ L"角", L"角" }, E_UMA, E_KAKU,  {}, { {-1, -1}, {-1, +1}, {+1, -1},{+1, +1} } },
		{ { "G", "g" },{ L"金", L"金" }, E_EMPTY, E_KIN, { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "S", "s" },{ L"銀", L"銀" }, E_NGIN, E_GIN,  { {-1, -1}, {-1,  0}, {-1, +1}, {+1, -1}, {+1, +1} }, {} },
		{ { "N", "n" },{ L"桂", L"桂" }, E_NKEI, E_KEI,  { {-2, -1}, {-2, +1} }, {} },
		{ { "L", "l" },{ L"香", L"香" }, E_NKYO, E_KYO,  {}, { {-1,  0} } },
		{ { "P", "p" },{ L"歩", L"歩" }, E_TO, E_FU,     { {-1,  0} }, {} },

		{ { "+R", "+r" },{ L"龍", L"竜" }, E_EMPTY, E_HI,       { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, { {-1,  0}, { 0, -1}, { 0, +1},{+1,  0} } },
		{ { "+B", "+b" },{ L"馬", L"馬" }, E_EMPTY, E_KAKU,     { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, { {-1, -1}, {-1, +1}, {+1, -1},{+1, +1} } },
		{ { "+S", "+s" },{ L"成銀", L"全" }, E_EMPTY, E_GIN,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+N", "+n" },{ L"成桂", L"圭" }, E_EMPTY, E_KEI,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+L", "+l" },{ L"成香", L"杏" },  E_EMPTY, E_KYO, { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+P", "+p" },{ L"と", L"と" }, E_EMPTY, E_FU,       { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
	};

	const wstring mKomaMark[NUM_SEN_GO] = { L"▲", L"△" };
	const wstring mJapX[BOARD_SIZE] = { L"１", L"２", L"３", L"４", L"５", L"６", L"７", L"８", L"９" };
	const wstring mJapY[BOARD_SIZE] = { L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九" };
	const wstring mJapSemiNumber[BOARD_SIZE] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
	const wstring mJapDou  = { L"同" };
	const wstring mJapUtsu = { L"打" };
	const wstring mJapNaru = { L"成" };

private:
	void InitValidMoveGrid();
	bool IsTekijin(int y, ESengo teban) const;
	bool IsBanjyo(const GridPos& gp) const;
	bool IsBanjyo(int y, int x) const;
	bool IsIkidomari(int y, EKomaType type, ESengo teban) const;

	vector < vector <Masu> > mGrid;				// mGrid[y][x]	将棋盤座標(y,x)のマス情報
	vector < vector <int> > mMochigoma;			// mMochigoma[ESengo][EKomaType] 持ち駒の数
	ESengo mTeban;								// 現在の手番
	vector < vector <bool> > mValidMoveGrid;	// mValidMoveGrid[y][x] 将棋盤座標(y,x)へ駒を移動できたり打ったりできるかどうか。

	Move mNextMove;								// 次の手
	vector <Move> mMoves;						// 初手から、現在までの指し手
};
