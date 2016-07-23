#pragma once

#include "board.h"

// 1�肳��
// �����q�ւ̃����N�ɓ����肪�Ȃ�������A�V�����q�̃m�[�h���쐬
// ���W���X�V
// ����Έړ�

// 1��߂�
// �e�̃m�[�h�ֈړ�


struct Link
{
	// �Z�[�u����
	int		nodeID;
	string	te;

	// �Z�[�u���Ȃ�
	wstring teJap;
};

class Node
{
public:
	Node()
	{
		mState = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
		mLinks.clear();
		mScore = 0;
		mComment.clear();

		mVisualX = 0;
		mVisualY = 0;
		mParentNodeID = NG;
	}


	void AddLink(int newNodeID, const string& te, const wstring& teJap)
	{
		Link link;

		link.nodeID = newNodeID;
		link.te = te;
		link.teJap = teJap;

		mLinks.push_back(link);
	}

	int HasLink(const string& te) const
	{
		for (const auto& link : mLinks)
		{
			if (link.te == te)
			{
				return link.nodeID;
			}
		}

		return NG;
	}


	// �Z�[�u������
	string			mState;			// ���[�g�ȊO�͖{���͕K�{�ł��Ȃ��B
	int				mScore;			// �]���l
	int				mParentNodeID;

private:
	// �Z�[�u������
	vector <Link>	mLinks;
	wstring			mComment;		// �R�����g

									// �Z�[�u���Ȃ����
	int mVisualX;
	int mVisualY;
};

class Tree
{
public:
	Tree(Board* board)
	{
		mVersion = 0;
		mNodeID = 0;
		mNodes.clear();
		mNodes.push_back(Node());
		mBoard = board; // �O��new delete����̂ŁA�|�C���^�n������
	}

	void Draw()
	{
		mBoard->Draw();
	};

	void Update()
	{
		string	te;
		wstring	teJap;
		if (mBoard->Update(te, teJap))
		{
			AddLink(te, teJap);
		}
	};

	void AddLink(const string& te, const wstring& teJap)
	{
		int nextNodeID = mNodes[mNodeID].HasLink(te);
		if (nextNodeID == NG)
		{
			// �����N���Ȃ��Ƃ��́A�����N�����̃m�[�h�������āA
			Node nextNode;

			// currentNode.state + te = 
			//			mBoard->SetState(currentNode.mState); // �s�v�Ȃ͂�
			// ��������߂�

			nextNode.mState = mBoard->GetState();
			nextNode.mScore = mBoard->CalcBestMoveAndScore();
			nextNode.mParentNodeID = mNodeID;


			mNodes.push_back(nextNode);
			nextNodeID = SZ(mNodes) - 1;
			mNodes[mNodeID].AddLink(nextNodeID, te, teJap);
			mNodeID = nextNodeID;

			// �S�m�[�h�̕\�����W���Čv�Z
		}
		else
		{
			// ���������N������Ƃ��́A������֐i�߂�
			mNodeID = nextNodeID;
		}
	}

private:
	int mVersion;
	int mNodeID;
	vector <Node> mNodes;

	Board* mBoard;

	virtual int CalcBestMoveAndScore() { return 0; }

};