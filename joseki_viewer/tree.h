#pragma once

#include "board.h"
#include <fstream>

#define MAX_SAVE_SIZE	(0x1000)

struct Link
{
	// セーブする
	int		destNodeID;
	string	te;

	// セーブしない
	wstring teJap;

	void Save(wfstream& wfs)
	{
		wfs << destNodeID << endl;
		wfs << wstring(te.begin(), te.end()) << endl;
	}

	void Load(wfstream& wfs)
	{
		wstring ws;
		getline(wfs, ws);	destNodeID = stoi(ws);
		getline(wfs, ws);	te = string(ws.begin(), ws.end());
	}
};

class Node
{
public:
	Node();
	void AddLink(int newNodeID, const string& te, const wstring* pTeJap = nullptr);
	int HasLink(const string& te) const;
	bool IsRoot() const { return mParentNodeID == NG; }

	void Save(wfstream& wfs)
	{
//		wfs << wstring(mState.begin(), mState.end()) << endl;
		wfs << mScore << endl;
		wfs << mParentNodeID << endl;
		wfs << SZ(mLinks) << endl;
		for (int i = 0; i < SZ(mLinks); ++i)
		{
			mLinks[i].Save(wfs);
		}
		wfs << mComment << endl;;		// コメント
	}

	void Load(wfstream& wfs)
	{
		wstring ws;
//		getline(wfs, ws);	mState = string(ws.begin(), ws.end());
		getline(wfs, ws);	mScore = stoi(ws);
		getline(wfs, ws);	mParentNodeID = stoi(ws);

		int numLinks = 0;
		getline(wfs, ws);	numLinks = stoi(ws);
		mLinks.clear();
		mLinks.resize(numLinks);
		for (int i = 0; i < SZ(mLinks); ++i)
		{
			mLinks[i].Load(wfs);
		}

		getline(wfs, ws);	mComment = ws;
	}

	// セーブする情報
	int				mScore;			// 評価値
	int				mParentNodeID;
	vector <Link>	mLinks;
	wstring			mComment;		// コメント

	// セーブしない情報
	string			mState;			// ルート以外は本当は必須でもない。
	int mVisualX;
	int mVisualY;
	int mWidth;
};

class Tree
{
public:
	Tree(Board* board);
	virtual ~Tree() {};
	virtual void Draw();
	virtual void Update();
	void AddLink(const string& te, const wstring* pTeJap = nullptr);
	void CalculateVisualPos();
	void SetSelectedNodeID(int nodeID);
	const Node& GetNode(int nodeID) const { return mNodes[nodeID]; }
	void InitializeAfterLoad();

	void Save(const wstring& path)
	{
		wfstream wfs;
		wfs.open(path, std::fstream::out);

		wfs << mVersion << endl;
		wfs << SZ(mNodes) << endl;
		for (int i = 0; i < SZ(mNodes); ++i)
		{
			mNodes[i].Save(wfs);
		}

		wfs.close();
	};

	void Load(const wstring& path)
	{
		wfstream wfs;
		wfs.open(path, std::fstream::in);

		wstring ws;
		getline(wfs, ws);	mVersion = stoi(ws);

		int numNodes = 0;
		getline(wfs, ws);	numNodes = stoi(ws);

		mNodes.clear();
		mNodes.resize(numNodes);
		for (int i = 0; i < SZ(mNodes); ++i)
		{
			mNodes[i].Load(wfs);
		}

		wfs.close();
	}

protected:
	Tree() {};

	int GetRootNodeID();
	int DfsWidth(int nodeID);
	void DfsVisualPos(int nodeID, int y, int x);
	void DfsState(int nodeID);


	// セーブする
	int mVersion;
	vector <Node> mNodes;

	// セーブしない
	int mSelectedNodeID;
	Board* mBoard;

};
