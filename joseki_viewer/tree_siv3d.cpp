#include "tree_siv3d.h"
#include "board_siv3d.h"
#include "util.h"

#include <algorithm>
#include <numeric>

// 評価値バー
//void TreeSiv3D::DrawScoreBar(int score, int maxScore, float cx, float cy, float w, float h)
//{
//	const int x = static_cast<int>(cx - w * 0.5f);
//	const int y = static_cast<int>(cy - h * 0.5f);
//	float blueRatio = 0.5f - 0.5f * score / maxScore;
//	
//	if (blueRatio < 0.f)
//	{
//		blueRatio = 0.f;
//	}
//	else if (blueRatio > 1.0f)
//	{
//		blueRatio = 1.f;
//	}
//
//	Rect(x, y, w*blueRatio, h).draw(Palette::Blue);
//	Rect(x + w*blueRatio, y, w*(1.0f-blueRatio), h).draw(Palette::Red);
//}

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
	if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
	{
		mFont(L"Offset=(", mOffsetX, L",", mOffsetY, L") GridScale=", mGridScale).draw(WINDOW_W / 2 + 20.f, 20.0f, Palette::Orange);

		vector <const GUI*> allGUI =
		{
			&mGuiFile,
			&mGuiEvaluator,
			&mGuiScore,
			&mGuiSettings,
			&mGuiDelete,
			&mGuiBoard,
		};


		int i = 0;
		for (const GUI* pGUI : allGUI)
		{
			mFont(L"(", pGUI->getPos().x, L",", pGUI->getPos().y, L")").draw(Window::Width() - 200, Window::Height() - 200 + i * 20, Palette::Orange);
			++i;
		}
	}

	// 読んだ評価ソフト・読んだ手数・読んだ時間の表示
	if (mEvaluator.IsActive())
	{
		wstring wname(mEvaluator.GetName().begin(), mEvaluator.GetName().end());
		mGuiEvaluator.text(L"evaluator_name").text = Format(L"評価ソフト ", wname, L"  読んだ手数 ", mEvaluator.GetPonderNodes(), L"手  読んだ時間 ", mEvaluator.GetPonderTime(), L"ミリ秒");
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
		mGuiEvaluator.text(L"option_name").text = optionString;
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

			Line(stX, stY, deX, deY).draw(2, Color(192, 192, 192, 128));

			if (mGuiSettings.checkBox(L"settings").checked(SHOW_TE))
			{
				const float centerX = 0.5f * (stX + deX);
				const float centerY = 0.5f * (stY + deY);
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
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
		{
			// デバッグ用：表示場所の表示
			mFont(centerX, L",", centerY).draw(centerX, centerY, Palette::Orange);
		}

		// ノード背景の表示
		Color color = Palette::White;
		const NodeSize nodeSize = mGuiSettings.checkBox(L"settings").checked(SMALL_NODE) ? NS_SMALL : NS_BIG;
		const s3d::RoundRect& roundRect = GetNodeShape(centerX, centerY, nodeSize);
		if (nodeID == GetSelectedNodeID())
		{
			color = Palette::Yellow;
		}
		else if (roundRect.contains(Mouse::Pos()))
		{
			color = Palette::Orange;
		}
		else if (mEvaluator.IsNodeEvaluating(nodeID))
		{
			color = Palette::Lightgreen;
		}
		roundRect.draw(color);

		if (mGuiSettings.checkBox(L"settings").checked(SHOW_SCORE))
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
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_TAG))
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
			mGuiScore.text(L"score").text = Format(L"評価値 ", node.ConverScoreToWString());
			mGuiScore.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 1);
		}
		else
		{
			mGuiScore.text(L"score").text = Format(L"未評価 -----");
			mGuiScore.text(L"tejunJap").text = L"";
		}

		//		// 評価値バー
		//		if (node.IsScoreEvaluated())
		//		{
		//			DrawScoreBar(node.mScore, 2000, 400, 200, 400, 50);
		//		}
	}
}

void TreeSiv3D::DrawAfterBoard() const
{
	if (mGuiSettings.checkBox(L"settings").checked(SHOW_ARROW))
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
			(*pFont)(ws).drawCenter(centerX, centerY, Palette::Black);
		}
		else if (node.mScore > 0)
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

// ノードの表示に使う図形を返す
s3d::RoundRect TreeSiv3D::GetNodeShape(int centerX, int centerY, NodeSize nodeSize) const
{
	const int halfWidth[NUM_NS]=
	{
		25,
		16,
		12,
	};

	const int radius[NUM_NS]=
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
	mGuiBoard.textField(L"summary").setText(node.mSummary);
//	mGuiNode.textArea(L"comment").setText(node.mComment);

	if (mGuiSettings.checkBox(L"settings").checked(FIX_SELECTED_NODE))
	{
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);
		mOffsetX = GetTreeCenterX() - node.mVisualX * mGridScale;
		mOffsetY = GetTreeCenterY() - node.mVisualY * mGridScale;
	}
}

