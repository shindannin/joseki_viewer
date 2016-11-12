#include "tree_siv3d.h"
#include "util.h"

#include <algorithm>
#include <numeric>

#define DEBUG_DRAW_INFO 

void TreeSiv3D::DrawScoreBar(int score, int maxScore, float cx, float cy, float w, float h)
{
	const float x = cx - w * 0.5f;
	const float y = cy - h * 0.5f;
	float blueRatio = 0.5f - 0.5f * score / maxScore;
	
	if (blueRatio < 0.f)
	{
		blueRatio = 0.f;
	}
	else if (blueRatio > 1.0f)
	{
		blueRatio = 1.f;
	}

	Rect(x, y, w*blueRatio, h).draw(Palette::Blue);
	Rect(x + w*blueRatio, y, w*(1.0f-blueRatio), h).draw(Palette::Red);
}


void TreeSiv3D::Draw()
{
	Tree::Draw();

#ifdef DEBUG_DRAW_INFO
	if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
	{
		// デバッグ用 
		mFont(L"Offset=(", mOffsetX, L",", mOffsetY, L") GridScale=", mGridScale).draw(WINDOW_W / 2 + 20.f, 20.0f, Palette::Orange);
	}
#endif

	// 読んだ評価ソフト・読んだ手数・読んだ時間の表示
	if (mEvaluator.IsActive())
	{
		wstring wname(mEvaluator.GetName().begin(), mEvaluator.GetName().end());
		mGuiEvaluator.text(L"evaluator_name").text = Format(L"評価ソフト=", wname, L" 読んだ手数=", mEvaluator.GetPonderNodes(), L" 読んだ時間=", mEvaluator.GetPonderTime());
//		mFont(L"評価ソフト=", wname, L" 読んだ手数=", mEvaluator.GetPonderNodes(), L" 読んだ時間=", mEvaluator.GetPonderTime()).draw(370, 8.0f, Palette::White);
	}

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

	// リンク
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

			Line(stX, stY, deX, deY).draw(5, Color(255, 255, 255, 128));

			if (mGuiSettings.checkBox(L"settings").checked(SHOW_TE))
			{
				const float centerX = 0.5f * (stX + deX);
				const float centerY = 0.5f * (stY + deY);
				mFont(link.teJap).drawCenter(centerX, centerY);
			}
		}
	}

	// ノード
	for (int nodeID = 0; nodeID<SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);

		// ノード背景の表示
		Color color = Palette::White;
		const s3d::RoundRect& roundRect = GetNodeShape(centerX, centerY);
		if (nodeID == GetSelectedNodeID())
		{
			color = Palette::Yellow;
		}
		else if (roundRect.contains(Mouse::Pos()))
		{
			color = Palette::Orange;
		}
		roundRect.draw(color);


#ifdef DEBUG_DRAW_INFO
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
		{
			// デバッグ用：表示場所の表示
			mFont(centerX, L",", centerY).draw(centerX, centerY, Palette::Orange);
		}
#endif // DEBUG_DRAW_INFO
		
		// 評価値の表示
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_SCORE))
		{
			if (node.IsScoreEvaluated())
			{
				//			DrawScoreBar(node.mScore, 2000, centerX, centerY, 80, 10);

				if (node.mScore > 0)
				{
					mFontScore(node.mScore).drawCenter(centerX, centerY, Palette::Red);
				}
				else if (node.mScore < 0)
				{
					mFontScore(-node.mScore).drawCenter(centerX, centerY, Palette::Blue);
				}
				else
				{
					mFontScore(node.mScore).drawCenter(centerX, centerY, Palette::Purple);
				}
			}
		}

		// タグの表示
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_TAG))
		{
			if (!node.mSummary.empty())
			{
				Rect rect = mFont(node.mSummary).region(centerX, centerY - 22);

				rect.pos.x -= rect.size.x / 2;
				rect.pos.y -= rect.size.y / 2;
				rect.draw(Color(0, 255, 0, 128));
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
			mGuiScore.text(L"score").text = Format(L"評価値 ", node.mScore);
			mGuiScore.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 1);

// 			mFont(node.mScore).draw(600, 100);
// 			mFont(AddNewLine(node.mTejunJap, 1)).draw(600, 200);

		}
		else
		{
			mGuiScore.text(L"score").text = Format(L"未評価");
			mGuiScore.text(L"tejunJap").text = L"";
			// 			mFont(L"未評価").draw(600, 100);
		}
	}
}

