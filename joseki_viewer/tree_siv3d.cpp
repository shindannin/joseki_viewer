#include "tree_siv3d.h"


void TreeSiv3D::Draw()
{
	Tree::Draw();

	// ƒŠƒ“ƒN
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		for (const Link link : node.mLinks)
		{
			const Node& destNode = mNodes[link.destNodeID];

			Line(ScaleX(node.mVisualX), ScaleY(node.mVisualY), ScaleX(destNode.mVisualX), ScaleY(destNode.mVisualY)).draw();
		}
	}

	// ƒm[ƒh
	for (int nodeID = 0; nodeID<SZ(mNodes); ++nodeID)
	{
		const Node& node = mNodes[nodeID];
		Color color = Palette::White;
		if (nodeID == mSelectedNodeID)
		{
			color = Palette::Yellow;
		}
		Circle(ScaleX(node.mVisualX), ScaleY(node.mVisualY), mNodeRadius).draw(color);
	}


}

void TreeSiv3D::Update()
{
	Tree::Update();
/*

	string	te;
	wstring	teJap;
	if (mBoard->Update(te, teJap))
	{
		AddLink(te, &teJap);
	}
*/
}
