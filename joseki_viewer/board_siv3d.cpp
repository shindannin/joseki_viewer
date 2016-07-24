#include "board_siv3d.h"


#include <cassert>
#include <algorithm>
#include <numeric>



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

	mInputState = E_IDLE;

	SetOffset(100, 100);

	mServer = nullptr;

	///////////////////////////////////
	//
	// クライアントを起動
	//
	const auto path = Dialog::GetOpen({ { L"実行ファイル (*.exe)", L"*.exe" } });

	if (path)
	{
		mServer = new Server(path.value(), false);
	}
}

BoardSiv3D::~BoardSiv3D()
{
	if (mServer != nullptr)
	{
		mServer->write("quit\n");

		delete mServer;
		mServer = nullptr;
	}

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
	Rect(leftX + (BOARD_SIZE - 1 - gp.x) * mKomaTextureWidth, topY + gp.y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw(color);
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
			DrawCursor(gp, { 128, 0, 0, 127 });
		}
	}

	// つかんだ駒のマス
	if (mInputState == E_GRABBED || mInputState == E_CHOICE)
	{
		DrawCursor(GetMoveFromPos(), { 255, 0, 0, 127 });
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
					GetMochigomaPos(s, k, y, x);
					DrawKoma(s, k, y, x, numKoma);
				}
			}
		}
	}

	//----- 手番 -----
	{
		wstring name[2] = { L"先手番", L"後手番" };
		mFont(name[GetTeban()]).draw(leftX + tebanOffestX * mKomaTextureWidth, topY);

		mFont(mScore).draw(leftX + tebanOffestX * mKomaTextureWidth, topY + 30);
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

			if (GetMasu(y,x).type != E_EMPTY)
			{
				DrawKoma(GetMasu(y,x), y, x);
			}
		}
	}

	// つかんだ駒

	// TODO : ここ、もうちょっとリファクタリングできそう。
	if (mInputState == E_GRABBED || mInputState == E_CHOICE)
	{
		const Masu& masu = GetMasu(GetMoveFromPos());
		mTexture[masu.sengo][masu.type].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
	}
	else if (mInputState == E_UTSU)
	{
		mTexture[GetTeban()][GetUtsuKomaType()].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
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

void BoardSiv3D::DrawKoma(int sengo, int type, int y, int x, int maisu, bool isChoice) const
{
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();

	if (isChoice)
	{
		const EKomaType nariType = mKoma[type].narigoma;

		NariOffsetY(y);

		{
			const int drawX = static_cast<int>(leftX + (BOARD_SIZE - 1.5f - x)*mKomaTextureWidth);
			const int drawY = static_cast<int>(topY + y*mKomaTextureHeight);
			Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw({ 128, 128, 0, 192 });
			mTexture[sengo][type].draw(drawX, drawY);
		}

		{
			const int drawX = static_cast<int>(leftX + (BOARD_SIZE - 0.5f - x)*mKomaTextureWidth);
			const int drawY = static_cast<int>(topY + y*mKomaTextureHeight);
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
	const int leftX = GetGridLeftX();
	const int topY = GetGridTopY();
	const int x = BOARD_SIZE - 1 - (Mouse::Pos().x - leftX) / mKomaTextureWidth;
	const int y = (Mouse::Pos().y - topY) / mKomaTextureHeight;

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

void BoardSiv3D::InitServer()
{
	if (mServer)
	{
		if (mServer->write("usi\n"))
		{
			std::string readStr;

			System::Sleep(500);
			if (mServer->read(readStr))
			{
				const vector <string> options = {
					"isready\n",
				};

				string allOptions = accumulate(options.begin(), options.end(), string() );

				if (mServer->write(allOptions))
				{
					System::Sleep(500);
					if (mServer->read(readStr))
					{
						std::wstring wsTmp(readStr.begin(), readStr.end());
						Print(wsTmp);
						if (mServer->write("usinewgame\n"))
						{
						}
					}
				}
			}
		}
	}

}

int BoardSiv3D::CalcBestMoveAndScore()
{
	string writeStr;
	writeStr += "position sfen " + GetState()+ "\n";

	{
		std::wstring wsTmp(writeStr.begin(), writeStr.end());
		Print(wsTmp);
	}

	if (mServer->write(writeStr))
	{
		writeStr = "go btime 0 wtime 0 byoyomi 4000\n";
		mServer->write(writeStr);
			
		System::Sleep(4000);

		
		string readStr;
		if (mServer->read(readStr))
		{
			vector <string> vs;
			Split1(readStr, vs, '\n');

			for (int i=SZ(vs)-1; i>=0; --i)
			{
				const string& lastInfo = vs[i];
				vector <string> tmp;
				Split1(lastInfo, tmp, ' ');

				for (int k = 0; k < SZ(tmp); ++k)
				{
					if (tmp[k] == "score")
					{
						mScore = stoi(tmp[k + 2]);
						std::wstring wsTmp(lastInfo.begin(), lastInfo.end());
						Print(wsTmp);
						goto NUKE;
					}
				}
			}
		}

	NUKE:;
	}

	return mScore;
}

void BoardSiv3D::UpdateDecided(string& te, wstring& teJap, bool& isMoved)
{
	te = GetTeFromMove(GetNextMove());
	teJap = DecideMove();
	isMoved = true;
	mInputState = E_IDLE;
}

bool BoardSiv3D::Update(string& te, wstring& teJap)
{
	bool isMoved = false;

	if (Input::KeyA.released)
	{
		if(mServer==nullptr)
		{
			InitServer();
		}
		CalcBestMoveAndScore();
	}
	else if (Input::MouseL.clicked)
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
				if (GetMasu(gp).sengo == GetTeban() && GetMasu(gp).type != E_EMPTY)
				{
					SetMoveFromPos(gp);
					mInputState = E_GRABBED;
				}
			}
			else
			{
				EKomaType k;
				if (CalcUtsuKomaType(k))
				{
					SetUtsuKomaType(k);
					mInputState = E_UTSU;
				}
			}
		}
		break;

		case E_GRABBED:
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp) &&
				gp != GetMoveFromPos() &&   // 移動先移動元と同じではダメ
				GetMasu(gp).sengo != GetTeban()	// 自分の駒のあるところには動かせない。
				)
			{
				// TODO : 合法手のチェック
				


				// 成りのチェック
				SetMoveToPos(gp);
				if (IsNareru(GetMoveFromPos(), gp, GetTeban()))
				{
					// 成り選択画面へ
					mInputState = E_CHOICE;
				}
				else
				{
					UpdateDecided(te, teJap, isMoved);
				}
			}


			// TODO:変なところで駒を離したらキャンセル
		}
		break;

		case E_UTSU:
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp) && GetMasu(gp).type == E_EMPTY)
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

void BoardSiv3D::GetMochigomaPos(int s, int k, int& y, int& x) const
{
	const int posX[2] = { -2, 10 };
	const int posY[2] = { 1,  7 };
	const int signY[2] = { +1, -1 };

	y = k*signY[s] + posY[s];
	x = posX[s];
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

	float		cy = static_cast<float>(GetMoveToPos().y);
	const float	cx = static_cast<float>(GetMoveToPos().x);
	NariOffsetY(cy);

	return (INRANGE(x, cx - 0.5f, cx + 0.5f) && INRANGE(y, cy, cy + 1.0f));
}

bool BoardSiv3D::IsNarazuChoice() const
{
	float y, x;
	GetXYNaruNarazuChoice(y, x);

	float		cy = static_cast<float>(GetMoveToPos().y);
	const float	cx = static_cast<float>(GetMoveToPos().x);
	NariOffsetY(cy);

	return (INRANGE(x, cx + 0.5f, cx + 1.5f) && INRANGE(y, cy, cy + 1.0f));
}