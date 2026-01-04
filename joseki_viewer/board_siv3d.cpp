#include "board_siv3d.h"
#include "gui_siv3d.h"
#include "color.h"
#include "util.h"

#include <cassert>

BoardSiv3D::BoardSiv3D(GuiSiv3D& gui) : mGui(gui)
{
	for (int s = 0; s < NUM_SEN_GO; ++s)
	{
		mTexture[s].clear();
		for (int k = 0; k < NUM_KOMA_TYPE; ++k)
		{
			String fileName(L"pictures/");
			fileName += mFileName[k][s];
			fileName += L".png";
			mTexture[s].push_back(Texture(fileName));
		}
	}
	mKomaTextureWidth = mTexture[0][0].width;
	mKomaTextureHeight = mTexture[0][0].height;

	mTextureBoard = Texture(L"pictures/ban_kaya_a.png");
	mTextureGrid = Texture(L"pictures/masu_dot.png");

	mBoardTextureWidth = mTextureBoard.width;
	mBoardTextureHeight = mTextureBoard.height;

	mFont = Font(15, L"メイリオ");
	mFontTeban = Font(10, L"メイリオ");

	mOffsetX = 0;
	mOffsetY = 0;

	mInputState = E_IDLE;

	mSoundKomaoto = Sound(L"sounds/komaoto5.wav");


	SetOffset(0, 0);
}

BoardSiv3D::~BoardSiv3D()
{
}

void BoardSiv3D::Init()
{
	Board::Init();
	mInputState = E_IDLE;
}

// 将棋番の黒線グリッド左上のX座標
int BoardSiv3D::GetGridLeftX() const
{
	return (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
}

// 将棋番の黒線グリッド左上のY座標
int BoardSiv3D::GetGridTopY() const
{
	return (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
}

int BoardSiv3D::BoardReverse(int a) const
{
	return BOARD_SIZE-1-a;
}

void BoardSiv3D::DrawCursor(const GridPos& gp, const Color& color) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	int y = gp.y;
	int x = gp.x;

	if (GetReverse())
	{
		y = BoardReverse(y);
		x = BoardReverse(x);
	}

	Rect(leftX + (BoardReverse(x)) * mKomaTextureWidth, topY + y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw(color);
}

void BoardSiv3D::Draw()
{
	RoundRect rrect(mOffsetX - 100, mOffsetY - 50, 612, 525, 10);
	rrect.draw(COLOR_NEUTRAL2);

	mTextureBoard.draw(mOffsetX, mOffsetY);
	mTextureGrid.draw(mOffsetX, mOffsetY);

	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();
	const int tebanOffestX = 10;




	//----- カーソル -----
	// マウスが指しているマス
	{
		GridPos gp;
		if (GetGridPosFromMouse(gp))
		{
			DrawCursor(gp, cursorColor[GetTeban()]);
		}
	}

	// つかんだ駒のマス
	if (mInputState == E_GRABBED || mInputState == E_CHOICE)
	{
		DrawCursor(GetMoveFromPos(), grabbedColor[GetTeban()]);
	}

	//----- 駒台 -----
	{
		const int w = static_cast<int>(mKomaTextureWidth *1.75f);
		const int h = static_cast<int>(mKomaTextureHeight*7.50f);
		const int spaceW = static_cast<int>(mKomaTextureWidth * 0.5f);

		mTextureBoard(0, 0, w, h).draw(mOffsetX - spaceW - w, mOffsetY);
		mTextureBoard(0, 0, w, h).draw(mOffsetX  + mTextureBoard.width + spaceW, mOffsetY + mTextureBoard.height - h);
	
	}

	//----- 持ち駒 -----
	{
		for (int s = 0; s < NUM_SEN_GO; ++s)
		{
			for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
			{
				int numKoma = GetNumMochigoma(s, k);
				if (mInputState == E_UTSU && s == GetTeban() && GetUtsuKomaType() == k)
				{
					numKoma--;
				}

				if (numKoma >= 1)
				{
					int y, x;
					GetMochigomaPos(s, k, y, x); // すでに先手後手を考慮していて、場所もあってる。
					DrawKomaDetail(s, k, y, x, numKoma, false, true);
				}
			}
		}
	}

	//----- 手番 -----
	{
		wstring name[2] = { L"先手番", L"後手番" };
		const int w = 50;
		const int rad = 8;
		const int x = mOffsetX - w / 2 + mTextureBoard.width / 2;

		const int y_top = mOffsetY - (rad * 2);
		const int y_bot = mOffsetY + mTextureBoard.height;

		mFontTeban(mKifHeader.mSente).draw(x + w + 5, GetReverse() ? y_top - 1 : y_bot - 1);
		mFontTeban(mKifHeader.mGote).draw(x + w + 5, GetReverse() ? y_bot - 1 : y_top - 1);

		int teban_marker_y;
		if ((GetTeban()==E_GO) ^ GetReverse())
		{
			teban_marker_y = y_top;
		}
		else
		{
			teban_marker_y = y_bot;
		}

		RoundRect rect(x, teban_marker_y, w, rad * 2, rad);
		rect.draw(grabbedColor[GetTeban()]);
		mFontTeban(name[GetTeban()]).draw(x+5, teban_marker_y -1);
	}

	//----- 盤面 -----
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			if (mInputState == E_GRABBED || mInputState == E_CHOICE)
			{
				// つかんだ駒は、つかんだ位置には表示しない。
				if (GridPos(y,x) == GetMoveFromPos())
				{
					continue;
				}
			}

			if (mInputState == E_GRABBED || mInputState == E_UTSU)
			{
				GridPos gp(y, x);
				if (IsValidMove(GridPos(y, x)))
				{
					DrawCursor(gp, cursorColor[GetTeban()]);
				}
			}

			if (GetMasu(y,x).type != E_EMPTY)
			{
				DrawKomaSimple(GetMasu(y,x), y, x);
			}
		}
	}

	// つかんだ駒
	if (mInputState == E_GRABBED || mInputState == E_UTSU)
	{
		const Masu& masu = GetMasu(GetMoveFromPos());
		int sengo = GetTeban();
		if (GetReverse())
		{
			sengo = 1 - sengo;
		}

		EKomaType type = E_EMPTY;
		if (mInputState == E_GRABBED)
		{
			type = masu.type;
		}
		else if (mInputState == E_UTSU)
		{
			type = GetUtsuKomaType();
		}

		mTexture[sengo][type].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
	}

	// 成り選択画面
	if (mInputState == E_CHOICE)
	{
		const Masu& grabbed = GetMasu(GetMoveFromPos());
		DrawKomaDetail(grabbed.sengo, grabbed.type, GetMoveToPos().y, GetMoveToPos().x, 0, true);
	}
}

