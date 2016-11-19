#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "board.h"

using namespace std;
using namespace s3d;

class BoardSiv3D : public Board
{
public:
	BoardSiv3D();
	virtual ~BoardSiv3D() override;
	virtual void Draw() override;
	virtual bool Update(string& te, wstring& teJap) override;

	void DrawCursor(const GridPos& gp, const Color& color) const;
	void DrawKoma(const Masu& masu, int y, int x) const;
	void DrawKoma(int sengo, int type, int y, int x, int maisu = 0, bool isChoice = false) const;
	void SetOffset(int offsetX, int offsetY);
	bool GetGridPosFromMouse(GridPos& gridPos) const;
	void GetXYNaruNarazuChoice(int& y, int& x) const;
	void GetXYNaruNarazuChoice(float& y, float& x) const;
	template <class T> void NariOffsetY(T& y) const;
	void GetMochigomaPos(int s, int k, int& y, int& x) const;
	bool CalcUtsuKomaType(EKomaType& harukomaType) const;
	bool IsNaruChoice() const;
	bool IsNarazuChoice() const;
	int GetGridLeftX() const;
	int GetGridTopY() const;

private:
	void UpdateDecided(string& te, wstring& teJap, bool& isMoved);

	const wstring mFileName[NUM_KOMA_TYPE][NUM_SEN_GO] =
	{
		{ L"Sou", L"Gou" },
		{ L"Shi", L"Ghi" },
		{ L"Skaku", L"Gkaku" },
		{ L"Skin", L"Gkin" },
		{ L"Sgin", L"Ggin" },
		{ L"Skei", L"Gkei" },
		{ L"Skyo", L"Gkyo" },
		{ L"Sfu", L"Gfu" },

		{ L"Sryu", L"Gryu" },
		{ L"Suma", L"Guma" },
		{ L"Sngin", L"Gngin" },
		{ L"Snkei", L"Gnkei" },
		{ L"Snkyo", L"Gnkyo" },
		{ L"Sto", L"Gto" },
	};

	void InitServer();

	vector <Texture> mTexture[NUM_SEN_GO];
	Texture mTextureBoard;
	Texture mTextureGrid;
	Font mFont;

	int mOffsetX;
	int mOffsetY;
	int mKomaTextureWidth;
	int mKomaTextureHeight;
	int mBoardTextureWidth;
	int mBoardTextureHeight;
	EInputState mInputState = E_IDLE;
};
