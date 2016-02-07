#include "board_siv3d.h"

#include <assert.h>




BoardSiv3D::BoardSiv3D()
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

	mOffsetX = 0;
	mOffsetY = 0;

	SetOffset(100, 0);
}

void BoardSiv3D::Draw()
{
	mTextureBoard.draw(mOffsetX, mOffsetY);
	mTextureGrid.draw(mOffsetX, mOffsetY);

	const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
	const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
	const int tebanOffestX = 10;

	//----- カーソル -----
	// 現在
	{
		GridPos gp;
		if (GetGridPosFromMouse(gp))
		{
			Rect(leftX + (BOARD_SIZE - 1 - gp.x) * mKomaTextureWidth, topY + gp.y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw({ 128, 0, 0, 127 });
		}
	}

	if (mInputState == E_GRABBED || mInputState == E_CHOICE)
	{
		// つかんだ位置
		Rect(leftX + (BOARD_SIZE - 1 - mGrabbedPos.x) * mKomaTextureWidth, topY + mGrabbedPos.y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw({ 255, 0, 0, 127 });
	}

	//----- 持ち駒 -----
	{
		for (int s = 0; s < NUM_SEN_GO; ++s)
		{
			for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
			{
				int numKoma = mMochigoma[s][k];
				if (mInputState == E_HARU && s == mTeban && mHaruKomaType == k)
				{
					numKoma--;
				}

				if (numKoma >= 1)
				{
					int y, x;
					GetMochigomaPos(s, k, y, x);
					DrawKoma(s, k, y, x, numKoma);
				}
			}
		}
	}

	//----- 手番 -----
	{
		wstring name[2] = { L"先手番", L"後手番" };
		mFont(name[mTeban]).draw(leftX + tebanOffestX * mKomaTextureWidth, topY);
	}

	//----- 盤面 -----
	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			if (mInputState == E_GRABBED || mInputState == E_CHOICE)
			{
				if (y == mGrabbedPos.y && x == mGrabbedPos.x)
				{
					continue;
				}
			}

			if (mGrid[y][x].type != E_EMPTY)
			{
				DrawKoma(mGrid[y][x], y, x);
			}
		}
	}

	// つかんだ駒
	if (mInputState == E_GRABBED || mInputState == E_CHOICE)
	{
		const Masu& masu = mGrid[mGrabbedPos.y][mGrabbedPos.x];
		mTexture[masu.sengo][masu.type].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
	}
	else if (mInputState == E_HARU)
	{
		mTexture[mTeban][mHaruKomaType].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
	}

	// 成り選択画面
	if (mInputState == E_CHOICE)
	{
		const Masu& grabbed = mGrid[mGrabbedPos.y][mGrabbedPos.x];
		DrawKoma(grabbed.sengo, grabbed.type, mMoveToPos.y, mMoveToPos.x, 0, true);
	}

}

void BoardSiv3D::DrawKoma(const Masu& masu, int y, int x) const
{
	DrawKoma(masu.sengo, masu.type, y, x);
}

void BoardSiv3D::DrawKoma(int sengo, int type, int y, int x, int maisu, bool isChoice) const
{
	const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
	const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

	if (isChoice)
	{
		const EKomaType nariType = mKoma[type].narigoma;

		NariOffsetY(y);

		{
			const int drawX = leftX + (BOARD_SIZE - 1.5f - x)*mKomaTextureWidth;
			const int drawY = topY + y*mKomaTextureHeight;
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw({ 128, 128, 0, 192 });
			mTexture[sengo][type].draw(drawX, drawY);
		}

		{
			const int drawX = leftX + (BOARD_SIZE - 0.5f - x)*mKomaTextureWidth;
			const int drawY = topY + y*mKomaTextureHeight;
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw({ 255, 0, 0, 192 });
			mTexture[sengo][nariType].draw(drawX, drawY);
		}
	}
	else
	{
		mTexture[sengo][type].draw(leftX + (BOARD_SIZE - 1 - x)*mKomaTextureWidth, topY + y*mKomaTextureHeight);
	}

	if (maisu >= 1)
	{
		mFont(L"", maisu).draw(leftX + (BOARD_SIZE - 1 - x + 1)*mKomaTextureWidth, topY + y*mKomaTextureHeight);
	}
}

void BoardSiv3D::SetOffset(int offsetX, int offsetY)
{
	mOffsetX = offsetX;
	mOffsetY = offsetY;
}


