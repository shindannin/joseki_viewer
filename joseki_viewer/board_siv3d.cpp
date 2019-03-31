#include "board_siv3d.h"
#include "gui_siv3d.h"
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

int BoardSiv3D::GetGridLeftX() const
{
	return (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
}

int BoardSiv3D::GetGridTopY() const
{
	return (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
}

void BoardSiv3D::DrawCursor(const GridPos& gp, const Color& color) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	int y = gp.y;
	int x = gp.x;

	if (GetReverse())
	{
		y = BOARD_SIZE - 1 - y;
		x = BOARD_SIZE - 1 - x;
	}

	Rect(leftX + (BOARD_SIZE - 1 - x) * mKomaTextureWidth, topY + y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw(color);
}

void BoardSiv3D::Draw()
{
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
					DrawKoma(s, k, y, x, numKoma, false, true);
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
		int y = mOffsetY;
		if ((GetTeban()==E_GO) ^ GetReverse())
		{
			y -= rad * 2;
		}
		else
		{
			y += mTextureBoard.height;
		}


		RoundRect rect(x, y, w, rad * 2, rad);
		rect.draw(grabbedColor[GetTeban()]);
		mFontTeban(name[GetTeban()]).draw(x+5, y-1);
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
				DrawKoma(GetMasu(y,x), y, x);
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

		EKomaType type;
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
		DrawKoma(grabbed.sengo, grabbed.type, GetMoveToPos().y, GetMoveToPos().x, 0, true);
	}
}

void BoardSiv3D::DrawKoma(const Masu& masu, int y, int x) const
{
	DrawKoma(masu.sengo, masu.type, y, x);
}

void BoardSiv3D::DrawKoma(int baseSengo, int type, int y, int x, int maisu, bool isChoice, bool isMochigoma) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	int sengo = baseSengo;
	if (GetReverse())
	{
		sengo = 1 - sengo;
		if (!isMochigoma)
		{
			y = BOARD_SIZE - 1 - y;
			x = BOARD_SIZE - 1 - x;
		}
	}

	if (isChoice)
	{
		const EKomaType nariType = mKoma[type].narigoma;

		{
			const int drawX = static_cast<int>(leftX + (BOARD_SIZE-1-x)*mKomaTextureWidth);
			const int drawY = static_cast<int>(topY + y*mKomaTextureHeight);
			mTexture[sengo][type].draw(drawX, drawY);
		}

		NariOffsetY(y);

		{
			const int drawX = static_cast<int>(leftX + (BOARD_SIZE - 1.5f - x)*mKomaTextureWidth);
			const int drawY = static_cast<int>(topY + y*mKomaTextureHeight);
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw(narazuColor[GetTeban()]);
			mTexture[sengo][type].draw(drawX, drawY);
		}

		{
			const int drawX = static_cast<int>(leftX + (BOARD_SIZE - 0.5f - x)*mKomaTextureWidth);
			const int drawY = static_cast<int>(topY + y*mKomaTextureHeight);
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw(nariColor[GetTeban()]);
			mTexture[sengo][nariType].draw(drawX, drawY);
		}

	}
	else
	{
		mTexture[sengo][type].draw(leftX + (BOARD_SIZE - 1 - x)*mKomaTextureWidth, topY + y*mKomaTextureHeight);
	}

	// 駒枚数の数字
	if (maisu >= 1)
	{
		const float dx[] = { +1.0f, -0.25f };
		mFont(L"", maisu).draw(leftX + (BOARD_SIZE - 1 - x + dx[sengo])*mKomaTextureWidth, topY + y*mKomaTextureHeight, maisuFontColor[baseSengo]);
	}
}

void BoardSiv3D::SetOffset(int offsetX, int offsetY)
{
	mOffsetX = offsetX;
	mOffsetY = offsetY;
}


bool BoardSiv3D::GetGridPosFromMouse(GridPos& gridPos) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();
	int x = BOARD_SIZE - 1 - (Mouse::Pos().x - leftX) / mKomaTextureWidth;
	int y = (Mouse::Pos().y - topY) / mKomaTextureHeight;

	if (GetReverse())
	{
		x = BOARD_SIZE - 1 - x;
		y = BOARD_SIZE - 1 - y;
	}

	if (!(INRANGE(x, 0, BOARD_SIZE - 1) && INRANGE(y, 0, BOARD_SIZE - 1)))
	{
		return false;
	}

	gridPos.x = x;
	gridPos.y = y;
	return true;
}

void BoardSiv3D::GetXYNaruNarazuChoice(int& y, int& x) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	if (Mouse::Pos().x - leftX >= 0)
	{
		x = BOARD_SIZE - 1 - static_cast<int>(Mouse::Pos().x - leftX) / mKomaTextureWidth;
	}
	else
	{
		x = BOARD_SIZE + static_cast<int>(leftX - Mouse::Pos().x) / mKomaTextureWidth;
	}

	y = (Mouse::Pos().y - topY) / mKomaTextureHeight;
}

void BoardSiv3D::GetXYNaruNarazuChoice(float& y, float& x) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	if (Mouse::Pos().x - leftX >= 0)
	{
		x = BOARD_SIZE - static_cast<float>(Mouse::Pos().x - leftX) / mKomaTextureWidth;
	}
	else
	{
		x = BOARD_SIZE + static_cast<float>(leftX - Mouse::Pos().x) / mKomaTextureWidth;
	}

	y = static_cast<float>( (Mouse::Pos().y - topY) / mKomaTextureHeight );
}



