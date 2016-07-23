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
	int		nodeID;
	string	te;

	// セーブしない
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


	// セーブする情報
	string			mState;			// ルート以外は本当は必須でもない。
	int				mScore;			// 評価値
	int				mParentNodeID;

private:
	// セーブする情報
	vector <Link>	mLinks;
	wstring			mComment;		// コメント

									// セーブしない情報
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
		mBoard = board; // 外でnew deleteするので、ポインタ渡すだけ
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
			// リンクがないときは、リンクも次のノードも足して、
			Node nextNode;

			// currentNode.state + te = 
			//			mBoard->SetState(currentNode.mState); // 不要なはず
			// 手をすすめる

			nextNode.mState = mBoard->GetState();
			nextNode.mScore = mBoard->CalcBestMoveAndScore();
			nextNode.mParentNodeID = mNodeID;


			mNodes.push_back(nextNode);
			nextNodeID = SZ(mNodes) - 1;
			mNodes[mNodeID].AddLink(nextNodeID, te, teJap);
			mNodeID = nextNodeID;

			// 全ノードの表示座標を再計算
		}
		else
		{
			// もしリンクがあるときは、そちらへ進める
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