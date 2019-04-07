#include "tree_siv3d.h"
#include "board_siv3d.h"
#include "gui_siv3d.h"
#include "util.h"

#include <algorithm>
#include <numeric>

// �]���l�o�[
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

// �\��
void TreeSiv3D::Draw()
{
	mTextureBackground.map(Window::Width(), Window::Height()).draw();

	// ���ʂ̕\���i�����Ղ̗��ɂȂ�j
	DrawBeforeBoard();

	// �����Ղ̕\��
	Tree::Draw();
	
	// �����Տ�̕\��
	DrawAfterBoard();
}

void TreeSiv3D::DrawBeforeBoard() const
{
	// �f�o�b�O�p���W�Ɗg��k�����
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


		int i = 0;
		for (const GUI* pGUI : allGUI)
		{
			mFont(L"(", pGUI->getPos().x, L",", pGUI->getPos().y, L")").draw(Window::Width() - 200, Window::Height() - 200 + i * 20, Palette::Orange);
			++i;
		}
	}

	// �ǂ񂾕]���\�t�g�E�ǂ񂾎萔�E�ǂ񂾎��Ԃ̕\��
	if (mEvaluator.IsActive())
	{
		wstring wname(mEvaluator.GetName().begin(), mEvaluator.GetName().end());
		mGui.mEvaluator.text(L"evaluator_name").text = Format(L"�]���\�t�g ", wname, L"  �ǂ񂾎萔 ", mEvaluator.GetPonderNodes(), L"��  �ǂ񂾎��� ", mEvaluator.GetPonderTime(), L"�~���b");
	}

	// �I�v�V�����̐ݒ�
	{
		String optionString;
		if (mEvaluator.IsOptionRead())
		{
			optionString = Format(L"�ݒ�ς�");
		}
		else
		{
			optionString = Format(L"���ݒ�");
		}
		mGui.mEvaluator.text(L"option_name").text = optionString;
	}

	// �؂̃����N
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

			if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_TE))
			{
				const int centerX = static_cast<int>(deX); // (0.5f * (stX + deX));
				const int centerY = static_cast<int>(0.5f * (stY + deY));
				mFont(link.teJap).drawCenter(centerX, centerY);
			}
		}
	}

	// �؂̃m�[�h
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		const int centerX = static_cast<int>(ScaleX(node.mVisualX));
		const int centerY = static_cast<int>(ScaleY(node.mVisualY));
		if (mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SHOW_DEBUG))
		{
			// �f�o�b�O�p�F�\���ꏊ�̕\��
			mFont(centerX, L",", centerY).draw(centerX, centerY, Palette::Orange);
		}

		// �m�[�h�w�i�̕\��
		Color color = Palette::White;
		const NodeSize nodeSize = mGui.mSettings.checkBox(L"settings").checked(GuiSiv3D::SMALL_NODE) ? NS_SMALL : NS_BIG;
		const s3d::RoundRect& roundRect = GetNodeShape(centerX, centerY, nodeSize);
		if (nodeID == GetSelectedNodeID())
		{
			color = Color{ 0xff, 0xf0, 0x01, 0xff }; //  Palette::Yellow;
		}
		else if (roundRect.contains(Mouse::Pos()))
		{
			color = Palette::Orange; // Color{ 0xde, 0x96, 0x10, 0xff }; 
		}
		else if (mEvaluator.IsNodeEvaluating(nodeID))
		{
			color = Color{ 0xa0, 0xc2, 0x38, 0xff }; // Palette::Lightgreen;
		}
		else if (IsBestRoute(nodeID))
		{
			color = Palette::Lightblue;
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

		// �^�O�̕\��
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


	// �菇
	{
		const Node& node = mNodes[GetSelectedNodeID()];
		if (node.IsScoreEvaluated())
		{
			mGui.mScore.text(L"score").text = Format(L"�]���l ", node.ConverScoreToWString());
			mGui.mScore.text(L"tejunJap").text = AddNewLine(node.mTejunJap, 1);
		}
		else
		{
			mGui.mScore.text(L"score").text = Format(L"���]�� -----");
			mGui.mScore.text(L"tejunJap").text = L"";
		}

		//		// �]���l�o�[
		//		if (node.IsScoreEvaluated())
		//		{
		//			DrawScoreBar(node.mScore, 2000, 400, 200, 400, 50);
		//		}
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


			for (const Link& link : node.mLinks)
			{
				const Color grabbedColor[NUM_SEN_GO] =
				{
					{ 255, 0, 0, 127 },
					{ 0, 0, 255, 127 },
				};

				int cy, cx;

				// ���\��
				boardSiv3D->DrawMove(link.te, grabbedColor[boardSiv3D->GetTeban()], cy, cx);

				// ����1���̃m�[�h�̕]���l��������΁A�������ŕ\��
				const Node& destNode = GetNode(link.destNodeID);
				if (destNode.IsScoreEvaluated())
				{
					ScoreOnArrow tmp = { cy, cx, &destNode };
					scoreOnArrows.push_back(tmp);
				}
			}

			// �őP��̕\���i���j
			if (node.IsScoreEvaluated())
			{
				int cy, cx;
				const string firstTe = boardSiv3D->GetFirstTeFromTejun(node.mBestTejun);

				// ���\��
				if (!firstTe.empty())
				{
					boardSiv3D->DrawMove(firstTe, { 255, 0, 255, 127 }, cy, cx);
				}
			}

			for (const ScoreOnArrow& tmp : scoreOnArrows)
			{
				// �m�[�h�w�i�̕\��
				Color color = Palette::White;
				const s3d::RoundRect& roundRect = GetNodeShape(tmp.cx, tmp.cy, NS_MEDIUM);
				roundRect.draw(color);

				// �m�[�h�X�R�A�̕\��
				DrawScore(tmp.cx, tmp.cy, *tmp.pDestNode, NS_MEDIUM);
			}
		}
	}
}