void BoardSiv3D::DrawKomaSimple(const Masu& masu, int y, int x) const
{
	DrawKomaDetail(masu.sengo, masu.type, y, x);
}

void BoardSiv3D::DrawKomaDetail(int baseSengo, int type, int y, int x, int maisu, bool isChoice, bool ignoreReverse) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	int sengo = baseSengo;
	if (GetReverse())
	{
		sengo = 1 - sengo;
		if (!ignoreReverse)
		{
			y = BoardReverse(y);
			x = BoardReverse(x);
		}
	}

	const int drawBaseX = leftX + BoardReverse(x)*mKomaTextureWidth;
	const int drawBaseY = topY + y * mKomaTextureHeight;

	// つかんでいる駒の表示
	mTexture[sengo][type].draw(drawBaseX, drawBaseY);

	if (isChoice)
	{
		const EKomaType nariType = mKoma[type].narigoma;

		// 成らずの表示
		{
			const int drawX = static_cast<int>(drawBaseX - 0.5f * mKomaTextureWidth);
			const int drawY = static_cast<int>(drawBaseY + NariOffsetY(y)*mKomaTextureHeight);
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw(narazuColor[GetTeban()]);
			mTexture[sengo][type].draw(drawX, drawY);
		}

		// 成りの表示
		{
			const int drawX = static_cast<int>(drawBaseX + 0.5f * mKomaTextureWidth);
			const int drawY = static_cast<int>(drawBaseY + NariOffsetY(y)*mKomaTextureHeight);
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw(nariColor[GetTeban()]);
			mTexture[sengo][nariType].draw(drawX, drawY);
		}
	}


	// 駒枚数の数字
	if (maisu >= 1)
	{
		const float dx[] = { +1.0f, -0.25f };
		mFont(L"", maisu).draw(leftX + (BoardReverse(x) + dx[sengo])*mKomaTextureWidth, topY + y*mKomaTextureHeight, maisuFontColor[baseSengo]);
	}
}

// 成先の座標(y,x)
bool BoardSiv3D::IsNariChoice(const GridPos& gridPos) const
{
	return IsNariNarazuChoice(gridPos.y, gridPos.x, true);
}

