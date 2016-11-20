#include "tree_siv3d.h"
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
	mTextureBackground.draw(0, 0);

	// デバッグ用座標と拡大縮小情報
	if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
	{
		mFont(L"Offset=(", mOffsetX, L",", mOffsetY, L") GridScale=", mGridScale).draw(WINDOW_W / 2 + 20.f, 20.0f, Palette::Orange);
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

			Line(stX, stY, deX, deY).draw(5, Color(255, 255, 255, 128));

			if (mGuiSettings.checkBox(L"settings").checked(SHOW_TE))
			{
				const float centerX = 0.5f * (stX + deX);
				const float centerY = 0.5f * (stY + deY);
				mFont(link.teJap).drawCenter(centerX, centerY);
			}
		}
	}

	// 木のノード
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


		if (mGuiSettings.checkBox(L"settings").checked(SHOW_DEBUG))
		{
			// デバッグ用：表示場所の表示
			mFont(centerX, L",", centerY).draw(centerX, centerY, Palette::Orange);
		}
		
		// 評価値の表示
		if (mGuiSettings.checkBox(L"settings").checked(SHOW_SCORE))
		{
			if (node.IsScoreEvaluated())
			{
				//			DrawScoreBar(node.mScore, 2000, centerX, centerY, 80, 10);

				string s = node.ConverScoreToString();
				wstring ws(s.begin(), s.end());

				if (node.mScore > 0)
				{
					mFontScore(ws).drawCenter(centerX, centerY, Palette::Red);
				}
				else if (node.mScore < 0)
				{
					mFontScore(ws).drawCenter(centerX, centerY, Palette::Blue);
				}
				else
				{
					mFontScore(ws).drawCenter(centerX, centerY, Palette::Purple);
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
			mGuiScore.text(L"score").text = Format(L"評価値 ", node.ConverScoreToWString());
			mGuiScore.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 1);
		}
		else
		{
			mGuiScore.text(L"score").text = Format(L"未評価");
			mGuiScore.text(L"tejunJap").text = L"";
		}

//		// 評価値バー
//		if (node.IsScoreEvaluated())
//		{
//			DrawScoreBar(node.mScore, 2000, 400, 200, 400, 50);
//		}
	}

	Tree::Draw();
}

// ノードの表示に使う図形を返す
s3d::RoundRect TreeSiv3D::GetNodeShape(float centerX, float centerY)
{
	return s3d::RoundRect(centerX - 25, centerY - mNodeRadius, 50, mNodeRadius * 2, mNodeRadius);
}

// 現在選択中のノードが変更した瞬間に呼ばれる関数
void TreeSiv3D::OnSelectedNodeIDChanged()
{
	Node& node = mNodes[GetSelectedNodeID()];
	mGuiNode.textField(L"summary").setText(node.mSummary);
//	mGuiNode.textArea(L"comment").setText(node.mComment);

	if (mGuiSettings.checkBox(L"settings").checked(FIX_SELECTED_NODE))
	{
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);
		mOffsetX = RIGHT_CENTER_X - node.mVisualX * mGridScale;
		mOffsetY = RIGHT_CENTER_Y - node.mVisualY * mGridScale;
	}
}

// メインループ
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
			Window::SetTitle(path.value().str());

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
			Window::SetTitle(path.value().str());
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
				if (centerX >= WINDOW_W / 2) // 画面の左側にノードがあるときに、将棋盤とかぶって、誤クリックしないようにする。
				{
					if (GetNodeShape(centerX, centerY).contains(Mouse::Pos()))
					{
						SetSelectedNodeID(nodeID);

						if (mNodeSelectSound)
						{
							if (mNodeSelectSound.isPlaying())
							{
								// 停止して曲の先頭に戻る
								mNodeSelectSound.stop();
							}
							mNodeSelectSound.play();
						}

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

//		if (mGuiNode.textArea(L"comment").hasChanged)
//		{
//			String tmp = mGuiNode.textArea(L"comment").text;
//			node.mComment = tmp.str();
//		}
//
//		if (mGuiNode.textArea(L"comment").active)
//		{
//			const Point delta(24, 56); // ウィンドウからコメント枠への相対座標
//			// TODO ここは、ちゃんと相対座標を決める手段がないので、無理。
//			IME::SetCompositionWindowPos(mGuiNode.getPos() + delta);
//		}
	}
}

// 評価ソフトを開く
void Evaluator::Open()
{
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
}

// 評価を終了する
void Evaluator::Close()
{
	if (mServer != nullptr)
	{
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
		writeStr = "go btime 0 wtime 0 byoyomi " + to_string(mDurationMilliSec + mDurationMilliSecMargin) + "\n";
		return mServer->write(writeStr);
	}

	return false;
}

// 評価ソフトから、評価値と最善手を得る
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
				assert(k + 2 < SZ(tmp));
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
	}
}

// 標準入力から読み込んで、使わずに捨てる
void Evaluator::WaitAndCancel()
{
	string readStr;
	if (mServer->read(readStr))
	{
	}
}

// 評価中のときに、キャンセルして評価をやめる。
void Evaluator::RequestCancel()
{
	// もし評価中であれば、評価をキャンセル
	if (mEStateEvaluation == EStateEvaluation_WaitingScore)
	{
		mEStateEvaluation = EStateEvaluation_WaitingCancel;
	}
}
