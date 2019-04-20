#include "tree_siv3d.h"
#include "board_siv3d.h"
#include "gui_siv3d.h"
#include "color.h"
#include "util.h"

#include <algorithm>
#include <numeric>


// 表示
void TreeSiv3D::Draw()
{
	mTextureBackground.map(Window::Width(), Window::Height()).draw();

	// 普通の表示（将棋盤の裏になる）
	DrawBeforeBoard();

	// 将棋盤の表示
	Tree::Draw();
	
	// 将棋盤上の表示
	DrawAfterBoard();
}

void TreeSiv3D::DrawBeforeBoard() const
{
	// デバッグ用座標と拡大縮小情報
	if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_DEBUG))
	{
		mFont(L"Offset=(", mOffsetX, L",", mOffsetY, L") GridScale=", mGridScale).draw(WINDOW_W / 2 + 20.f, 20.0f, Palette::Orange);

		vector <const GUI*> allGUI =
		{
			&mGui.mFile,
			&mGui.mEvaluator,
			&mGui.mScore,
			&mGui.mSettings,
			&mGui.mDelete,
			&mGui.mBoard,
		};


		{
			int i = 0;
			for (const GUI* pGUI : allGUI)
			{
				mFont(L"(", pGUI->getPos().x, L",", pGUI->getPos().y, L")").draw(Window::Width() - 200, Window::Height() - 200 + i * 20, Palette::Orange);
				++i;
			}
		}

		// 通信デバッグログ
		const vector <string>& logs = mEvaluator.GetReadLogs();

		for (int i=0;i<SZ(logs);++i)
		{
			mFont(Widen(logs[i])).draw(800, 400+15*i, Palette::Orange);
		}
	}

	// 読んだ評価ソフト・読んだ手数・読んだ時間の表示
	if (mEvaluator.IsActive())
	{
		wstring wname(mEvaluator.GetName().begin(), mEvaluator.GetName().end());
		mGui.mEvaluator.text(L"evaluator_name").text = Format(L"評価ソフト ", wname, L"  読んだ手数 ", mEvaluator.GetPonderNodes(), L"手  読んだ時間 ", mEvaluator.GetPonderTime(), L"ミリ秒");
	}

	// オプションの設定
	{
		String optionString;
		if (mEvaluator.IsOptionRead())
		{
			optionString = Format(L"設定済み");
		}
		else
		{
			optionString = Format(L"未設定");
		}
		mGui.mEvaluator.text(L"option_name").text = optionString;
	}

	// 木のリンク
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		for (const Link link : node.mLinks)
		{
			const Node& destNode = mNodes[link.destNodeID];

			const float stX = ScaleX(node.mVisualX);
			const float stY = ScaleY(node.mVisualY);
			const float deX = ScaleX(destNode.mVisualX);
			const float deY = ScaleY(destNode.mVisualY);

			Color c;

			if (destNode.IsScoreEvaluated() && IsBestRoute(link.destNodeID))
			{
				c = COLOR_LINK_BEST;
			}
			else
			{
				c = COLOR_LINK_NORMAL;
			
			}

			Line(stX, stY, deX, deY).draw(2, c);

			if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_TE))
			{
				const int centerX = static_cast<int>(deX); // (0.5f * (stX + deX));
				const int centerY = static_cast<int>(0.5f * (stY + deY));
				mFont(link.teJap).drawCenter(centerX, centerY);
			}
		}
	}

	// 木のノード
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		const int centerX = static_cast<int>(ScaleX(node.mVisualX));
		const int centerY = static_cast<int>(ScaleY(node.mVisualY));
		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_DEBUG))
		{
			// デバッグ用：表示場所の表示
			mFont(centerX, L",", centerY).draw(centerX, centerY, Palette::Orange);
		}

		// ノード背景の表示
		Color color = Palette::White;
		const NodeSize nodeSize = mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SMALL_NODE) ? NS_SMALL : NS_BIG;
		const s3d::RoundRect& roundRect = GetNodeShape(centerX, centerY, nodeSize);
		if (nodeID == GetSelectedNodeID())
		{
			color = COLOR_SELECTED_NODE;
		}
		else if (roundRect.contains(Mouse::Pos()))
		{
			color = Palette::Orange; // Color{ 0xde, 0x96, 0x10, 0xff }; 
		}
		else if (mEvaluator.IsNodeEvaluating(nodeID))
		{
			color = Color{ 0xa0, 0xc2, 0x38, 0xff }; // Palette::Lightgreen;
		}

		roundRect.draw(color);

		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_SCORE))
		{
			DrawScore(centerX, centerY, node, nodeSize);
		}
	}

	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);

		// タグの表示
		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_TAG))
		{
			if (!node.mSummary.empty())
			{
				Rect rect = mFont(node.mSummary).region(centerX, centerY - 22);

				rect.pos.x -= rect.size.x / 2;
				rect.pos.y -= rect.size.y / 2;
				rect.draw(Color(0, 128, 0, 192));
				rect.drawFrame(0, 2, Color(255, 255, 255, 255));
				mFont(node.mSummary).drawCenter(centerX, centerY - 22, Palette::White);
			}
		}
	}


	// 手順
	{
		const Node& node = mNodes[GetSelectedNodeID()];
		if (node.IsScoreEvaluated())
		{
			mGui.mScore.text(L"score").text = Format(L"評価値 ", node.ConverScoreToWString());
			mGui.mScore.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 1);
		}
		else
		{
			mGui.mScore.text(L"score").text = Format(L"未評価 -----");
			mGui.mScore.text(L"tejunJap").text = L"";
		}
	}

	// 評価値グラフ
	if(mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_SCOREGRAPH))
	{
		const int offW = SCORE_GRAPH_W;
		const int offH = SCORE_GRAPH_H;

		const int halfH = offH / 2;
		const int offX = SCORE_GRAPH_X;
		const int offY = SCORE_GRAPH_Y;
		const int scaleX = SCORE_GRAPH_SW;
		const int maxScore = 2000;

		int sente_offset = 0;
		int gote_offset = 0;
		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::REVERSE_BOARD))
		{
			gote_offset = halfH;
		}
		else
		{
			sente_offset = halfH;
		}

		Rect(offX, offY + sente_offset, offW, halfH).draw(COLOR_SENTE_DARK);
		Rect(offX, offY + gote_offset, offW, halfH).draw(COLOR_GOTE_DARK);

		Line(offX, offY + halfH, offX + offW, offY + halfH).draw(1, Color(192, 192, 192, 128));

		const vector <int> scores = GetBestRouteScores();
		const vector <bool> branch = GetBestRouteBranch();

		int current_score = 0;

		// 折れ線グラフ
		for (int i = 0; i < SZ(scores) - 1; ++i)
		{
			const float scaleY = (float)halfH / maxScore;

			int next_score = current_score;
			Color c = COLOR_LINK_NORMAL;
			if (Node::IsScoreEvaluated(scores[i + 1]))
			{
				next_score = scores[i + 1];
				c = COLOR_LINK_BEST;
			}
			const int stX = offX + i * scaleX;
			const int stY = offY + halfH + CLAMP(static_cast<int>(current_score * scaleY), -halfH + 1, halfH - 1);
			const int deX = offX + (i + 1) * scaleX;
			const int deY = offY + halfH + CLAMP(static_cast<int>(next_score * scaleY), -halfH + 1, halfH - 1);

			Line(stX, stY, deX, deY).draw(2, c);
			if (branch[i])
			{
				Circle(stX, stY, 2).draw(Palette::White);
			}

			if (i == GetTesu()-1)
			{
				Circle(stX, stY, 3).draw(COLOR_SELECTED_NODE);
			}

			if (Node::IsScoreEvaluated(scores[i + 1]))
			{
				current_score = next_score;
			}
		}

		// 細長選択バー
		if (IsInScoreGraph(Mouse::Pos().x, Mouse::Pos().y))
		{
			const int order = CalcOrder(static_cast<float>(Mouse::Pos().x), static_cast<float>(offX), static_cast<float>(scaleX));
			const int x = offX + order * scaleX;
			Rect( static_cast<int>(x - scaleX * 0.5f), offY, scaleX, offH).draw(Color(255, 255, 0, 64));
		}
	}

	

}

