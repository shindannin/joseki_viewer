#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "board.h"

using namespace std;
using namespace s3d;

// 現在の状態。
enum EInputState
{
	E_IDLE,		// どの駒を動かすか選択中
	E_GRABBED,	// 掴んだ駒をどこへ動かすか選択中
	E_UTSU,		// 持ち駒をどこへ動かすか選択中
	E_CHOICE,	// 成るかどうか選択中
};

class GuiSiv3D;

class BoardSiv3D : public Board
{
public:
	BoardSiv3D(GuiSiv3D& gui);
	virtual ~BoardSiv3D() override;
	virtual void Draw() override;
	virtual bool Update(string& te, wstring& teJap) override;
	virtual void Init() override;

	void DrawCursor(const GridPos& gp, const Color& color) const;
	void DrawKoma(const Masu& masu, int y, int x) const;
	void DrawKoma(int baseSengo, int type, int y, int x, int maisu = 0, bool isChoice = false, bool ignoreReverse = false) const;
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
	void DrawMove(const string& te, const Color& color, int& cy, int& cx) const;
	bool IsInputStateIdle() const { return mInputState==E_IDLE; }

private:
	void DrawArrow(int startY, int startX, int destY, int destX, const Color& color, int& cy, int& cx) const;
	void UpdateDecided(string& te, wstring& teJap, bool& isMoved);
	bool GetReverse() const;

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

	const Color cursorColor[NUM_SEN_GO] = 
	{
		{ 128, 0, 0, 127 },
		{ 0, 0, 128, 127 },
	};

	const Color grabbedColor[NUM_SEN_GO] =
	{
		{ 0xc9,0x3a,0x40,127 },
		{ 0x00,0x74,0xbf,127 },
	};

	const Color nariColor[NUM_SEN_GO] =
	{
		{ 0xc9,0x3a,0x40, 192 },
		{ 0x00,0x74,0xbf, 192 },
	};

	const Color narazuColor[NUM_SEN_GO] =
	{
		{ 128, 128, 0, 192 },
		{ 0, 128, 128, 192 },
	};

	const Color fontColor[NUM_SEN_GO] =
	{
		{ 255, 192, 192, 255 },
		{ 192, 192, 255, 255 },
	};

	const Color maisuFontColor[NUM_SEN_GO] =
	{
		{ 0xc9, 0, 0, 255 },
		{ 0, 0, 0xbf, 255 },
	};

	vector <Texture> mTexture[NUM_SEN_GO];
	Texture mTextureBoard;
	Texture mTextureGrid;
	Font mFont;
	Font mFontTeban;
	Sound mSoundKomaoto;
	GuiSiv3D& mGui;

	int mOffsetX;
	int mOffsetY;
	int mKomaTextureWidth;
	int mKomaTextureHeight;
	int mBoardTextureWidth;
	int mBoardTextureHeight;
	EInputState mInputState = E_IDLE;
};