bool BoardSiv3D::IsNarazuChoice(const GridPos& gridPos) const
{
	return IsNariNarazuChoice(gridPos.y, gridPos.x, false);
}

bool BoardSiv3D::IsNariNarazuChoice(int y, int x, bool naru) const
{
	if (GetReverse())
	{
		y = BoardReverse(y);
		x = BoardReverse(x);
	}

	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();
	const int drawBaseX = leftX + BoardReverse(x)*mKomaTextureWidth;
	const int drawBaseY = topY + y * mKomaTextureHeight;
	const int drawX = static_cast<int>(drawBaseX - 0.5f * mKomaTextureWidth + (naru ? mKomaTextureWidth : 0) );
	const int drawY = static_cast<int>(drawBaseY + NariOffsetY(y)*mKomaTextureHeight);

	const int mx = Mouse::Pos().x;
	const int my = Mouse::Pos().y;

	return (drawX <= mx && mx < drawX + mKomaTextureWidth && drawY <= my && my < drawY + mKomaTextureHeight);
}


void BoardSiv3D::SetOffset(int offsetX, int offsetY)
{
	mOffsetX = offsetX;
	mOffsetY = offsetY;
}


// 将棋座標。盤面も反転にして(0,0)が１一、(8,8)が９九にあたる。盤面内ならtrue,盤面外ならfalseを返す。
bool BoardSiv3D::GetGridPosFromMouse(GridPos& gridPos, bool checkReverseFlag) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	gridPos.x = BoardReverse(static_cast<int>(floor(static_cast<float>(Mouse::Pos().x - leftX) / mKomaTextureWidth))); // これはもともとのX座標反転
	gridPos.y = static_cast<int>(floor(static_cast<float>(Mouse::Pos().y - topY) / mKomaTextureHeight));

	if (checkReverseFlag && GetReverse())
	{
		gridPos.x = BoardReverse(gridPos.x);
		gridPos.y = BoardReverse(gridPos.y);
	}

	return IsBanjyo(gridPos);
}

void BoardSiv3D::UpdateDecided(string& te, wstring& teJap, bool& isMoved)
{
	te = GetTeFromMove(GetNextMove());
	teJap = DecideMove();
	isMoved = true;
	mInputState = E_IDLE;
	if (mSoundKomaoto)
	{
		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SOUND))
		{
			mSoundKomaoto.play();
		}
	}
}

bool BoardSiv3D::Update(string& te, wstring& teJap)
{
	bool isMoved = false;

	if (Input::MouseL.clicked)
	{
		switch (mInputState)
		{
		case E_IDLE:
		{
			// 駒をつかむ
			GridPos gp; // 移動元のGridPos
			if (GetGridPosFromMouse(gp))
			{
				// 自分の駒だけ！
				if (GetMasu(gp).sengo == GetTeban() && GetMasu(gp).type != E_EMPTY)
				{
					SetMoveFromPos(gp);
					GenerateValidMoveGridGrabbed();
					mInputState = E_GRABBED;
				}
			}
			else
			{
				EKomaType k;
				if (CalcUtsuKomaType(k))
				{
					SetUtsuKomaType(k);
					GenerateValidMoveGridUtsu();
					mInputState = E_UTSU;
				}
			}
		}
		break;

		case E_GRABBED:
		{
			GridPos gp; // 移動先のGridPos
			if (GetGridPosFromMouse(gp) &&
				IsValidMove(gp)
				)
			{
				// 成りのチェック
				SetMoveToPos(gp);

				if (IsNarubeki(GetMoveFromPos(), gp, GetTeban()))
				{
					// 行き止まりなので、強制的に成る（歩・香車・桂馬）
					SetNaru(true);
					UpdateDecided(te, teJap, isMoved);
				}
				else if (IsNareru(GetMoveFromPos(), gp, GetTeban()))
				{
					// 成り選択画面へ
					mInputState = E_CHOICE;
				}
				else
				{
					UpdateDecided(te, teJap, isMoved);
				}
			}
		}
		break;

		case E_UTSU:
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp) && IsValidMove(gp))
			{
				SetMoveToPos(gp);
				UpdateDecided(te, teJap, isMoved);
			}
		}
		break;

		case E_CHOICE:
		// 成り成らずを選択する
		{
			GridPos gp = GetMoveToPos();
			if (IsNariChoice(gp))
			{
				SetNaru(true);
				UpdateDecided(te, teJap, isMoved);
			}
			else if (IsNarazuChoice(gp))
			{
				UpdateDecided(te, teJap, isMoved);
			}
		}
		break;

		default:
			break;
		}
	}
	else if (Input::MouseR.clicked)
	{
		switch (mInputState)
		{
		case E_IDLE:
			break;

		case E_UTSU:
		case E_GRABBED:
		case E_CHOICE:
			InitNextMove();
			mInputState = E_IDLE;
			break;

		default:
			break;
		}
	}

	return isMoved;
}

