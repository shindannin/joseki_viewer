#include "tree_siv3d.h"
#include "util.h"

#include <algorithm>
#include <numeric>


void TreeSiv3D::Draw()
{
	Tree::Draw();

	// �����N
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

	// �m�[�h
	for (int nodeID = 0; nodeID<SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		const float centerX = ScaleX(node.mVisualX);
		const float centerY = ScaleY(node.mVisualY);

		Color color = Palette::White;
		if (nodeID == GetSelectedNodeID())
		{
			color = Palette::Yellow;
		}
		else if (SQSUM(Mouse::Pos().x - centerX, Mouse::Pos().y - centerY) < SQ(mNodeRadius))
		{
			color = Palette::Orange;
		}

		Circle(centerX, centerY, mNodeRadius).draw(color);


		mFont(node.mScore).drawCenter(centerX, centerY, Palette::Red);
//		mFont(node.mTejunJap).drawCenter(centerX, centerY+15, Palette::Aqua);
	}
}

void TreeSiv3D::OnSelectedNodeIDChanged()
{
	Node& node = mNodes[GetSelectedNodeID()];
	mGuiNode.textField(L"comment").setText(node.mComment);
}

void TreeSiv3D::Update()
{
	Tree::Update();
	mEvaluator.Update();

	// ���j���[
	if (mGui.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"��Ճr���[���t�@�C�� (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Load(path.value().str());
			InitializeAfterLoad();
			CalculateVisualPos();
		}
	}
	else if (mGui.button(L"kifu_save").pushed)
	{
		const auto path = Dialog::GetSave({ { L"��Ճr���[���t�@�C�� (*.jsv)", L"*.jsv" } });
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
		// ���S���W�̕ύX�i�O�t���[������̃J�[�\���̈ړ��ʁj
		if (Input::MouseR.pressed)
		{
			const Point delta = Mouse::Delta();
			mOffsetX += delta.x;
			mOffsetY += delta.y;
		}
		else if (Input::MouseL.clicked)
		{
			// �m�[�h�̑I��
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

		// �g��k��
		const int wheelY = Mouse::Wheel();
		if (wheelY != 0)
		{
			const float prevGridScale = mGridScale;
			const float invScaledX = InvScaleX(WINDOW_W*0.75f);
			const float invScaledY = InvScaleY(WINDOW_H*0.5f);
			mGridScale *= (float)pow(1.1, -wheelY);
			const float diffGridScale = mGridScale - prevGridScale;

			mOffsetX -= invScaledX * diffGridScale;
			mOffsetY -= invScaledY * diffGridScale;
		}
	}

	// �m�[�h���
	{
		Node& node = mNodes[GetSelectedNodeID()];


		// �R�����g
		// ���͂��������� nodeComment�ɃZ�[�u
		// �����l��nodeComment���烍�[�h


//		mGuiNode.textField(L"comment").setText(node.mComment);
		if (mGuiNode.textField(L"comment").hasChanged)
		{
			String tmp = mGuiNode.textField(L"comment").text;
			node.mComment = tmp.str();
		}


		if (node.IsScoreEvaluated())
		{
			mGuiNode.text(L"score").text = Format(L"�]���l", node.mScore);
			mGuiNode.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 6);
		}
		else
		{
			mGuiNode.text(L"score").text = Format(L"���]��");
			mGuiNode.text(L"tejunJap").text = L"";
		}
	}
}

void Evaluator::Open()
{
	///////////////////////////////////
	//
	// �N���C�A���g���N��
	//
	const auto path = Dialog::GetOpen({ { L"���s�t�@�C�� (*.exe)", L"*.exe" } });

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
		// ���t���[���ĂԕK�v�͂Ȃ��I

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
			if (mTimerMilliSec.elapsed() > mDurationMilliSec) // ���Ԍo�߂�����󂯎���
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
				// TODO : �����̃t�H�[�}�b�g�A�l�݂̎��͈Ⴄ�I
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