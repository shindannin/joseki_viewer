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
		mFont = Font(10, L"���C���I");
		mGui = GUI(GUIStyle::Default);
		mGui.setTitle(L"���j���[");
		mGui.add(L"kifu_load", GUIButton::Create(L"��Ղ̃��[�h"));
		mGui.add(L"kifu_save", GUIButton::Create(L"��Ղ̃Z�[�u"));
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

