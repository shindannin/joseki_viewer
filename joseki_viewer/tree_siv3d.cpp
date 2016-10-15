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
	// デバッグ用 
	mFont(L"Offset=(", mOffsetX, L",", mOffsetY, L") GridScale=", mGridScale ).draw(WINDOW_W/2+20.f, 20.0f, Palette::Orange);
#endif

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

			const float centerX = 0.5f * (stX + deX);
			const float centerY = 0.5f * (stY + deY);
			mFont(link.teJap).drawCenter(centerX, centerY);
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
		if (nodeID == GetSelectedNodeID())
		{
			color = Palette::Yellow;
		}
		else if (SQSUM(Mouse::Pos().x - centerX, Mouse::Pos().y - centerY) < SQ(mNodeRadius))
		{
			color = Palette::Orange;
		}
		s3d::RoundRect(centerX-20, centerY-mNodeRadius, 40, mNodeRadius*2, mNodeRadius).draw(color);


#ifdef DEBUG_DRAW_INFO
		// デバッグ用：表示場所の表示
		mFont(centerX,L",",centerY).draw(centerX, centerY, Palette::Orange);
#endif // DEBUG_DRAW_INFO
		


		// 評価値の表示
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

		if (!node.mComment.empty())
		{
			Rect rect = mFont(node.mComment).region(centerX, centerY - 22);

			rect.pos.x -= rect.size.x / 2;
			rect.pos.y -= rect.size.y / 2;
			rect.draw(Color(0,255,0,128));
			rect.drawFrame(0, 2, Color(255, 255, 255, 255));
			mFont(node.mComment).drawCenter(centerX, centerY - 22, Palette::White);
		}
	}
}

void TreeSiv3D::OnSelectedNodeIDChanged()
{
	Node& node = mNodes[GetSelectedNodeID()];
	mGuiNode.textField(L"comment").setText(node.mComment);

	const float centerX = ScaleX(node.mVisualX);
	const float centerY = ScaleY(node.mVisualY);
 	mOffsetX = RIGHT_CENTER_X - node.mVisualX * mGridScale;
 	mOffsetY = RIGHT_CENTER_Y - node.mVisualY * mGridScale;
}

void TreeSiv3D::Update()
{
	Tree::Update();
	mEvaluator.Update();

	// メニュー
	if (mGui.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Load(path.value().str());
			InitializeAfterLoad();
			CalculateVisualPos();
		}
	}
	else if (mGui.button(L"kifu_save").pushed)
	{
		const auto path = Dialog::GetSave({ { L"定跡ビューワファイル (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Save(path.value().str());
		}
	}
	else if (mGui.button(L"evaluator_load").pushed)
	{
		mEvaluator.Open();
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
				if (SQSUM(Mouse::Pos().x - centerX, Mouse::Pos().y - centerY) < SQ(mNodeRadius))
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

		// コメント
		// 入力があったら nodeCommentにセーブ
		// 初期値はnodeCommentからロード


//		mGuiNode.textField(L"comment").setText(node.mComment);


		if (mGuiNode.textField(L"comment").hasChanged)
		{

			String tmp = mGuiNode.textField(L"comment").text;
			node.mComment = tmp.str();
		}


		if (node.IsScoreEvaluated())
		{
			mGuiNode.text(L"score").text = Format(L"評価値", node.mScore);
			mGuiNode.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 6);
		}
		else
		{
			mGuiNode.text(L"score").text = Format(L"未評価");
			mGuiNode.text(L"tejunJap").text = L"";
		}

		
		const Point delta(24, 16); // ウィンドウからコメント枠への相対座標
		const Rect rect = mFontGuiDefault(node.mComment).region(mGuiNode.getPos() + delta);
		IME::SetCompositionWindowPos(Point(rect.x + rect.w, rect.y));
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

			System::Sleep(mDurationMilliSecMargin);
			if (mServer->read(readStr))
			{
				const vector <string> options = {
					"isready\n",
				};

				string allOptions = accumulate(options.begin(), options.end(), string());

				if (mServer->write(allOptions))
				{
					System::Sleep(mDurationMilliSecMargin);
					if (mServer->read(readStr))
					{
// 						std::wstring wsTmp(readStr.begin(), readStr.end());
// 						Print(wsTmp);
						if (mServer->write("usinewgame\n"))
						{
						}
					}
				}
			}
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