void TreeSiv3D::DrawAfterBoard() const
{
	if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_ARROW))
	{
		BoardSiv3D* boardSiv3D = dynamic_cast<BoardSiv3D*>(mBoard);
		if (boardSiv3D != nullptr)
		{
			struct ScoreOnArrow
			{
				int cy;
				int cx;
				const Node* pDestNode;
			};

			vector <ScoreOnArrow> scoreOnArrows;

			const Node& node = GetSelectedNode();


			// 次の手の表示
			for (const Link& link : node.mLinks)
			{
				const Color grabbedColor[NUM_SEN_GO] =
				{
					{ 255, 0, 0, 127 },
					{ 0, 0, 255, 127 },
				};

				int cy, cx;

				// 矢印表示
				boardSiv3D->DrawMove(link.te, grabbedColor[boardSiv3D->GetTeban()], cy, cx);

				// もし1手先のノードの評価値が分かれば、それを後で表示
				const Node& destNode = GetNode(link.destNodeID);
				if (destNode.IsScoreEvaluated())
				{
					ScoreOnArrow tmp = { cy, cx, &destNode };
					scoreOnArrows.push_back(tmp);
				}
			}

			// 最善手の表示（紫）
			if (node.IsScoreEvaluated())
			{
				int cy, cx;
				const string firstTe = boardSiv3D->GetFirstTeFromTejun(node.mBestTejun);

				// 矢印表示
				if (!firstTe.empty())
				{
					boardSiv3D->DrawMove(firstTe, { 255, 0, 255, 127 }, cy, cx);

					// スコア表示の追加。もし、最善手の次の手が評価されている場合は、そちらのほうが正確なので、足さない
					bool ok = true;
					for ( const auto& a : scoreOnArrows)
					{
						if (a.cy == cy && a.cx == cx)
						{
							ok = false;
							break;
						}
					}

					if (ok)
					{
						ScoreOnArrow tmp = { cy, cx, &node };
						scoreOnArrows.push_back(tmp);
					}
				}
			}

			for (const ScoreOnArrow& tmp : scoreOnArrows)
			{
				// ノード背景の表示
				Color color = Palette::White;
				const s3d::RoundRect& roundRect = GetNodeShape(tmp.cx, tmp.cy, NS_MEDIUM);
				roundRect.draw(color);

				// ノードスコアの表示
				DrawScore(tmp.cx, tmp.cy, *tmp.pDestNode, NS_MEDIUM);
			}
		}
	}
}


