#pragma once

#include <Siv3D.hpp>
#include "color.h"
#include "util.h"

class GuiSiv3D
{
public:
	enum EGUICheckBox
	{
		SHOW_SCORE,			// "]Ώl" 
		SHOW_TE,			// "w΅θ"
		SHOW_TAG,			// "^O"
		SMALL_NODE,			// "¬³’m[h"
		SHOW_ARROW,			// "ξσ"
		SHOW_SCOREGRAPH,	// "]ΏlOt"
		FIX_SELECTED_NODE,	// "Iπm[hΜΕθ"
		FIX_UPDATED_NODE,	// "]Ώm[hΜΕθ"
		REVERSE_BOARD,		// "ΥΚ½]"
		SOUND,				// "TEh"
		SHOW_DEBUG,			// "fobO"
	};

	GuiSiv3D()
	{
		mFont = Font(8, L"CI");

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
		mFile.setTitle(L"t@C");
		mFile.addln(L"kifu_load", GUIButton::Create(L"t@CπJ­", widgetStyle2));
		mFile.addln(L"kifu_save", GUIButton::Create(L"t@CπΫΆ", widgetStyle2));
		mFile.addln(L"kif_format_load", GUIButton::Create(L"kift@CπJ­", widgetStyle2));

		mEvaluator = GUI(style3);
		mEvaluator.setTitle(L"]Ώ\tg");
		mEvaluator.setPos(132, 0);
		mEvaluator.add(L"evaluator_load", GUIButton::Create(L"]Ώ\tgπJ­", widgetStyle2));
		mEvaluator.addln(L"evaluator_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"option_load", GUIButton::Create(L"IvVπJ­", widgetStyle2));
		mEvaluator.add(L"option_name", GUIText::Create(L"", widgetStyle));
		mEvaluator.add(L"time_prefix", GUIText::Create(L"@@vlΤ", widgetStyle));
		mEvaluator.add(L"time_sec", GUITextField::Create(DEFAULT_EVALUATION_SEC, widgetStyle2));
		mEvaluator.addln(L"time_suffix", GUIText::Create(L"b", widgetStyle));
		mEvaluator.textField(L"time_sec").setText(to_wstring(DEFAULT_EVALUATION_SEC));
		mEvaluator.add(L"folder_analysis", GUIButton::Create(L"tH_ΰA±πΝ", widgetStyle2));
		mEvaluator.add(L"folder_analysis_waiting_num", GUIText::Create(L"0", widgetStyle));

		const int syogibanY = 100;

		mScore = GUI(style);
		mScore.setTitle(L"]Ώl");
		mScore.setPos(SHOGIBAN_W, syogibanY);
		mScore.addln(L"score", GUIText::Create(L"", widgetStyle));
		mScore.addln(L"tejunJap", GUIText::Create(L"", widgetStyle));

		mSettings = GUI(style2);
		mSettings.setTitle(L"έθ");
		mSettings.setPos(WINDOW_W - 114, 0);
		mSettings.add(L"settings",
			GUICheckBox::Create({ L"]Ώl", L"w΅θ", L"^O", L"¬³’m[h", L"ξσ", L"]ΏlOt", L"Iπm[hΜΕθ", L"]Ώm[hΜΕθ", L"ΥΚ½]", L"TEh", L"fobO" },
				{ SHOW_SCORE, SHOW_TE, SHOW_TAG, FIX_SELECTED_NODE, SHOW_ARROW, SHOW_SCOREGRAPH, SOUND }, // ftHgΕONΜΰΜ
				true, widgetStyle2));

		mDelete = GUI(style4);
		mDelete.setPos(WINDOW_W - 114, 234);
		mDelete.setTitle(L"ν [Del]L[");
		mDelete.addln(L"delete_score", GUIButton::Create(L"]Ώl   PΒ", false, widgetStyle2));
		mDelete.addln(L"delete_all_score", GUIButton::Create(L"]Ώl   SΔ", false, widgetStyle2));
		mDelete.addln(L"delete_all_node", GUIButton::Create(L"ΗΚ   SΔ", false, widgetStyle2));


		GUIStyle style5 = style;
		style5.background.color = Color(255, 255, 255, 0);
		mBoard = GUI(style5);
		mBoard.setPos(0, syogibanY);
		mBoard.addln(L"summary", GUITextField::Create(33));
		mBoard.setTitle(L"«ϋΥ");
	}

	GUI mFile;
	GUI mEvaluator;
	GUI mScore;
	GUI mSettings;
	GUI mDelete;
	GUI mBoard;

	Font mFont;
};