s3d::RoundRect TreeSiv3D::GetNodeShape(float centerX, float centerY)
{
	return s3d::RoundRect(centerX - 20, centerY - mNodeRadius, 40, mNodeRadius * 2, mNodeRadius);
}

void TreeSiv3D::OnSelectedNodeIDChanged()
{
	Node& node = mNodes[GetSelectedNodeID()];
	mGuiNode.textField(L"summary").setText(node.mSummary);
	mGuiNode.textArea(L"comment").setText(node.mComment);

	if (mGuiSettings.checkBox(L"settings").checked(FIX_SELECTED_NODE))
	{
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);
		mOffsetX = RIGHT_CENTER_X - node.mVisualX * mGridScale;
		mOffsetY = RIGHT_CENTER_Y - node.mVisualY * mGridScale;
	}
}

void TreeSiv3D::Update()
{
	Tree::Update();
	mEvaluator.Update();

	// メニュー
	if (mGuiFile.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Load(path.value().str());
			InitializeAfterLoad();
			CalculateVisualPos();
		}
	}
	else if (mGuiFile.button(L"kifu_save").pushed)
	{
		const auto path = Dialog::GetSave({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Save(path.value().str());
		}
	}

	if (mGuiEvaluator.button(L"evaluator_load").pushed)
	{
		mEvaluator.Open();
	}
	else if (mGuiEvaluator.button(L"option_load").pushed)
	{
		mEvaluator.OpenOption();
	}

	{
		// 中心座標の変更（前フレームからのカーソルの移動量）
		if (Input::MouseR.pressed)
		{
			const Point delta = Mouse::Delta();
			mOffsetX += delta.x;
			mOffsetY += delta.y;
		}
		else if (Input::MouseL.clicked)
		{
			// ノードの選択
			for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
			{
				const Node& node = mNodes[nodeID];
				const float centerX = ScaleX(node.mVisualX);
				const float centerY = ScaleY(node.mVisualY);
				if (GetNodeShape(centerX, centerY).contains(Mouse::Pos()))
				{
					SetSelectedNodeID(nodeID);
					break;
				}
			}
		}

		// 拡大縮小
		const int wheelY = Mouse::Wheel();
		if (wheelY != 0)
		{
			const float prevGridScale = mGridScale;
			const float invScaledRightCenterX = InvScaleX(RIGHT_CENTER_X);
			const float invScaledRightCenterY = InvScaleY(RIGHT_CENTER_Y);
			mGridScale *= (float)pow(1.1, -wheelY);
			const float diffGridScale = mGridScale - prevGridScale;

			mOffsetX -= invScaledRightCenterX * diffGridScale;
			mOffsetY -= invScaledRightCenterY * diffGridScale;
		}

		// ノードの削除
		if (Input::KeyDelete.clicked)
		{
			mEvaluator.RequestCancel();
			DeleteSelectedNode();
		}
	}

	// ノード情報
	{
		Node& node = mNodes[GetSelectedNodeID()];

		Graphics::SetBackground(Color(160, 200, 100));

		if (mGuiNode.textField(L"summary").hasChanged)
		{
			String tmp = mGuiNode.textField(L"summary").text;
			node.mSummary = tmp.str();
		}

		if (mGuiNode.textField(L"summary").active)
		{
			const Point delta(24, 16); // ウィンドウからサマリ枠への相対座標
			const Rect rect = mFontGuiDefault(node.mSummary).region(mGuiNode.getPos() + delta);
			IME::SetCompositionWindowPos(Point(rect.x + rect.w, rect.y));
		}

		if (mGuiNode.textArea(L"comment").hasChanged)
		{
			String tmp = mGuiNode.textArea(L"comment").text;
			node.mComment = tmp.str();
		}

		if (mGuiNode.textArea(L"comment").active)
		{
			const Point delta(24, 56); // ウィンドウからコメント枠への相対座標
			// TODO ここは、ちゃんと相対座標を決める手段がないので、無理。
			IME::SetCompositionWindowPos(mGuiNode.getPos() + delta);
		}



		
	}
}

