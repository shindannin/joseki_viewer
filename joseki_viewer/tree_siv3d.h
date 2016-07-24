#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "tree.h"

class TreeSiv3D : public Tree
{
public:
	TreeSiv3D() 
	{
		Init();
	};


	TreeSiv3D(Board* board) : Tree(board) { Init(); };
	virtual ~TreeSiv3D() override {};
	virtual void Draw() override;
	virtual void Update() override;

private:
	void Init()
	{
		mOffsetX = 960;
		mOffsetY = 100;
		mNodeRadius = 8.f;
		mGridScale = 30.f;
	}

	float ScaleX(int x) const
	{
		return mOffsetX + x * mGridScale;
	}

	float ScaleY(int y) const
	{
		return mOffsetY + y * mGridScale;
	}


	int mOffsetX;
	int mOffsetY;
	float mNodeRadius;
	float mGridScale;
};

