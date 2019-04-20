#include "tree.h"

Node::Node()
{
	mState = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
	mLinks.clear();
	mScore = SCORE_NOT_EVALUATED;
	mSummary.clear();
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
	mVersion = VERSION_DATA;
	mBoard = board; // �O��new delete����̂ŁA�|�C���^�n������
	Init();
}

void Tree::Init()
{
	mNodes.clear();
	mNodes.push_back(Node());
	CalculateVisualPos();
	SetSelectedNodeID(0);
	UpdateBestRouteNodeIDs();
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
		// �����N���Ȃ��Ƃ��́A�����N�����̃m�[�h�������āA
		Node nextNode;

		nextNode.mState = mBoard->GetState();
		nextNode.mScore = Node::SCORE_NOT_EVALUATED;
		nextNode.mParentNodeID = mSelectedNodeID;

		mNodes.push_back(nextNode);
		nextNodeID = SZ(mNodes) - 1;
		mNodes[mSelectedNodeID].AddLink(nextNodeID, te, pTeJap);
		CalculateVisualPos();
		SetSelectedNodeID(nextNodeID);
	}
	else
	{
		// ���������N������Ƃ��́A������֐i�߂�
		SetSelectedNodeID(nextNodeID);
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

float Tree::DfsWidth(int nodeID)
{
	Node& node = mNodes[nodeID];

	node.mWidth = 0.f;
	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		node.mWidth += DfsWidth(node.mLinks[i].destNodeID);
	}

	if (SZ(node.mLinks) >= 1)
	{
		node.mWidth += 2.0f * (SZ(node.mLinks) - 1.0f);
	}

	assert(node.mWidth >= 0);

	return node.mWidth;
}

// �m�[�h�̍��W���ċA�ŋ��߂�B
void Tree::DfsVisualPos(int nodeID, float y, float x)
{
	Node& node = mNodes[nodeID];
	node.mVisualY = y;
	node.mVisualX = x;

	float dx = -node.mWidth / 2.0f;
	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		const int nextNodeID = node.mLinks[i].destNodeID;
		const Node& nextNode = mNodes[nextNodeID];

		dx += nextNode.mWidth / 2.0f;
		DfsVisualPos(nextNodeID, y + 1.0f, x + dx);
		dx += nextNode.mWidth / 2.0f;
		dx += 2.0f;
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
	UpdateBestRouteNodeIDs();
	OnSelectedNodeIDChanged();
}

void Tree::SetFixUpdatedNodeID(int nodeID)
{
	if (IsFixUpdatedNode())
	{
		if (nodeID == NG) return;
		if (mNodes[nodeID].mParentNodeID == NG) return;
		SetSelectedNodeID(mNodes[nodeID].mParentNodeID);
	}
}

void Tree::InitializeAfterLoad()
{
	mBoard->Init();

	const int rootNodeID = GetRootNodeID();
	DfsState(rootNodeID);

	// ���{��菇�����v�Z
	for (Node& node : mNodes)
	{
		if (!node.mBestTejun.empty())
		{
			mBoard->SetState(node.mState);
			node.mTejunJap = mBoard->MoveByTejun(node.mBestTejun);
		}
	}

	SetSelectedNodeID(rootNodeID);
}

void Tree::UpdateNode(int nodeID, int score, const string& tejun, bool isMate)
{
	Node& node = mNodes[nodeID];

	// ���̃m�[�h�����Ԃ����Ԃ�

	node.mBestTejun = tejun;

	Board tmpBoard;
	tmpBoard.SetState(node.mState);
	switch (tmpBoard.GetTeban())
	{
	case E_SEN:
		node.mScore = score;
		break;

	case E_GO:
		node.mScore = -score;
		break;

	default:
		assert(0);
		break;
	}

	if (isMate)
	{
		node.mScore = -node.mScore;
	}

	node.mTejunJap = tmpBoard.MoveByTejun(tejun);
}

// nodeID�̎q���̃m�[�h���A�ċA��newNodeIDs�ɗ񋓂���B
void Tree::DfsCalcNewNodeID(int nodeID, vector <int>& newNodeIDs)
{
	newNodeIDs.push_back(nodeID);

	const Node& node = mNodes[nodeID];

	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		const Link& link = node.mLinks[i];

		const int nextNodeID = link.destNodeID;
		DfsCalcNewNodeID(nextNodeID, newNodeIDs);
	}
}

// nodeID�̎q���̃m�[�h�̒��ŁA���ݑI�𒆃m�[�h�̎q���ȊO���A�ċA��newNodeIDs�ɗ񋓂���B
void Tree::DfsCalcNewNodeIDExceptSelected(int nodeID, vector <int>& newNodeIDs)
{
	if (nodeID == mSelectedNodeID)
	{
		return;	// �I���m�[�h�Ƃ��������������̂ŁAnewNodeIDs�͓o�^���Ȃ�
	}

	newNodeIDs.push_back(nodeID);

	const Node& node = mNodes[nodeID];

	for (int i = 0; i < SZ(node.mLinks); ++i)
	{
		const Link& link = node.mLinks[i];

		const int nextNodeID = link.destNodeID;
		DfsCalcNewNodeIDExceptSelected(nextNodeID, newNodeIDs);
	}
}

