#include "tree.h"

Node::Node()
{
	mState = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
	mLinks.clear();
	mScore = SCORE_NOT_EVALUATED;
	mComment.clear();

	mVisualX = 0;
	mVisualY = 0;
	mWidth = 0;

	mParentNodeID = NG;
}


void Node::AddLink(int newNodeID, const string& te, const wstring* pTeJap)
{
	Link link;

	link.destNodeID = newNodeID;
	link.te = te;
	if (pTeJap != nullptr)
	{
		link.teJap = *pTeJap;
	}

	mLinks.push_back(link);
}

int Node::HasLink(const string& te) const
{
	for (const auto& link : mLinks)
	{
		if (link.te == te)
		{
			return link.destNodeID;
		}
	}

	return NG;
}

Tree::Tree(Board* board)
{
	mVersion = 1;
	mSelectedNodeID = 0;
	mNodes.clear();
	mNodes.push_back(Node());
	mBoard = board; // 外でnew deleteするので、ポインタ渡すだけ
	CalculateVisualPos();
}

void Tree::Draw()
{
	mBoard->Draw();
};

void Tree::Update()
{
	string	te;
	wstring	teJap;
	if (mBoard->Update(te, teJap))
	{
		AddLink(te, &teJap);
	}
};

void Tree::AddLink(const string& te, const wstring* pTeJap)
{
	int nextNodeID = mNodes[mSelectedNodeID].HasLink(te);
	if (nextNodeID == NG)
	{
		// リンクがないときは、リンクも次のノードも足して、
		Node nextNode;

		nextNode.mState = mBoard->GetState();
		nextNode.mScore = Node::SCORE_NOT_EVALUATED;
		nextNode.mParentNodeID = mSelectedNodeID;

		mNodes.push_back(nextNode);
		nextNodeID = SZ(mNodes) - 1;
		mNodes[mSelectedNodeID].AddLink(nextNodeID, te, pTeJap);
		mSelectedNodeID = nextNodeID;

		CalculateVisualPos();
	}
	else
	{
		// もしリンクがあるときは、そちらへ進める
		mSelectedNodeID = nextNodeID;
	}
}

int Tree::GetRootNodeID()
{
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		if (mNodes[nodeID].IsRoot())
		{
			return nodeID;
		}
	}

	assert(0);
	return NG;
}

int Tree::GetNextEvaludatedNodeID() const
{
	for (int nodeID = 0; nodeID < SZ(mNodes); ++nodeID)
	{
		if (!mNodes[nodeID].IsScoreEvaluated())
		{
			return nodeID;
		}
	}

	return NG;
}

int Tree::DfsWidth(int nodeID)
{
	Node& node = mNodes[nodeID];

	node.mWidth = 0;
	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		node.mWidth += DfsWidth(node.mLinks[i].destNodeID);
	}

	if (SZ(node.mLinks) >= 1)
	{
		node.mWidth += 2 * (SZ(node.mLinks) - 1);
	}

	assert(node.mWidth >= 0);

	return node.mWidth;
}

void Tree::DfsVisualPos(int nodeID, int y, int x)
{
	Node& node = mNodes[nodeID];
	node.mVisualY = y;
	node.mVisualX = x;

	int dx = -node.mWidth / 2;
	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		const int nextNodeID = node.mLinks[i].destNodeID;
		const Node& nextNode = mNodes[nextNodeID];

		dx += nextNode.mWidth / 2;
		DfsVisualPos(nextNodeID, y + 1, x + dx);
		dx += nextNode.mWidth / 2;
		dx += 2;
	}
}

void Tree::DfsState(int nodeID)
{
	Node& node = mNodes[nodeID];
	node.mState = mBoard->GetState();

	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		Link& link = node.mLinks[i];
		
		const int nextNodeID = link.destNodeID;
		const Node& nextNode = mNodes[nextNodeID];

		link.teJap = mBoard->MoveByTe(link.te);
		DfsState(nextNodeID);
		mBoard->SetState(node.mState);
	}
}

void Tree::CalculateVisualPos()
{
	const int rootNodeID = GetRootNodeID();
	assert(rootNodeID != NG);
	DfsWidth(rootNodeID);;
	DfsVisualPos(rootNodeID, 0, 0);
}

void Tree::SetSelectedNodeID(int nodeID)
{
	mSelectedNodeID = nodeID;
	mBoard->SetState(mNodes[nodeID].mState);
}

void Tree::InitializeAfterLoad()
{
	mBoard->InitState();

	const int rootNodeID = GetRootNodeID();
	DfsState(rootNodeID);

	SetSelectedNodeID(rootNodeID);
}

