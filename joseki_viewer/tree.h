#pragma once

#include "board.h"
#include "util.h"
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
		GetLineTrim(wfs, ws);	destNodeID = stoi(ws);
		GetLineTrim(wfs, ws);	te = string(ws.begin(), ws.end());
	}
};

class Node
{
public:
	enum { SCORE_NOT_EVALUATED = 123456789 };

	Node();
	void AddLink(int newNodeID, const string& te, const wstring* pTeJap = nullptr);
	int HasLink(const string& te) const;
	bool IsRoot() const { return mParentNodeID == NG; }
	bool IsScoreEvaluated() const { return mScore != SCORE_NOT_EVALUATED;  }

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
		wfs << mSummary << endl;
		wfs << EasyEscape(mComment) << endl;
		wfs << wstring(mTejun.begin(), mTejun.end()) << endl;
	}

	void Load(wfstream& wfs)
	{
		wstring ws;
//		GetLineTrim(wfs, ws);	mState = string(ws.begin(), ws.end());
		GetLineTrim(wfs, ws);	mScore = stoi(ws);
		GetLineTrim(wfs, ws);	mParentNodeID = stoi(ws);

		int numLinks = 0;
		GetLineTrim(wfs, ws);	numLinks = stoi(ws);
		mLinks.clear();
		mLinks.resize(numLinks);
		for (int i = 0; i < SZ(mLinks); ++i)
		{
			mLinks[i].Load(wfs);
		}

		GetLineTrim(wfs, ws);	mSummary = ws;
		GetLineTrim(wfs, ws);	mComment = EasyUnescape(ws);
		GetLineTrim(wfs, ws);	mTejun = string(ws.begin(), ws.end());
	}

	// セーブする情報
	int				mScore;			// 評価値
	int				mParentNodeID;
	vector <Link>	mLinks;
	wstring			mSummary;		// コメントの要約
	wstring			mComment;		// コメント
	string			mTejun;			// 最善手

	// セーブしない情報
	string			mState;			// ルート以外は本当は必須でもない。
	float			mVisualX;
	float			mVisualY;
	float			mWidth;
	wstring			mTejunJap;
};

class Tree
{
public:
	Tree(Board* board);
	virtual ~Tree() {};
	virtual void Draw();
	virtual void Update();
	virtual void OnSelectedNodeIDChanged() {};
	void AddLink(const string& te, const wstring* pTeJap = nullptr);
	void CalculateVisualPos();
	int GetNextEvaludatedNodeID() const;
	const Node& GetNode(int nodeID) const { return mNodes[nodeID]; }
	void InitializeAfterLoad();
	void UpdateNode(int nodeID, int score, const string& tejun);
	void SetSelectedNodeID(int nodeID);
	int  GetSelectedNodeID() const { return mSelectedNodeID; }

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
		GetLineTrim(wfs, ws);	mVersion = stoi(ws);

		int numNodes = 0;
		GetLineTrim(wfs, ws);	numNodes = stoi(ws);

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
	float DfsWidth(int nodeID);
	void DfsVisualPos(int nodeID, float y, float x);
	void DfsState(int nodeID);
	void DeleteSelectedNode();

	// セーブする
	int mVersion;
	vector <Node> mNodes;

	// セーブしない
	Board* mBoard;

private:
	void DfsCalcNewNodeID(int nodeID, vector <int>& newNodeIDs);
	int mSelectedNodeID;
};
