#pragma once

#include <Siv3D.hpp>
#include "util.h"

class GuiSiv3D
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
		REVERSE_BOARD,		// "盤面反転"
		SHOW_DEBUG,			// "デバッグ"
	};

	GuiSiv3D()
	{
		mFont = Font(8, L"メイリオ");

		const Color NeutralColor{ 0x94, 0x60, 0xa0, 0x40 };

		GUIStyle style = GUIStyle::Default;
		style.font = mFont;
		style.background.color = NeutralColor;
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
		style3.background.color = NeutralColor;



		GUIStyle style4 = style;
		style4.background.color = NeutralColor;

		mFile = GUI(style2);
		mFile.setTitle(L"ファイル");
		mFile.addln(L"kifu_load", GUIButton::Create(L"定跡ファイルを開く", widgetStyle2));
		mFile.addln(L"kifu_save", GUIButton::Create(L"定跡ファイルを保存", widgetStyle2));
		mFile.addln(L"kif_format_load", GUIButton::Create(L"kifファイルを開く", widgetStyle2));

		mEvaluator = GUI(style3);
		mEvaluator.setTitle(L"評価ソフト");
		mEvaluator.setPos(144, 0);
		mEvaluator.add(L"evaluator_load", GUIButton::Create(L"評価ソフトを開く", widgetStyle2));
		mEvaluator.addln(L"evaluator_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"option_load", GUIButton::Create(L"オプションを開く", widgetStyle2));
		mEvaluator.add(L"option_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"time_prefix", GUIText::Create(L"　　思考時間", widgetStyle));
		mEvaluator.add(L"time_sec", GUITextField::Create(2, widgetStyle2));
		mEvaluator.addln(L"time_suffix", GUIText::Create(L"秒", widgetStyle));


		mEvaluator.textField(L"time_sec").setText(L"0");

		const int syogibanY = 90;

		mScore = GUI(style);
		mScore.setTitle(L"評価値");
		mScore.setPos(SHOGIBAN_W, syogibanY);
		mScore.addln(L"score", GUIText::Create(L"", widgetStyle));
		mScore.addln(L"tejunJap", GUIText::Create(L"", widgetStyle));

		mSettings = GUI(style2);
		mSettings.setTitle(L"設定");
		mSettings.setPos(WINDOW_W - 114, 0);
		mSettings.add(L"settings",
			GUICheckBox::Create({ L"評価値", L"指し手", L"コメント", L"小さいノード", L"選択ノードの固定", L"矢印", L"盤面反転", L"デバッグ" },
				{ SHOW_SCORE, SHOW_TE, SHOW_TAG, FIX_SELECTED_NODE, SHOW_ARROW }, // デフォルトでONのもの
				true, widgetStyle2));

		mDelete = GUI(style4);
		mDelete.setPos(WINDOW_W - 99, 180);
		mDelete.setTitle(L"削除 [Del]キー");
		mDelete.addln(L"delete_score", GUIButton::Create(L"評価値１つ", false, widgetStyle2));
		mDelete.addln(L"delete_all_score", GUIButton::Create(L"評価値全て", false, widgetStyle2));
		mDelete.addln(L"delete_all_node", GUIButton::Create(L"局面全て", false, widgetStyle2));


		GUIStyle style5 = style;
		style5.background.color = Color(255, 255, 255, 0);
		mBoard = GUI(style5);
		mBoard.setPos(0, syogibanY);
		mBoard.addln(L"summary", GUITextField::Create(33));
		mBoard.setTitle(L"将棋盤");
	}

	GUI mFile;
	GUI mEvaluator;
	GUI mScore;
	GUI mSettings;
	GUI mDelete;
	GUI mBoard;

	Font mFont;
};