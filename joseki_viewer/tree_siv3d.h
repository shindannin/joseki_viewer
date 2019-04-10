// tree_siv3d.h : Siv3D�Ɉˑ���������̖؁{�����ՈȊO��Siv3D��GUI�ȂǁB

#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "evaluator.h"
#include "tree.h"

class GuiSiv3D;

// TreeSiv3D : �����ՈȊO�̕������ׂāi�E���̃c���[�\���ƁA�����̏����ՈȊO�j
class TreeSiv3D : public Tree
{
public:

	TreeSiv3D(Board* board, GuiSiv3D& gui) : Tree(board), mEvaluator(this), mGui(gui)
	{
		mOffsetX = RIGHT_CENTER_X;
		mOffsetY = RIGHT_CENTER_Y;
		mGridScale = 40.f;
		mFont = Font(8, L"���C���I");
		mFontScore = Font(11, L"Impact");
		mFontScoreMedium = Font(9, L"Impact");
		mFontScoreSmall = Font(5, L"MS UI Gothic", FontStyle::Bitmap); // L"Consolas");
		mFontGuiDefault = GUIManager::GetDefaultFont();

		mTextureBackground = Texture(L"pictures/background.jpg");
		mNodeSelectSound = Sound(L"sounds/cursor4.mp3");

	}

	virtual void Draw() override;
	virtual void Update() override;
	virtual void OnSelectedNodeIDChanged() override;

	long long GetPonderNodes() const { return mEvaluator.GetPonderNodes();}
	long long GetPonderTime() const { return mEvaluator.GetPonderTime(); }


private:

	float ScaleX(float x) const
	{
		return mOffsetX + x * mGridScale;
	}

	float ScaleY(float y) const
	{
		return mOffsetY + y * mGridScale;
	}

	float InvScaleX(float scaledX) const
	{
		assert(mGridScale > 0.f);
		return (scaledX - mOffsetX) / mGridScale;
	}

	float InvScaleY(float scaledY) const
	{
		assert(mGridScale > 0.f);
		return (scaledY - mOffsetY) / mGridScale;;
	}

	float GetTreeCenterX() const
	{
		return Window::Width()*0.75f; // TODO ����͎w��\�Ȃ悤�ɂ������B�Ⴆ�΍��̏����Ղ�u�������l�͂ǂ�����́H
	}

	float GetTreeCenterY() const
	{
		return Window::Height()*0.5f; // TODO ����͎w��\�Ȃ悤�ɂ������B�Ⴆ�΍��̏����Ղ�u�������l�͂ǂ�����́H
	}

//	void DrawScoreBar(int score, int maxScore, float cx, float cy, float w, float h);

	enum NodeSize
	{
		NS_BIG,
		NS_MEDIUM,
		NS_SMALL,
		NUM_NS,
	};

	void PlayNodeSelectSound();
	bool IsInShogiban(int x, int y) const;
	bool IsInScoreGraph(int x, int y) const;
	s3d::RoundRect GetNodeShape(int centerX, int centerY, NodeSize nodeSize) const;
	void DrawBeforeBoard() const;
	void DrawAfterBoard() const;
	void DrawScore(int centerX, int centerY, const Node& node, NodeSize nodeSize) const;

	Font mFont;
	Font mFontScore;
	Font mFontScoreMedium;
	Font mFontScoreSmall;
	Font mFontGuiDefault;
	Texture mTextureBackground;
	Sound mNodeSelectSound;
	GuiSiv3D& mGui;

	float mOffsetX;
	float mOffsetY;
	float mGridScale;

	Evaluator	mEvaluator; // TODO : �]���\�t�g�Ȃ̂ŁAtree�Ɉړ������ق����ǂ��̂ł́B�����t�@�C���ǂݍ��݂Ƃ�����������SIV3D�g���Ă���̂ŁA������ƈړ��͑�ς����B
};

