// tree_siv3d.h : Siv3Dに依存する棋譜の木＋将棋盤以外のSiv3DのGUIなど。

#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "evaluator.h"
#include "tree.h"

// TreeSiv3D : 将棋盤以外の部分すべて（右側のツリー表示と、左側の将棋盤以外）
class TreeSiv3D : public Tree
{
public:
	enum EGUICheckBox
	{
		SHOW_SCORE,			// "評価値" 
		SHOW_TE,			// "指し手"
		SHOW_TAG,			// "タグ"
		SMALL_NODE,			// "小さいノード"
		FIX_SELECTED_NODE,	// "選択ノードの固定"
		SHOW_ARROW,			// "矢印"
		SHOW_DEBUG,			// "デバッグ"
	};


	TreeSiv3D(Board* board) : Tree(board), mEvaluator(this)
	{
		mOffsetX = RIGHT_CENTER_X;
		mOffsetY = RIGHT_CENTER_Y;
		mGridScale = 40.f;
		mFont = Font(8, L"メイリオ");
		mFontScore = Font(11, L"Impact");
		mFontScoreMedium = Font(9, L"Impact");
		mFontScoreSmall = Font(5, L"MS UI Gothic", FontStyle::Bitmap); // L"Consolas");
		mFontGuiDefault = GUIManager::GetDefaultFont();

		GUIStyle style = GUIStyle::Default;
		style.font = mFont;
		style.background.color = Color(255, 0, 255, 64);
		style.padding.bottom = 0;
		style.padding.top = 0;
		style.padding.left = 0;
		style.padding.right = 0;

		WidgetStyle widgetStyle;
		widgetStyle.font = mFont;
		widgetStyle.color = Color(255, 255, 255, 255);


		GUIStyle style2 = style;
		style2.background.color = Color(255, 255, 255, 196);

		WidgetStyle widgetStyle2 = widgetStyle;
		widgetStyle2.color = Color(0, 0, 0, 255);
		widgetStyle2.margin = Margin(2);

		GUIStyle style3 = style;
		style3.background.color = Color(255, 0, 255, 64);

		GUIStyle style4 = style;
		style4.background.color = Color(255, 0, 255, 64);

		mGuiFile = GUI(style2);
		mGuiFile.setTitle(L"ファイル");
		mGuiFile.addln(L"kifu_load", GUIButton::Create(L"定跡ファイルを開く", widgetStyle2));
		mGuiFile.addln(L"kifu_save", GUIButton::Create(L"定跡ファイルを保存", widgetStyle2));
		mGuiFile.addln(L"kif_format_load", GUIButton::Create(L"kifファイルを開く", widgetStyle2));

		mGuiEvaluator = GUI(style3);
		mGuiEvaluator.setTitle(L"評価ソフト");
		mGuiEvaluator.setPos(144, 0);
		mGuiEvaluator.add(L"evaluator_load", GUIButton::Create(L"評価ソフトを開く", widgetStyle2));
		mGuiEvaluator.addln(L"evaluator_name", GUIText::Create(L"", widgetStyle));
		mGuiEvaluator.add(L"option_load", GUIButton::Create(L"オプションを開く", widgetStyle2));
		mGuiEvaluator.add(L"option_name", GUIText::Create(L"", widgetStyle));
		mGuiEvaluator.add(L"time_prefix", GUIText::Create(L"　　思考時間", widgetStyle));
		mGuiEvaluator.add(L"time_sec", GUITextField::Create(2, widgetStyle2));
		mGuiEvaluator.addln(L"time_suffix", GUIText::Create(L"秒", widgetStyle));


		String x = CharacterSet::Widen(to_string(mEvaluator.GetDurationSec()));
		mGuiEvaluator.textField(L"time_sec").setText(x);

		const int syogibanY = 90;

		mGuiScore = GUI(style);
		mGuiScore.setTitle(L"評価値");
		mGuiScore.setPos(mShogibanWidth, syogibanY);
		mGuiScore.addln(L"score", GUIText::Create(L"", widgetStyle));
		mGuiScore.addln(L"tejunJap", GUIText::Create(L"", widgetStyle));
		
		mGuiSettings = GUI(style2);
		mGuiSettings.setTitle(L"設定");
		mGuiSettings.setPos(WINDOW_W - 114, 0);
		mGuiSettings.add(L"settings", 
			GUICheckBox::Create({ L"評価値", L"指し手", L"コメント", L"小さいノード", L"選択ノードの固定", L"矢印", L"デバッグ" },
		                        { SHOW_SCORE, SHOW_TE, SHOW_TAG, FIX_SELECTED_NODE, SHOW_ARROW }, true, widgetStyle2));

		mGuiDelete = GUI(style4);
		mGuiDelete.setPos(WINDOW_W - 99, 158);
		mGuiDelete.setTitle(L"削除 [Del]キー");
		mGuiDelete.addln(L"delete_score", GUIButton::Create(L"評価値１つ", false, widgetStyle2));
		mGuiDelete.addln(L"delete_all_score", GUIButton::Create(L"評価値全て", false, widgetStyle2));
		mGuiDelete.addln(L"delete_all_node", GUIButton::Create(L"局面全て", false, widgetStyle2));


		mTextureBackground = Texture(L"pictures/background.jpg");
		mNodeSelectSound = Sound(L"sounds/decision22.mp3");

		GUIStyle style5 = style;
		style5.background.color = Color(255, 255, 255, 0);
		mGuiBoard = GUI(style5);
		mGuiBoard.setPos(0, syogibanY);
		mGuiBoard.addln(L"summary", GUITextField::Create(33));
		mGuiBoard.setTitle(L"将棋盤");
	}

	virtual void Draw() override;
	virtual void Update() override;
	virtual void OnSelectedNodeIDChanged() override;

	long long GetPonderNodes() const { return mEvaluator.GetPonderNodes();}
	long long GetPonderTime() const { return mEvaluator.GetPonderTime(); }


private:
	TreeSiv3D()	{};

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
		return Window::Width()*0.75f; // TODO これは指定可能なようにしたい。例えば左の将棋盤を置きたい人はどうするの？
	}

	float GetTreeCenterY() const
	{
		return Window::Height()*0.5f; // TODO これは指定可能なようにしたい。例えば左の将棋盤を置きたい人はどうするの？
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
	s3d::RoundRect GetNodeShape(int centerX, int centerY, NodeSize nodeSize) const;
	void DrawBeforeBoard() const;
	void DrawAfterBoard() const;
	void DrawScore(int centerX, int centerY, const Node& node, NodeSize nodeSize) const;

	Font mFont;
	Font mFontScore;
	Font mFontScoreMedium;
	Font mFontScoreSmall;
	Font mFontGuiDefault;
	GUI mGuiFile;
	GUI mGuiEvaluator;
	GUI mGuiScore;
	GUI mGuiSettings;
	GUI mGuiDelete;
	GUI mGuiBoard;
	Texture mTextureBackground;
	Sound mNodeSelectSound;

	float mOffsetX;
	float mOffsetY;
	float mGridScale;

	Evaluator	mEvaluator; // TODO : 評価ソフトなので、treeに移動したほうが良いのでは。ただファイル読み込みとかがちがちにSIV3D使っているので、ちょっと移動は大変かも。

	const int mShogibanWidth  = 613;
	const int mShogibanHeight = 523;

};