// mSelectedNodeID�̉ߋ��̃m�[�h+�]���ς݂̒��Ńx�X�g���[�g�����ǂ����Ƃ���
void Tree::UpdateBestRouteNodeIDs()
{
	mBestRouteNodeIDs.clear();

	// �ߋ��̃m�[�h�����ǂ�
	mTesu = 0;
	for (int nodeID = mSelectedNodeID; nodeID!=NG;)
	{
		mBestRouteNodeIDs.push_back(nodeID);
		nodeID = mNodes[nodeID].mParentNodeID;
		mTesu++;
	}
	
	reverse(mBestRouteNodeIDs.begin(), mBestRouteNodeIDs.end());


	// �����̃m�[�h�����ǂ�
	for (int nodeID = mSelectedNodeID; ;)
	{
		// ���[�g�܂Ŗ߂��O��Ƃ��Ă���̂ŁA����Ő���肪������
		int sengo = (SZ(mBestRouteNodeIDs) + 1) % 2;
		assert(sengo != E_NO_SENGO);

		const Node& node = mNodes[nodeID];

		int bestScore = Node::SCORE_RESIGN + 10000; // ���̓X�R�A�ŏ����i�ň����ő�j
		if (sengo == E_SEN)
		{
			bestScore = -bestScore;	// ���̓X�R�A�ő剻�i�ň����ŏ��j
		}
		int bestNextNodeID = NG;

		for (int i = 0; i < SZ(node.mLinks); ++i)
		{
			const Link& link = node.mLinks[i];
			const int nextNodeID = link.destNodeID;
			const Node& nextNode = mNodes[nextNodeID];
			if (nextNode.IsScoreEvaluated())
			{
				if (sengo == E_SEN && nextNode.mScore > bestScore ||
					sengo == E_GO && nextNode.mScore < bestScore)
				{
					bestScore = nextNode.mScore;
					bestNextNodeID = nextNodeID;
				}
			}
			else
			{
				// �]������Ă��Ȃ��m�[�h���܂߂�ꍇ
				bestNextNodeID = nextNodeID;
			}
		}

		if (bestNextNodeID == NG)
		{
			break;
		}
		else
		{
			mBestRouteNodeIDs.push_back(bestNextNodeID);
			nodeID = bestNextNodeID;
		}
	}
}

void Tree::DeleteSelectedAncientNode()
{
	const int rootNodeID = GetRootNodeID();
	if (rootNodeID == mSelectedNodeID)
	{
		return;
	}

	vector <int> newNodeIDs;
	DfsCalcNewNodeIDExceptSelected(rootNodeID, newNodeIDs);

	vector <int> invNewNodeIDs(SZ(mNodes), NG);
	for (int i = 0; i < SZ(newNodeIDs); ++i)
	{
		invNewNodeIDs[newNodeIDs[i]] = i;
	}

	vector <Node> mNewNodes;
	for (int i = 0; i < SZ(newNodeIDs); ++i)
	{
		mNewNodes.push_back(mNodes[newNodeIDs[i]]);
	}

	for (int i = 0; i < SZ(mNewNodes); ++i)
	{
		Node& newNode = mNewNodes[i];

		if (newNode.mParentNodeID != NG)
		{
			newNode.mParentNodeID = invNewNodeIDs[newNode.mParentNodeID];
			assert(newNode.mParentNodeID != NG);	// �e�������Ă���\���͂Ȃ�
		}

		vector <Link>	newLinks;
		for (int k=0;k<SZ(newNode.mLinks);++k)
		{
			const int newDestNodeID = invNewNodeIDs[newNode.mLinks[k].destNodeID];
			if (newDestNodeID != NG)
			{
				newLinks.push_back(newNode.mLinks[k]);
				newLinks[SZ(newLinks) - 1].destNodeID = newDestNodeID;
			}
		}

		newNode.mLinks = newLinks;
	}

	const int newSelectedNodeID = mNodes[mSelectedNodeID].mParentNodeID;
	assert(newSelectedNodeID != NG);

	mNodes = mNewNodes;
	SetSelectedNodeID(newSelectedNodeID);
	CalculateVisualPos();
}

void Tree::ResetSelectedScore()
{
	mNodes[mSelectedNodeID].ResetScore();
	UpdateBestRouteNodeIDs();
}

void Tree::ResetSelectedAncientScore()
{
	vector <int> newNodeIDs;
	DfsCalcNewNodeID(mSelectedNodeID, newNodeIDs);

	for (int nodeID : newNodeIDs)
	{
		mNodes[nodeID].ResetScore();
	}
	UpdateBestRouteNodeIDs();
}