void Evaluator::Open()
{
	///////////////////////////////////
	//
	// クライアントを起動
	//
	const auto path = Dialog::GetOpen({ { L"実行ファイル (*.exe)", L"*.exe" } });

	if (path)
	{
		if (mServer != nullptr)
		{
			Close();
		}
		mServer = new Server(path.value(), false);
	}

	if (mServer)
	{
		if (mServer->write("usi\n"))
		{
			std::string readStr;

			System::Sleep(mDurationMilliSecStartMargin);

			if (mServer->read(readStr))
			{
				// 
				vector <string> vs;
				Split1(readStr, vs, '\n');

				for (const string& s : vs)
				{
					vector <string> tmp;
					Split1(s, tmp, ' ');
					Trim(tmp.back());

					if (SZ(tmp)==3 && tmp[0] == "id" && tmp[1] == "name")
					{
						mName = tmp[2];
						break;
					}
				}

				vector <string> options = mOptions;
				options.push_back("isready\n");
				string allOptions = accumulate(options.begin(), options.end(), string());
				if (mServer->write(allOptions))
				{
					System::Sleep(mDurationMilliSecStartMargin);
					if (mServer->read(readStr))
					{
// 						std::wstring wsTmp(readStr.begin(), readStr.end());
// 						Print(wsTmp);
						if (mServer->write("usinewgame\r\n"))
						{
						}
					}
				}
			}
		}
	}
}

void Evaluator::OpenOption()
{
	///////////////////////////////////
	//
	// クライアントを起動
	//
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


}

void Evaluator::Close()
{
	if (mServer != nullptr)
	{
		mServer->write("quit\n");

		delete mServer;
		mServer = nullptr;
	}
}

void Evaluator::Update()
{
	if (mServer)
	{
		// 毎フレーム呼ぶ必要はない！

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
				}
			}
		}
		break;

		case EStateEvaluation_WaitingScore:
			if (mStopwatch.ms() > mDurationMilliSec) // 時間経過したら受け取りに
			{
				ReceiveBestMoveAndScore();
				mEStateEvaluation = EStateEvaluation_FindingNode;
			}
			break;

		case EStateEvaluation_WaitingCancel:
			if (mStopwatch.ms() > mDurationMilliSec) // 時間経過したら受け取りに
			{
				WaitAndCancel();
				mEStateEvaluation = EStateEvaluation_FindingNode;
			}
			break;

		default:
			assert(0);
			break;
		}
	}
}

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
		writeStr = "go btime 0 wtime 0 byoyomi " + to_string(mDurationMilliSec + mDurationMilliSecMargin) + "\n";
		return mServer->write(writeStr);
	}

	return false;
}

void Evaluator::ReceiveBestMoveAndScore()
{
	string readStr;
	if (mServer->read(readStr))
	{
		assert(mEvaludatingNodeID != NG);


		vector <string> vs;
		Split1(readStr, vs, '\n');

		for (int i = SZ(vs) - 1; i >= 0; --i)
		{
			const string& lastInfo = vs[i];
// 			std::wstring wsTmp(lastInfo.begin(), lastInfo.end());
// 			Print(wsTmp);

			vector <string> tmp;
			Split1(lastInfo, tmp, ' ');
			Trim(tmp.back());

			bool isScoreFound = false;
			int score = 0;
			string tejun;

			for (int k = 0; k < SZ(tmp); ++k)
			{
				// TODO : ここのフォーマット、詰みの時は違う！
				if (tmp[k] == "score")
				{
					score = stoi(tmp[k + 2]);
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
	}
}

void Evaluator::WaitAndCancel()
{
	string readStr;
	if (mServer->read(readStr))
	{
	}
}

void Evaluator::RequestCancel()
{
	// もし評価中であれば、評価をキャンセル
	if (mEStateEvaluation == EStateEvaluation_WaitingScore)
	{
		mEStateEvaluation = EStateEvaluation_WaitingCancel;
	}
}
