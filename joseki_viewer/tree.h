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
	int		destNodeID;
	string	te;

	// �Z�[�u���Ȃ�
	wstring teJap;
};

class Node
{
public:
	Node();
	void AddLink(int newNodeID, const string& te, const wstring* pTeJap = nullptr);
	int HasLink(const string& te) const;
	bool IsRoot() const { return mParentNodeID == NG; }

	// �Z�[�u������
	string			mState;			// ���[�g�ȊO�͖{���͕K�{�ł��Ȃ��B
	int				mScore;			// �]���l
	int				mParentNodeID;

	// �Z�[�u������
	vector <Link>	mLinks;
	wstring			mComment;		// �R�����g

	// �Z�[�u���Ȃ����
	int mVisualX;
	int mVisualY;
	int mWidth;
};

class Tree
{
public:
	Tree() {};
	Tree(Board* board);
	virtual ~Tree() {};
	virtual void Draw();
	virtual void Update();
	void AddLink(const string& te, const wstring* pTeJap = nullptr);
	void CalculateVisualPos();
	void SetSelectedNodeID(int nodeID) { mSelectedNodeID = nodeID;  }
	const Node& GetNode(int nodeID) const { return mNodes[nodeID]; }
private:
	int GetRootNodeID();
	int DfsWidth(int nodeID);
	void DfsVisualPos(int nodeID, int y, int x);

	int mVersion;
	int mSelectedNodeID;
	vector <Node> mNodes;

	Board* mBoard;
};