int BoardSiv3D::NariOffsetY(int y) const
{

	if (y <= BOARD_SIZE / 2)
	{
		return +1; // 0, 1, 2, 3, 4
	}

	return -1; // 5, 6, 7, 8
}

void BoardSiv3D::GetMochigomaPos(int sengo, int k, int& y, int& x) const
{
	const int posX[2] = { -2, 10 };
	const int posY[2] = { 1,  7 };
	const int signY[2] = { +1, -1 };
	assert(INRANGE(sengo,0,1));

	if (GetReverse())
	{
		sengo = 1 - sengo;
	}

	y = k*signY[sengo] + posY[sengo];
	x = posX[sengo];
}


bool BoardSiv3D::CalcUtsuKomaType(EKomaType& utsuKomaType) const
{
	GridPos gridPos;
	GetGridPosFromMouse(gridPos, false);

	for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
	{
		if (GetNumMochigoma(GetTeban(), k) >= 1)
		{
			int my, mx;
			GetMochigomaPos(GetTeban(), k, my, mx);
			if (my == gridPos.y && mx == gridPos.x)
			{
				utsuKomaType = static_cast<EKomaType>(k);
				return true;
			}
		}
	}

	return false;
}

// 移動をあらわす表示。また、その中央座標をcy,cxで返す（矢印の上の評価値表示など、tree側で使用する目的）
void BoardSiv3D::DrawMove(const string& te, const Color& color, int& cy, int& cx) const
{
	DrawMove(te, color, cy, cx, 10.0, nullptr, nullptr);
}

void BoardSiv3D::DrawMove(const string& te, const Color& color, int& cy, int& cx, double width, Vec2* startPos, Vec2* endPos) const
{
	if (IsSpecialMoveTe(te))
	{
		cy = -1;
		cx = -1;
		if (startPos != nullptr)
		{
			*startPos = Vec2(-1, -1);
		}
		if (endPos != nullptr)
		{
			*endPos = Vec2(-1, -1);
		}
		return;
	}

	Move mv = GetMoveFromTe(te);
	int startY, startX;
	if (mv.utsuKomaType == E_EMPTY)
	{
		startY = mv.from.y;
		startX = mv.from.x;

		if (GetReverse())
		{
			startY = BoardReverse(startY);
			startX = BoardReverse(startX);
		}
	}
	else
	{
		// 駒を打つ場合の座標に注意
		const int s = static_cast<int>(GetTeban());
		GetMochigomaPos(s, mv.utsuKomaType, startY, startX);
	}

	if (GetReverse())
	{
		mv.to.y = BoardReverse(mv.to.y);
		mv.to.x = BoardReverse(mv.to.x);
	}

	DrawArrow(startY, startX, mv.to.y, mv.to.x, color, cy, cx, width, startPos, endPos);
}

// 矢印を表示
void BoardSiv3D::DrawArrow(int startY, int startX, int destY, int destX, const Color& color, int& cy, int& cx, double width, Vec2* startPos, Vec2* endPos) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();


	const float sx = leftX + (BoardReverse(startX) + 0.5f) * mKomaTextureWidth;
	const float sy = topY  + (startY + 0.5f) * mKomaTextureHeight;
	const float dx = leftX + (BoardReverse(destX) + 0.5f) * mKomaTextureWidth;
	const float dy = topY +  (destY + 0.5f) * mKomaTextureHeight;

	cy = static_cast<int>((sy + dy) * 0.5f);
	cx = static_cast<int>((sx + dx) * 0.5f);

	if (startPos != nullptr)
	{
		*startPos = Vec2(sx, sy);
	}
	if (endPos != nullptr)
	{
		*endPos = Vec2(dx, dy);
	}

	Line(sx, sy, dx, dy).drawArrow(width, { 20, 20 }, color);
}

bool BoardSiv3D::GetReverse() const
{
	return mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::REVERSE_BOARD);
}
