#pragma once

#include "board.h"

// 1手さす
// もし子へのリンクに同じ手がなかったら、新しい子のノードを作成
// 座標も更新
// あれば移動

// 1手戻す
// 親のノードへ移動


struct Link
{
	// セーブする
	int		destNodeID;
	string	te;

	// セーブしない
	wstring teJap;
};

class Node
{
public:
	Node();
	void AddLink(int newNodeID, const string& te, const wstring* pTeJap = nullptr);
	int HasLink(const string& te) const;
	bool IsRoot() const { return mParentNodeID == NG; }

	// セーブする情報
	string			mState;			// ルート以外は本当は必須でもない。
	int				mScore;			// 評価値
	int				mParentNodeID;

	// セーブする情報
	vector <Link>	mLinks;
	wstring			mComment;		// コメント

	// セーブしない情報
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
