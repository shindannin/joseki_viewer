#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "tree.h"

class TreeSiv3D : public Tree
{
public:
	TreeSiv3D(Board* board) : Tree(board)
	{
		mOffsetX = 960;
		mOffsetY = 100;
		mNodeRadius = 12.f;
		mGridScale = 40.f;
		mFont = Font(10, L"メイリオ");
		mGui = GUI(GUIStyle::Default);
		mGui.setTitle(L"メニュー");
		mGui.add(L"kifu_load", GUIButton::Create(L"定跡のロード"));
		mGui.add(L"kifu_save", GUIButton::Create(L"定跡のセーブ"));
	}

	virtual ~TreeSiv3D() override {};
	virtual void Draw() override;
	virtual void Update() override;

private:
	TreeSiv3D()	{};

	float ScaleX(int x) const
	{
		return mOffsetX + x * mGridScale;
	}

	float ScaleY(int y) const
	{
		return mOffsetY + y * mGridScale;
	}

	Font mFont;
	GUI mGui;
	
	float mOffsetX;
	float mOffsetY;
	float mNodeRadius;
	float mGridScale;
};

