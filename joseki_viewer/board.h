// board.h : �����Ղɂ��Ẵt�@�C���B�����ɏ����Ă���N���X�́ASiv3D�ɂ͈ˑ������A�P�̃e�X�g���\(unittest1.cpp)�B

#pragma once

#include "util.h"
#include <cassert>



// ��E��
enum ESengo
{
	E_SEN,			// ���
	E_GO,			// ���
	NUM_SEN_GO,

	E_NO_SENGO = -1,	// �ǂ���ł��Ȃ��i�}�X�̏�ԂŁA�g����j
};

constexpr int NUM_JAP_DESCRIPTIONS = 2;

// ��̎�ށB����͕ʂ̎�ނƂ��Ĉ�����̂Œ���
enum EKomaType
{
	E_OU,	// ��
	E_HI,	// ��
	E_KAKU,	// �p
	E_KIN,	// ��
	E_GIN,	// ��
	E_KEI,	// �j
	E_KYO,	// ��
	E_FU,	// ��
	NUM_NARAZU_KOMA_TYPE,

	E_RYU = NUM_NARAZU_KOMA_TYPE,	// ��
	E_UMA,	// �n
	E_NGIN,	// ����
	E_NKEI,	// ���j
	E_NKYO,	// ����
	E_TO,	// ��
	NUM_KOMA_TYPE,

	E_EMPTY = -1,
};


// ������1�}�X���Ƃ̏��
struct Masu
{
	Masu() : type(E_EMPTY), sengo(E_NO_SENGO) {}
	EKomaType type;	// ��̎��
	ESengo sengo;	// �����ꍇ�͐����ǂ���̋�A��Ȃ��ꍇ�́AE_NO_SENGO�ɂȂ�B
};

// �����Ղ̃T�C�Y�B���ʂ̏����ł͓��R 9
const int BOARD_SIZE = 9;

// �����Ղ̍��W�B0-index�Ȃ̂ŁA(y,x)��(0,0)-(8,8)�͈̔͂̒l���Ƃ�
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

// ��
struct Koma
{
	string notation[NUM_SEN_GO];		// SFEN�\�L�ɂ���̉p��\�L
	wstring jap[NUM_JAP_DESCRIPTIONS];	// ��̓��{��\�L
	EKomaType narigoma;					// ��������́A��̎��
	EKomaType motogoma;					// ����O�́A��̎��
	vector <GridPos> movableDirections;	// �������Ɉړ��\�ȕ���
	vector <GridPos> flyingDirections;	// �r���ɋ�Ȃ�����A�ړ��\�ȕ���
};

// 1�蕪
struct Move
{
	GridPos from;	// �ړ����̍��W
	GridPos to;		// �ړ���̍��W
	bool naru;		// ��Ȃ�ꍇ��true
	EKomaType utsuKomaType;	// ���łꍇ�̋�̎��

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

// �����Ձ{��B��Ԃ��Ԃ��܂�ł���̂ŁA���̃N���X�����ŏ������w�����Ƃ͉\�B
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
		{ { "K", "k" },{ L"��", L"��" }, E_EMPTY, E_OU,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, {} }, // ���̃\�t�g�ɂ��킹�ė����ʂɂ��Ă܂�
		{ { "R", "r" },{ L"��", L"��" }, E_RYU  , E_HI,  {}, { {-1,  0}, { 0, -1}, { 0, +1},{+1,  0} } },
		{ { "B", "b" },{ L"�p", L"�p" }, E_UMA, E_KAKU,  {}, { {-1, -1}, {-1, +1}, {+1, -1},{+1, +1} } },
		{ { "G", "g" },{ L"��", L"��" }, E_EMPTY, E_KIN, { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "S", "s" },{ L"��", L"��" }, E_NGIN, E_GIN,  { {-1, -1}, {-1,  0}, {-1, +1}, {+1, -1}, {+1, +1} }, {} },
		{ { "N", "n" },{ L"�j", L"�j" }, E_NKEI, E_KEI,  { {-2, -1}, {-2, +1} }, {} },
		{ { "L", "l" },{ L"��", L"��" }, E_NKYO, E_KYO,  {}, { {-1,  0} } },
		{ { "P", "p" },{ L"��", L"��" }, E_TO, E_FU,     { {-1,  0} }, {} },

		{ { "+R", "+r" },{ L"��", L"��" }, E_EMPTY, E_HI,       { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, { {-1,  0}, { 0, -1}, { 0, +1},{+1,  0} } },
		{ { "+B", "+b" },{ L"�n", L"�n" }, E_EMPTY, E_KAKU,     { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1, -1}, {+1,  0}, {+1, +1} }, { {-1, -1}, {-1, +1}, {+1, -1},{+1, +1} } },
		{ { "+S", "+s" },{ L"����", L"�S" }, E_EMPTY, E_GIN,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+N", "+n" },{ L"���j", L"�\" }, E_EMPTY, E_KEI,  { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+L", "+l" },{ L"����", L"��" },  E_EMPTY, E_KYO, { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
		{ { "+P", "+p" },{ L"��", L"��" }, E_EMPTY, E_FU,       { {-1, -1}, {-1,  0}, {-1, +1}, { 0, -1}, { 0, +1}, {+1,  0} }, {} },
	};

	const wstring mKomaMark[NUM_SEN_GO] = { L"��", L"��" };
	const wstring mJapX[BOARD_SIZE] = { L"�P", L"�Q", L"�R", L"�S", L"�T", L"�U", L"�V", L"�W", L"�X" };
	const wstring mJapY[BOARD_SIZE] = { L"��", L"��", L"�O", L"�l", L"��", L"�Z", L"��", L"��", L"��" };
	const wstring mJapSemiNumber[BOARD_SIZE] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
	const wstring mJapDou  = { L"��" };
	const wstring mJapUtsu = { L"��" };
	const wstring mJapNaru = { L"��" };

private:
	void InitValidMoveGrid();
	bool IsTekijin(int y, ESengo teban) const;
	bool IsBanjyo(const GridPos& gp) const;
	bool IsBanjyo(int y, int x) const;
	bool IsIkidomari(int y, EKomaType type, ESengo teban) const;

	vector < vector <Masu> > mGrid;				// mGrid[y][x]	�����Ս��W(y,x)�̃}�X���
	vector < vector <int> > mMochigoma;			// mMochigoma[ESengo][EKomaType] ������̐�
	ESengo mTeban;								// ���݂̎��
	vector < vector <bool> > mValidMoveGrid;	// mValidMoveGrid[y][x] �����Ս��W(y,x)�֋���ړ��ł�����ł�����ł��邩�ǂ����B

	Move mNextMove;								// ���̎�
	vector <Move> mMoves;						// ���肩��A���݂܂ł̎w����
};