// メインループ
void TreeSiv3D::Update()
{
	// TODO メニューはtree3dから分離して、TreeSiv3D->BoardSiv3Dと参照しないように。
	{
		BoardSiv3D* boardSiv3D = dynamic_cast<BoardSiv3D*>(mBoard);
		if (boardSiv3D != nullptr)
		{
			boardSiv3D->SetOffset(mGuiBoard.getPos().x + 100, mGuiBoard.getPos().y+70);
		}
	}


	// メニュー
	if (mGuiFile.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Load(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));

			InitializeAfterLoad();
			CalculateVisualPos();
			mEvaluator.RequestCancel();
		}
	}
	else if (mGuiFile.button(L"kifu_save").pushed)
	{
		const auto path = Dialog::GetSave({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Save(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));
		}
	}
	else if (mGuiFile.button(L"kif_format_load").pushed)
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
	else if (mGuiEvaluator.button(L"evaluator_load").pushed)
	{
		mEvaluator.Open();
	}
	else if (mGuiEvaluator.button(L"option_load").pushed)
	{
		mEvaluator.OpenOption();
	}

	// 時間の更新
	if (mGuiEvaluator.textField(L"time_sec").hasChanged)
	{
		string text = mGuiEvaluator.textField(L"time_sec").text.narrow();
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
		else if (Input::MouseL.clicked)
		{
			// ノードの選択
			const NodeSize nodeSize = mGuiSettings.checkBox(L"settings").checked(SMALL_NODE) ? NS_SMALL : NS_BIG;
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
			const bool nextFlip = !mGuiDelete.button(L"delete_score").enabled;

			mGuiDelete.button(L"delete_score").enabled = nextFlip;
			mGuiDelete.button(L"delete_all_score").enabled = nextFlip;
			mGuiDelete.button(L"delete_all_node").enabled = nextFlip;
		}
		else if (mGuiDelete.button(L"delete_score").pushed)
		{
			mEvaluator.RequestCancel();
			ResetSelectedScore();
		}
		else if (mGuiDelete.button(L"delete_all_score").pushed)
		{
			mEvaluator.RequestCancel();
			ResetSelectedAncientScore();
		}
		else if (mGuiDelete.button(L"delete_all_node").pushed)
		{
			mEvaluator.RequestCancel();
			DeleteSelectedAncientNode();
		}
	}

	// ノード情報
	{
		Node& node = mNodes[GetSelectedNodeID()];

		Graphics::SetBackground(Color(160, 200, 100));

		if (mGuiBoard.textField(L"summary").hasChanged)
		{
			String tmp = mGuiBoard.textField(L"summary").text;
			node.mSummary = tmp.str();
		}

		if (mGuiBoard.textField(L"summary").active)
		{
			const Point delta(20, 26); // ウィンドウからサマリ枠への相対座標
			const Rect rect = mFontGuiDefault(node.mSummary).region(mGuiBoard.getPos() + delta);
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

// ノード選択音を鳴らす
bool TreeSiv3D::IsInShogiban(int x, int y) const
{
	return	INRANGE(x, mGuiBoard.getPos().x, mGuiBoard.getPos().x + mShogibanWidth) &&
			INRANGE(y, mGuiBoard.getPos().y, mGuiBoard.getPos().y + mShogibanHeight);
}


// 評価ソフトを開く
void Evaluator::Open()
{
	const auto path = Dialog::GetOpen({ { L"実行ファイル (*.exe)", L"*.exe" } });

	if (path)
	{
		OpenSub(path.value());
	}
}

// 評価ソフトの初期設定オプションを開く
void Evaluator::OpenOption()
{
	const auto path = Dialog::GetOpen({ { L"テキストファイル (*.txt)", L"*.txt" } });
	if (path)
	{
		mOptions.clear();

		TextReader reader(path.value());
		String line;
		while (reader.readLine(line))
		{
			mOptions.push_back("setoption " + line.narrow()+"\n");
		}
	}

	OpenSub(mEvaluatorPath);
}

void Evaluator::OpenSub(const FilePath& newEvaluatorPath)
{
	// 現在、ノードを評価中であれば読み捨て
	RequestCancel();

	// 現在の評価ソフトがあれば、同じものを選んでいたとしても、閉じる
	if (mServer != nullptr)
	{
		Close();
	}

	if (newEvaluatorPath != L"")
	{
		mEvaluatorPath = newEvaluatorPath;

		mServer = new Server(mEvaluatorPath, false);

		if (mServer->write("usi\n"))
		{
			// usiokを受け取るまで待つ。
			while (!IsUSIOKReceived())
			{
				UpdateReadFromStdio();
			}

			// 評価ソフト名を受け取る
			for (const string& s : mReadLogs)
			{
				vector <string> tmp;
				Split1(s, tmp, ' ');
				Trim(tmp.back());

				if (SZ(tmp) == 3 && tmp[0] == "id" && tmp[1] == "name")
				{
					mName = tmp[2];
					break;
				}
			}
			mReadLogs.clear();

			// オプションを送る
			vector <string> options = mOptions;
			options.push_back("isready\n");
			string allOptions = accumulate(options.begin(), options.end(), string());
			if (mServer->write(allOptions))
			{
				// usiokを受け取るまで待つ。
				while (!IsReadyOKReceived())
				{
					UpdateReadFromStdio();
				}
				mReadLogs.clear();

				// スタート！
				if (mServer->write("usinewgame\n"))
				{
				}
				else
				{
					assert(0);
				}
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			assert(0);
		}
	}
}

// 評価を終了する
void Evaluator::Close()
{
	if (mServer != nullptr)
	{
		// この時点で読み捨ては終了してるはず。
		mServer->write("quit\n");

		delete mServer;
		mServer = nullptr;
	}
}

// メインループ
void Evaluator::Update()
{
	if (mServer)
	{

		// 評価の状態遷移
		switch (mEStateEvaluation)
		{
		case EStateEvaluation_FindingNode:
		{
			mEvaludatingNodeID = mTree->GetNextEvaludatedNodeID();
			if (mEvaludatingNodeID != NG)
			{
				if (Go())
				{
					mEStateEvaluation = EStateEvaluation_WaitingScore;
					mStopwatch.restart();
					mPollingStopwatch.restart();
				}
			}
		}
		break;

		case EStateEvaluation_WaitingScore:
			if (UpdateReadFromStdio())
			{
				if (ReceiveBestMoveAndScore())
				{
					mEStateEvaluation = EStateEvaluation_FindingNode;
				}
			}
			break;

		default:
			assert(0);
			break;
		}
	}
}

// 評価を開始する
bool Evaluator::Go()
{
	assert(mEvaludatingNodeID != NG);
	const Node& node = mTree->GetNode(mEvaludatingNodeID);

	string writeStr;
	writeStr += "position sfen " + node.mState + "\n";

// 	{
// 		std::wstring wsTmp(writeStr.begin(), writeStr.end());
// 		Print(wsTmp);
// 	}

	assert(mServer != nullptr);

	if (mServer != nullptr && mServer->write(writeStr))
	{
		writeStr = "go btime 0 wtime 0 byoyomi " + to_string(mDurationMilliSec) + "\n";
		return mServer->write(writeStr);
	}

	return false;
}

void Evaluator::RequestCancel()
{
	if (mEStateEvaluation == EStateEvaluation_WaitingScore)
	{
		if (mServer)
		{
			if (mServer->write("stop\n"))
			{
				while (!IsBestMoveReceived())
				{
					UpdateReadFromStdio();
				}
			}
		}
	}

	mEStateEvaluation = EStateEvaluation_FindingNode;
	mReadLogs.clear();
}

// 定期的に、評価ソフトから、標準入力経由で、何かを受け取ったか確認する。
// 更新があったらtrueを返す。
bool Evaluator::UpdateReadFromStdio()
{
	if (mPollingStopwatch.ms() > mDurationMilliSecPolling) // 時間経過したら受け取りに
	{
		string readStr;
		if (mServer->read(readStr))
		{
			vector <string> vs;
			Split1(readStr, vs, '\n');

			for(const string& s : vs)
			{
				mReadLogs.push_back(s);
			}
			return true;
		}

		mPollingStopwatch.restart();
	}

	return false;
}


// 評価ソフトから、評価値と最善手を得る
bool Evaluator::ReceiveBestMoveAndScore()
{
	assert(mEvaludatingNodeID != NG);

	if (!IsBestMoveReceived())
	{
		return false;
	}

	bool isScoreFound = false;

	for (int i = SZ(mReadLogs) - 2; i >= 0; --i) // 最後の行はbestmoveなので、-2からチェックで正しい
	{
		const string& lastInfo = mReadLogs[i];
// 		std::wstring wsTmp(lastInfo.begin(), lastInfo.end());
// 		Print(wsTmp);

		vector <string> tmp;
		Split1(lastInfo, tmp, ' ');
		Trim(tmp.back());

		int score = 0;
		string tejun;

		assert(tmp[0]=="info");

		for (int k = 0; k < SZ(tmp); ++k)
		{
			if (tmp[k] == "score" && k + 2 < SZ(tmp))
			{
				if (tmp[k + 1] == "cp")
				{
					score = stoi(tmp[k + 2]);
				}
				else if (tmp[k + 1] == "mate")
				{
					const int mate = stoi(tmp[k + 2]);
					score = Node::ConvertMateToScore(mate);
				}
				isScoreFound = true;
			}
			else if (tmp[k] == "pv")
			{
				for (int x = k+1; x < SZ(tmp); ++x)
				{
					tejun += tmp[x];
					if (x < SZ(tmp) - 1)
					{
						tejun += " ";
					}
				}
			}
			else if (tmp[k] == "nodes")
			{
				mPonderNodes = stoll(tmp[k + 1]);
			}
			else if (tmp[k] == "time")
			{
				mPonderTime = stoll(tmp[k + 1]);
			}
		}

		if (isScoreFound)
		{
			mTree->UpdateNode(mEvaludatingNodeID, score, tejun);
			break;
		}
	}

	assert(isScoreFound);

	mReadLogs.clear();
	return true;
}