bool BoardSiv3D::GetGridPosFromMouse(GridPos& gridPos) const
{
	const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
	const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
	const int x = BOARD_SIZE - 1 - (Mouse::Pos().x - leftX) / mKomaTextureWidth;
	const int y = (Mouse::Pos().y - topY) / mKomaTextureHeight;

	if (!(INRANGE(x, 0, BOARD_SIZE - 1) && INRANGE(y, 0, BOARD_SIZE - 1)))
	{
		return false;
	}

	// こまだいチェックはあとで実装！
	gridPos.x = x;
	gridPos.y = y;
	return true;
}

void BoardSiv3D::GetXYNaruNarazuChoice(int& y, int& x) const
{
	const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
	const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

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
	const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
	const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

	if (Mouse::Pos().x - leftX >= 0)
	{
		x = BOARD_SIZE - static_cast<float>(Mouse::Pos().x - leftX) / mKomaTextureWidth;
	}
	else
	{
		x = BOARD_SIZE + static_cast<float>(leftX - Mouse::Pos().x) / mKomaTextureWidth;
	}

	y = (Mouse::Pos().y - topY) / mKomaTextureHeight;
}

void BoardSiv3D::Update()
{
	if (Input::MouseL.clicked)
	{
		switch (mInputState)
		{
		case E_IDLE:
		{
			// 駒をつかむ
			GridPos gp;
			if (GetGridPosFromMouse(gp))
			{
				// 自分の駒だけ！
				if (mGrid[gp.y][gp.x].sengo == mTeban && mGrid[gp.y][gp.x].type != E_EMPTY)
				{
					mGrabbedPos = gp;
					mInputState = E_GRABBED;
				}
			}
			else if (GetHarukomaType(mHaruKomaType))
			{
				mInputState = E_HARU;
			}
		}
		break;

		case E_GRABBED:
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp) && gp != mGrabbedPos)
			{
				// TODO : 合法手のチェック
				// 成りのチェック
				mMoveToPos = gp;
				if (IsNareru(mGrabbedPos, gp, mTeban))
				{
					// 成り選択画面へ
					mInputState = E_CHOICE;
				}
				else
				{
					DoMove(false);
				}
			}


			// TODO:変なところで駒を離したらキャンセル
		}
		break;

		case E_HARU:
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp) && mGrid[gp.y][gp.x].type == E_EMPTY)
			{
				mMoveToPos = gp;

				mMochigoma[mTeban][mHaruKomaType]--;
				mGrid[mMoveToPos.y][mMoveToPos.x].sengo = mTeban;
				mGrid[mMoveToPos.y][mMoveToPos.x].type = mHaruKomaType;

				mInputState = E_IDLE;
				mTeban = static_cast<ESengo>(1 - mTeban);
			}
		}
		break;

		case E_CHOICE:
			// 成り成らずを選択する
			if (IsNaruChoice() || IsNarazuChoice())
			{
				DoMove(IsNaruChoice());
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

		case E_HARU:
		case E_GRABBED:
		case E_CHOICE:
			mInputState = E_IDLE;
			mHaruKomaType = E_EMPTY;
			break;

		default:
			break;
		}
	}
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

						SetSFEN(line.substr(st, len));
						break;
					}
				}
				assert(st != NG);
			}
		}
	}
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

void BoardSiv3D::GetMochigomaPos(int s, int k, int& y, int& x) const
{
	const int posX[2] = { -2, 10 };
	const int posY[2] = { 1,  7 };
	const int signY[2] = { +1, -1 };

	y = k*signY[s] + posY[s];
	x = posX[s];
}


bool BoardSiv3D::GetHarukomaType(EKomaType& harukomaType) const
{
	int y, x;
	GetXYNaruNarazuChoice(y, x);
	for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
	{
		if (mMochigoma[mTeban][k] >= 1)
		{
			int my, mx;
			GetMochigomaPos(mTeban, k, my, mx);
			if (my == y && mx == x)
			{
				harukomaType = static_cast<EKomaType>(k);
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

	float cy = mMoveToPos.y;
	float cx = mMoveToPos.x;
	NariOffsetY(cy);

	return (INRANGE(x, cx - 0.5f, cx + 0.5f) && INRANGE(y, cy, cy + 1.0f));
}

bool BoardSiv3D::IsNarazuChoice() const
{
	float y, x;
	GetXYNaruNarazuChoice(y, x);

	float cy = mMoveToPos.y;
	float cx = mMoveToPos.x;
	NariOffsetY(cy);

	return (INRANGE(x, cx + 0.5f, cx + 1.5f) && INRANGE(y, cy, cy + 1.0f));
}