// 評価値の表示
void TreeSiv3D::DrawScore(int centerX, int centerY, const Node& node, NodeSize nodeSize) const
{
	if (node.IsScoreEvaluated())
	{
		string s = node.ConverScoreToString();
		wstring ws(s.begin(), s.end());

		const Font* pFont = nullptr;
		switch (nodeSize)
		{
		case NS_BIG:
			pFont = &mFontScore;
			break;
		case NS_MEDIUM:
			pFont = &mFontScoreMedium;
			break;
		case NS_SMALL:
			pFont = &mFontScoreSmall;
			break;
		}

		if (node.IsResign())
		{
			if (node.IsSenteKachi())
			{
				(*pFont)(ws).drawCenter(centerX, centerY, Palette::Red);
			}
			else if (node.IsGoteKachi())
			{
				(*pFont)(ws).drawCenter(centerX, centerY, Palette::Blue);
			}
		}
		else
		{
			if (node.mScore > 0)
			{
				(*pFont)(ws).drawCenter(centerX, centerY, Palette::Red);
			}
			else if (node.mScore < 0)
			{
				(*pFont)(ws).drawCenter(centerX, centerY, Palette::Blue);
			}
			else
			{
				(*pFont)(ws).drawCenter(centerX, centerY, Palette::Purple);
			}
		}
	}
}

// ノードの表示に使う図形を返す
s3d::RoundRect TreeSiv3D::GetNodeShape(int centerX, int centerY, NodeSize nodeSize) const
{
	const static int halfWidth[NUM_NS]=
	{
		25,
		16,
		12,
	};

	const static int radius[NUM_NS]=
	{
		8,
		6,
		4,
	};

	return s3d::RoundRect(centerX - halfWidth[nodeSize], centerY - radius[nodeSize], halfWidth[nodeSize] * 2, radius[nodeSize]* 2, radius[nodeSize]);
}

