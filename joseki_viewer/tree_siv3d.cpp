#include "tree_siv3d.h"
#include "util.h"

#include <algorithm>
#include <numeric>


void TreeSiv3D::Draw()
{
	Tree::Draw();

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

			Line(stX, stY, deX, deY).draw();

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

		Color color = Palette::White;
		if (nodeID == mSelectedNodeID)
		{
			color = Palette::Yellow;
		}
		else if (SQSUM(Mouse::Pos().x - centerX, Mouse::Pos().y - centerY) < SQ(mNodeRadius))
		{
			color = Palette::Orange;
		}

		Circle(centerX, centerY, mNodeRadius).draw(color);
		mFont(node.mScore).drawCenter(centerX, centerY, Palette::Red);
	}


}

void TreeSiv3D::Update()
{
	Tree::Update();
	mEvaluator.Update();

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
		mGridScale *= (float)pow(1.1, -wheelY);
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
						std::wstring wsTmp(readStr.begin(), readStr.end());
						Print(wsTmp);
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
					mTimerMilliSec.restart();
				}
			}
		}
		break;

		case EStateEvaluation_WaitingScore:
			if (mTimerMilliSec.elapsed() > mDurationMilliSec) // 時間経過したら受け取りに
			{
				ReceiveBestMoveAndScore();
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

	{
		std::wstring wsTmp(writeStr.begin(), writeStr.end());
		Print(wsTmp);
	}

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
		int score = 0;

		vector <string> vs;
		Split1(readStr, vs, '\n');

		for (int i = SZ(vs) - 1; i >= 0; --i)
		{
			const string& lastInfo = vs[i];
			vector <string> tmp;
			Split1(lastInfo, tmp, ' ');

			for (int k = 0; k < SZ(tmp); ++k)
			{
				// TODO : ここのフォーマット、詰みの時は違う！
				if (tmp[k] == "score")
				{
					assert(mEvaludatingNodeID != NG);
					score = stoi(tmp[k + 2]);
					std::wstring wsTmp(lastInfo.begin(), lastInfo.end());
					Print(wsTmp);
					goto NUKE;
				}
			}
		
		}

		NUKE:;
		mTree->SetScore(mEvaludatingNodeID, score);
	}
}