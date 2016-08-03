#include "tree_siv3d.h"


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