// 現在選択中のノードが変更した瞬間に呼ばれる関数
void TreeSiv3D::OnSelectedNodeIDChanged()
{
	Node& node = mNodes[GetSelectedNodeID()];
	mGui.mBoard.textField(L"summary").setText(node.mSummary);
//	mGui.mNode.textArea(L"comment").setText(node.mComment);

	if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::FIX_SELECTED_NODE))
	{
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);
		mOffsetX = GetTreeCenterX() - node.mVisualX * mGridScale;
		mOffsetY = GetTreeCenterY() - node.mVisualY * mGridScale;
	}
}

bool TreeSiv3D::IsFixUpdatedNode() const 
{
	return mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::FIX_UPDATED_NODE);
}


// メインループ
void TreeSiv3D::Update()
{
	// TODO メニューはtree3dから分離して、TreeSiv3D->BoardSiv3Dと参照しないように。
	{
		BoardSiv3D* boardSiv3D = dynamic_cast<BoardSiv3D*>(mBoard);
		if (boardSiv3D != nullptr)
		{
			boardSiv3D->SetOffset(mGui.mBoard.getPos().x + 100, mGui.mBoard.getPos().y+80);
		}
	}


	// メニュー
	if (mGui.mFile.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"将棋ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Load(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));

			InitializeAfterLoad();
			CalculateVisualPos();
			mEvaluator.RequestCancel();
		}
	}
	else if (mGui.mFile.button(L"kifu_save").pushed)
	{
		const auto path = Dialog::GetSave({ { L"将棋ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Save(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));
		}
	}
	else if (mGui.mFile.button(L"kif_format_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"kifファイル (*.kif)", L"*.kif" } });
		if (path.has_value())
		{
			LoadKif(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));

			CalculateVisualPos();
			mEvaluator.RequestCancel();
		}
	}
	else if (mGui.mEvaluator.button(L"evaluator_load").pushed)
	{
		mEvaluator.Open();
	}
	else if (mGui.mEvaluator.button(L"option_load").pushed)
	{
		mEvaluator.OpenOption();
	}

	// 時間の更新
	if (mGui.mEvaluator.textField(L"time_sec").hasChanged)
	{
		string text = mGui.mEvaluator.textField(L"time_sec").text.narrow();
		if(!text.empty())
		{
			const int sec = strtol(text.c_str(),NULL,0);
			if (sec > 0)
			{
				mEvaluator.SetDurationSec(sec);
			}
		}
	}


	{

		if (Input::MouseR.clicked && IsInShogiban(Mouse::Pos().x, Mouse::Pos().y))
		{
			// 1手戻る
			BoardSiv3D* boardSiv3D = dynamic_cast<BoardSiv3D*>(mBoard);
			if (boardSiv3D != nullptr && boardSiv3D->IsInputStateIdle())
			{
				const Node& node = mNodes[GetSelectedNodeID()];
				if (!node.IsRoot())
				{
					SetSelectedNodeID(node.GetParentNodeID());
					PlayNodeSelectSound();
				}
			}
		}
		else if (Input::MouseR.pressed)
		{
			// 中心座標の変更（前フレームからのカーソルの移動量）
			const Point delta = Mouse::Delta();
			mOffsetX += delta.x;
			mOffsetY += delta.y;
		}
		else if (Input::MouseL.pressed && IsInScoreGraph(Mouse::Pos().x, Mouse::Pos().y))
		{
			// 評価値グラフ上の選択
			if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_SCOREGRAPH))
			{
				// 現在何番目を選択しているか？
				int order = CalcOrder(static_cast<float>(Mouse::Pos().x), static_cast<float>(SCORE_GRAPH_X), static_cast<float>(SCORE_GRAPH_SW));

				const vector <int>& bestRoundNodeIDs = GetBestRouteNodeIDs();
				const int num = SZ(bestRoundNodeIDs);
				order = CLAMP(order, 0, num - 1);
				SetSelectedNodeID(bestRoundNodeIDs[order]);
				PlayNodeSelectSound();
			}
		}
		else if (Input::MouseL.clicked)
		{
			// ノードの選択
			const NodeSize nodeSize = mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SMALL_NODE) ? NS_SMALL : NS_BIG;
			for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
			{
				const Node& node = mNodes[nodeID];
				const int centerX = static_cast<int>(ScaleX(node.mVisualX));
				const int centerY = static_cast<int>(ScaleY(node.mVisualY));

				if (!IsInShogiban(centerX, centerY))
				{
					if (GetNodeShape(centerX, centerY, nodeSize).contains(Mouse::Pos()))
					{
						SetSelectedNodeID(nodeID);
						PlayNodeSelectSound();
						break;
					}
				}
			}

		}

		// 拡大縮小
		const int wheelY = Mouse::Wheel();
		if (wheelY != 0)
		{
			const float prevGridScale = mGridScale;
			const float invScaledRightCenterX = InvScaleX(GetTreeCenterX());
			const float invScaledRightCenterY = InvScaleY(GetTreeCenterY());
			mGridScale *= (float)pow(1.1, -wheelY);
			const float diffGridScale = mGridScale - prevGridScale;

			mOffsetX -= invScaledRightCenterX * diffGridScale;
			mOffsetY -= invScaledRightCenterY * diffGridScale;
		}

		// 削除
		if (Input::KeyDelete.clicked)
		{
			const bool nextFlip = !mGui.mDelete.button(L"delete_score").enabled;

			mGui.mDelete.button(L"delete_score").enabled = nextFlip;
			mGui.mDelete.button(L"delete_all_score").enabled = nextFlip;
			mGui.mDelete.button(L"delete_all_node").enabled = nextFlip;
		}
		else if (mGui.mDelete.button(L"delete_score").pushed)
		{
			mEvaluator.RequestCancel();
			ResetSelectedScore();
		}
		else if (mGui.mDelete.button(L"delete_all_score").pushed)
		{
			mEvaluator.RequestCancel();
			ResetSelectedAncientScore();
		}
		else if (mGui.mDelete.button(L"delete_all_node").pushed)
		{
			mEvaluator.RequestCancel();
			DeleteSelectedAncientNode();
		}
	}

	// ノード情報
	{
		Node& node = mNodes[GetSelectedNodeID()];

		Graphics::SetBackground(Color(160, 200, 100));

		if (mGui.mBoard.textField(L"summary").hasChanged)
		{
			String tmp = mGui.mBoard.textField(L"summary").text;
			node.mSummary = tmp.str();
		}

		if (mGui.mBoard.textField(L"summary").active)
		{
			const Point delta(20, 26); // ウィンドウからサマリ枠への相対座標
			const Rect rect = mFontGuiDefault(node.mSummary).region(mGui.mBoard.getPos() + delta);
			IME::SetCompositionWindowPos(Point(rect.x + rect.w, rect.y));
		}
	}

	// 親クラスの更新
	Tree::Update();
	mEvaluator.Update();
}

// ノード選択音を鳴らす
void TreeSiv3D::PlayNodeSelectSound()
{
	if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SOUND))
	{
		if (mNodeSelectSound)
		{
			if (mNodeSelectSound.isPlaying())
			{
				// 停止して曲の先頭に戻る
				mNodeSelectSound.stop();
			}
			mNodeSelectSound.play();
		}
	}
}

// 座標が将棋盤の中かどうかをチェック
bool TreeSiv3D::IsInShogiban(int x, int y) const
{
	return	INRANGE(x, mGui.mBoard.getPos().x, mGui.mBoard.getPos().x + SHOGIBAN_W) &&
			INRANGE(y, mGui.mBoard.getPos().y, mGui.mBoard.getPos().y + SHOGIBAN_H);
}

// 座標が評価値グラフの中かどうかチェック
bool TreeSiv3D::IsInScoreGraph(int x, int y) const
{
	return	INRANGE(x, SCORE_GRAPH_X, SCORE_GRAPH_X + SCORE_GRAPH_W) &&
			INRANGE(y, SCORE_GRAPH_Y, SCORE_GRAPH_Y + SCORE_GRAPH_H);
}