void BoardSiv3D::UpdateDecided(string& te, wstring& teJap, bool& isMoved)
{
	te = GetTeFromMove(GetNextMove());
	teJap = DecideMove();
	isMoved = true;
	mInputState = E_IDLE;
	if (mSoundKomaoto)
	{
		mSoundKomaoto.play();
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

				if (IsNarubeki( GetMoveFromPos(), gp, GetTeban()))
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
			if (IsNaruChoice() || IsNarazuChoice())
			{
				if (IsNaruChoice())
				{
					SetNaru(true);
				}

				UpdateDecided(te, teJap, isMoved);
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

/* 盤面ペースト機能は、現在のバージョンは、初期盤面からスタートなので合わない気がする…
	else if ((Input::KeyControl + Input::KeyV).clicked)
	{
		// 盤面をペースト
		wstring ws = Clipboard::GetText().str();
		string s(ws.begin(), ws.end());
		vector <string> vs;
		Split1(s, vs, '\n');

		const string header("[SFEN");
		for (int i = 0; i < SZ(vs); ++i)
		{
			const string& line = vs[i];
			if (line.substr(0, SZ(header)) == header)
			{
				int st = NG;
				for (int k = SZ(header); k < SZ(line); ++k)
				{
					if (st == NG && line[k] == '"')
					{
						st = k + 1;
					}
					else if (line[k] == '"')
					{
						const int ed = k;
						const int len = ed - st;

						SetState(line.substr(st, len));
						break;
					}
				}
				assert(st != NG);
			}
		}
	}
*/


	return isMoved;
}

template <class T>
void BoardSiv3D::NariOffsetY(T& y) const
{
	if (y <= BOARD_SIZE / 2)
	{
		y++;
	}
	else
	{
		y--;
	}
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
	int y, x;
	GetXYNaruNarazuChoice(y, x);
	for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
	{
		if (GetNumMochigoma(GetTeban(), k) >= 1)
		{
			int my, mx;
			GetMochigomaPos(GetTeban(), k, my, mx);
			if (my == y && mx == x)
			{
				utsuKomaType = static_cast<EKomaType>(k);
				return true;
			}
		}
	}

	return false;
}


bool BoardSiv3D::IsNaruChoice() const
{
	float y, x;
	GetXYNaruNarazuChoice(y, x);
	if (GetReverse())
	{
		y = BOARD_SIZE - 1 - y;
		x = BOARD_SIZE - 1 - x;
	}

	float		cy = static_cast<float>(GetMoveToPos().y);
	const float	cx = static_cast<float>(GetMoveToPos().x);
	NariOffsetY(cy);

	return (INRANGE(x, cx - 0.5f, cx + 0.5f) && INRANGE(y, cy, cy + 1.0f));
}

bool BoardSiv3D::IsNarazuChoice() const
{
	float y, x;
	GetXYNaruNarazuChoice(y, x);
	if (GetReverse())
	{
		y = BOARD_SIZE - 1 - y;
		x = BOARD_SIZE - 1 - x + 2;
	}

	float		cy = static_cast<float>(GetMoveToPos().y);
	const float	cx = static_cast<float>(GetMoveToPos().x);
	NariOffsetY(cy);

	return (INRANGE(x, cx + 0.5f, cx + 1.5f) && INRANGE(y, cy, cy + 1.0f));
}

// 移動をあらわす表示。また、その中央座標をcy,cxで返す（矢印の上の評価値表示など、tree側で使用する目的）
void BoardSiv3D::DrawMove(const string& te, const Color& color, int& cy, int& cx) const
{
	Move mv = GetMoveFromTe(te);
	int startY, startX;
	if (mv.utsuKomaType == E_EMPTY)
	{
		startY = mv.from.y;
		startX = mv.from.x;

		if (GetReverse())
		{
			startY = BOARD_SIZE - 1 - startY;
			startX = BOARD_SIZE - 1 - startX;
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
		mv.to.y = BOARD_SIZE - 1 - mv.to.y;
		mv.to.x = BOARD_SIZE - 1 - mv.to.x;
	}

	DrawArrow(startY, startX, mv.to.y, mv.to.x, color, cy, cx);
}

// 矢印を表示
void BoardSiv3D::DrawArrow(int startY, int startX, int destY, int destX, const Color& color, int& cy, int& cx) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();


	const float sx = leftX + (BOARD_SIZE - 1 - startX + 0.5f) * mKomaTextureWidth;
	const float sy = topY  + (startY + 0.5f) * mKomaTextureHeight;
	const float dx = leftX + (BOARD_SIZE - 1 - destX + 0.5f) * mKomaTextureWidth;
	const float dy = topY +  (destY + 0.5f) * mKomaTextureHeight;

	cy = static_cast<int>((sy + dy) * 0.5f);
	cx = static_cast<int>((sx + dx) * 0.5f);

	Line(sx, sy, dx, dy).drawArrow(10, { 20, 20 }, color);
}

bool BoardSiv3D::GetReverse() const
{
	return mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::REVERSE_BOARD);
}



