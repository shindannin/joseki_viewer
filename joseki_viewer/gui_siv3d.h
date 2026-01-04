#pragma once

#include <Siv3D.hpp>
#include "color.h"
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
		SHOW_ARROW,			// "矢印"
		SHOW_SCOREGRAPH,	// "評価値グラフ"
		FIX_SELECTED_NODE,	// "選択ノードの固定"
		FIX_UPDATED_NODE,	// "評価ノードの固定"
		REVERSE_BOARD,		// "盤面反転"
		SOUND,				// "サウンド"
		SHOW_DEBUG,			// "デバッグ"
	};

	GuiSiv3D()
	{
		mFont = Font(8, L"メイリオ");

		GUIStyle style = GUIStyle::Default;
		style.font = mFont;
		style.background.color = COLOR_NEUTRAL;
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
		style3.background.color = COLOR_NEUTRAL;



		GUIStyle style4 = style;
		style4.background.color = COLOR_NEUTRAL;

		mFile = GUI(style2);
		mFile.setTitle(L"ファイル");
		mFile.addln(L"kifu_load", GUIButton::Create(L"ファイルを開く", widgetStyle2));
		mFile.addln(L"kifu_save", GUIButton::Create(L"ファイルを保存", widgetStyle2));
		mFile.addln(L"kif_format_load", GUIButton::Create(L"kifファイルを開く", widgetStyle2));

		mEvaluator = GUI(style3);
		mEvaluator.setTitle(L"評価ソフト");
		mEvaluator.setPos(132, 0);
		mEvaluator.add(L"evaluator_load", GUIButton::Create(L"評価ソフトを開く", widgetStyle2));
		mEvaluator.addln(L"evaluator_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"option_load", GUIButton::Create(L"オプションを開く", widgetStyle2));
		mEvaluator.add(L"option_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"time_prefix", GUIText::Create(L"　　思考時間", widgetStyle));
		mEvaluator.add(L"time_sec", GUITextField::Create(DEFAULT_EVALUATION_SEC, widgetStyle2, Size(36, 18)));
		mEvaluator.add(L"time_suffix", GUIText::Create(L"秒", widgetStyle));
		mEvaluator.add(L"multipv_prefix", GUIText::Create(L"　候補数", widgetStyle));
		mEvaluator.add(L"multipv_num", GUITextField::Create(DEFAULT_MULTIPV_NUM, widgetStyle2, Size(36, 18)));
		mEvaluator.add(L"best_arrow_label", GUIText::Create(L"　最善手数", widgetStyle));
		mEvaluator.addln(L"best_arrow_depth", GUITextField::Create(DEFAULT_BEST_ARROW_DEPTH, widgetStyle2, Size(36, 18)));
		mEvaluator.textField(L"time_sec").setText(to_wstring(DEFAULT_EVALUATION_SEC));
		mEvaluator.textField(L"multipv_num").setText(to_wstring(DEFAULT_MULTIPV_NUM));
		mEvaluator.textField(L"best_arrow_depth").setText(to_wstring(DEFAULT_BEST_ARROW_DEPTH));
		mEvaluator.add(L"folder_analysis", GUIButton::Create(L"フォルダ内連続解析", widgetStyle2));
		mEvaluator.add(L"folder_analysis_waiting_num", GUIText::Create(L"0", widgetStyle));

		const int syogibanY = 100;

		mScore = GUI(style);
		mScore.setTitle(L"評価値");
		mScore.setPos(SHOGIBAN_W, syogibanY);
		mScore.addln(L"score", GUIText::Create(L"", widgetStyle));
		mScore.addln(L"tejunJap", GUIText::Create(L"", widgetStyle));

		mSettings = GUI(style2);
		mSettings.setTitle(L"設定");
		mSettings.setPos(WINDOW_W - 114, 0);
		mSettings.add(L"settings",
			GUICheckBox::Create({ L"評価値", L"指し手", L"タグ", L"小さいノード", L"矢印", L"評価値グラフ", L"選択ノードの固定", L"評価ノードの固定", L"盤面反転", L"サウンド", L"デバッグ" },
				{ SHOW_SCORE, SHOW_TE, SHOW_TAG, FIX_SELECTED_NODE, SHOW_ARROW, SHOW_SCOREGRAPH, SOUND }, // デフォルトでONのもの
				true, widgetStyle2));

		mDelete = GUI(style4);
		mDelete.setPos(WINDOW_W - 114, 234);
		mDelete.setTitle(L"削除 [Del]キー");
		mDelete.addln(L"delete_score", GUIButton::Create(L"評価値   １つ", false, widgetStyle2));
		mDelete.addln(L"delete_all_score", GUIButton::Create(L"評価値   全て", false, widgetStyle2));
		mDelete.addln(L"delete_all_node", GUIButton::Create(L"局面   全て", false, widgetStyle2));


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