// �]���l�̕\��
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

// �m�[�h�̕\���Ɏg���}�`��Ԃ�
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

// ���ݑI�𒆂̃m�[�h���ύX�����u�ԂɌĂ΂��֐�
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

// ���C�����[�v
void TreeSiv3D::Update()
{
	// TODO ���j���[��tree3d���番�����āATreeSiv3D->BoardSiv3D�ƎQ�Ƃ��Ȃ��悤�ɁB
	{
		BoardSiv3D* boardSiv3D = dynamic_cast<BoardSiv3D*>(mBoard);
		if (boardSiv3D != nullptr)
		{
			boardSiv3D->SetOffset(mGui.mBoard.getPos().x + 100, mGui.mBoard.getPos().y+70);
		}
	}


	// ���j���[
	if (mGui.mFile.button(L"kifu_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"��Ճr���[���t�@�C�� (*.jsv)", L"*.jsv" } });
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
		const auto path = Dialog::GetSave({ { L"��Ճr���[���t�@�C�� (*.jsv)", L"*.jsv" } });
		if (path.has_value())
		{
			Save(path.value().str());
			Window::SetTitle(GetVersionTitle(path.value().str()));
		}
	}
	else if (mGui.mFile.button(L"kif_format_load").pushed)
	{
		const auto path = Dialog::GetOpen({ { L"kif�t�@�C�� (*.kif)", L"*.kif" } });
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

	// ���Ԃ̍X�V
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
			// 1��߂�
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
			// ���S���W�̕ύX�i�O�t���[������̃J�[�\���̈ړ��ʁj
			const Point delta = Mouse::Delta();
			mOffsetX += delta.x;
			mOffsetY += delta.y;
		}
		else if (Input::MouseL.clicked)
		{
			// �m�[�h�̑I��
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

		// �g��k��
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

		// �폜
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

	// �m�[�h���
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
			const Point delta(20, 26); // �E�B���h�E����T�}���g�ւ̑��΍��W
			const Rect rect = mFontGuiDefault(node.mSummary).region(mGui.mBoard.getPos() + delta);
			IME::SetCompositionWindowPos(Point(rect.x + rect.w, rect.y));
		}
	}

	// �e�N���X�̍X�V
	Tree::Update();
	mEvaluator.Update();
}

// �m�[�h�I������炷
void TreeSiv3D::PlayNodeSelectSound()
{
	if (mNodeSelectSound)
	{
		if (mNodeSelectSound.isPlaying())
		{
			// ��~���ċȂ̐擪�ɖ߂�
			mNodeSelectSound.stop();
		}
		mNodeSelectSound.play();
	}
}

// ���W�������Ղ̒����ǂ������`�F�b�N
bool TreeSiv3D::IsInShogiban(int x, int y) const
{
	return	INRANGE(x, mGui.mBoard.getPos().x, mGui.mBoard.getPos().x + mShogibanWidth) &&
			INRANGE(y, mGui.mBoard.getPos().y, mGui.mBoard.getPos().y + mShogibanHeight);
}
