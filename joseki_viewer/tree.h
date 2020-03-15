// tree.h : 棋譜の木についてのファイル。ここに書いてあるクラスは、Siv3Dには依存せず、単体テストも可能(unittest1.cpp)。

#pragma once

#include "board.h"
#include "util.h"

#define MAX_SAVE_SIZE	(0x1000)

// Link : 棋譜の木のリンク。１手を表す。
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

// Node : 棋譜の木のノード。局面を表す。１つのノードは、複数のリンク（手の分岐）を持つ。
class Node
{
public:
	enum { SCORE_NOT_EVALUATED = 123456789 }; // 評価がされていない状態
	enum { SCORE_RESIGN        = 999999999 }; // 投了
	enum { SCORE_MATE          =  10000000 }; // x手詰みを表す

	Node();
	void AddLink(int newNodeID, const string& te, const wstring* pTeJap = nullptr);
	int HasLink(const string& te) const;
	bool IsRoot() const { return mParentNodeID == NG; }
	bool IsScoreEvaluated() const { return mScore != SCORE_NOT_EVALUATED;  }
	static bool IsScoreEvaluated(int score) { return score != SCORE_NOT_EVALUATED; }
	bool IsResign() const { return mScore == SCORE_RESIGN || mScore == -SCORE_RESIGN; }
	bool IsSenteKachi() const { return mScore ==  SCORE_RESIGN; }
	bool IsGoteKachi() const  { return mScore == -SCORE_RESIGN; }
	bool IsBranch() const { return SZ(mLinks) >= 2; }

	void ResetScore() { mScore = SCORE_NOT_EVALUATED; } 
	int GetParentNodeID() const { return mParentNodeID; }


	static int ConvertMateToScore(int mate) 
	{
		// mate = 0 は投了

		if (mate == 0)
		{
			return SCORE_RESIGN;
		}
		else if (mate > 0)
		{
			return SCORE_MATE + mate;
		}
		else if (mate < 0)
		{
			return -SCORE_MATE + mate;
		}

		return 0;
	}

	string ConverScoreToString() const
	{
		if (mScore == SCORE_NOT_EVALUATED)
		{
			return "";
		}
		else if (IsResign())
		{
			return "MATE";
		}
		else if (mScore >= SCORE_MATE)
		{
			return "M " + to_string(mScore - SCORE_MATE);
		}
		else if (mScore <= -SCORE_MATE)
		{
			return "M " + to_string(-mScore - SCORE_MATE);
		}
		else if (mScore > 0)
		{
			return to_string(mScore);
		}
		else if (mScore < 0)
		{
			return to_string(-mScore);
		}
		else
		{
			return to_string(mScore);
		}
	}

	wstring ConverScoreToWString() const
	{
		if (mScore == SCORE_NOT_EVALUATED)
		{
			return L"";
		}
		else if (mScore == SCORE_RESIGN)
		{
			return L"投了";
		}
		else if (mScore >= SCORE_MATE)
		{
			return L"先手 " + to_wstring(mScore - SCORE_MATE) + L"手詰";
		}
		else if (mScore <= -SCORE_MATE)
		{
			return L"後手 " + to_wstring(-mScore - SCORE_MATE) + L"手詰";
		}
		else if (mScore > 0)
		{
			return L"先手 " + to_wstring(mScore);
		}
		else if (mScore < 0)
		{
			return L"後手 " + to_wstring(-mScore);
		}
		else
		{
			return L"互角";
		}
	}

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
		wfs << wstring(mBestTejun.begin(), mBestTejun.end()) << endl;
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
		GetLineTrim(wfs, ws);	mBestTejun = string(ws.begin(), ws.end());
	}

	// セーブする情報
	int				mScore;			// 評価値
	int				mParentNodeID;
	vector <Link>	mLinks;
	wstring			mSummary;		// コメントの要約
	wstring			mComment;		// コメント
	string			mBestTejun;		// 最善手

	// セーブしない情報
	string			mState;			// ルート以外は本当は必須でもない。
	float			mVisualX;
	float			mVisualY;
	float			mWidth;
	wstring			mTejunJap;
};


// 棋譜の木。複数のノードと、それぞれのノードが持つ複数のリンクから成る。
class Tree
{
public:
	Tree(Board* board);
	virtual ~Tree() {};
	virtual void Draw();
	virtual void Update();
	virtual void OnSelectedNodeIDChanged() {};
	virtual bool IsFixUpdatedNode() const { return false; };

	void Init();
	void AddLink(const string& te, const wstring* pTeJap = nullptr);
	void CalculateVisualPos();
	int GetNextEvaludatedNodeID() const;
	const Node& GetNode(int nodeID) const { return mNodes[nodeID]; }
	void InitializeAfterLoad();
	void UpdateNode(int nodeID, int score, const string& tejun, bool isMate=false);
	void SetSelectedNodeID(int nodeID);
	void SetFixUpdatedNodeID(int nodeID);
	int  GetSelectedNodeID() const { return mSelectedNodeID; }
	const Node& GetSelectedNode() const { return mNodes[mSelectedNodeID]; }
	int GetTesu() const { return mTesu; };

	void Save(const wstring& path)
	{
		wfstream wfs;
		wfs.open(path, std::fstream::out);

		wfs << VERSION_DATA << endl; // mVersion
		mKifHeader.Save(wfs);

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

		if (mVersion >= 2)
		{
			mKifHeader.Load(wfs);
		}

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

	void InitKif(const vector <wstring>& vws)
	{
		Init();

		// ヘッダーのセット（てきとー）
		for (const wstring& ws : vws)
		{
			if (ws.find(L"開始日時：") == 0)
			{
				mKifHeader.mKaishiNichiji = ws.substr(SZ(wstring(L"開始日時：")));
			}
			else if (ws.find(L"棋戦：") == 0)
			{
				mKifHeader.mKisen = ws.substr(SZ(wstring(L"棋戦：")));
			}
			else if (ws.find(L"持ち時間：") == 0)
			{
				mKifHeader.mMochijikan = ws.substr(SZ(wstring(L"持ち時間：")));
			}
			else if (ws.find(L"手合割：") == 0)
			{
				mKifHeader.mTeaiWari = ws.substr(SZ(wstring(L"手合割：")));
			}
			else if (ws.find(L"先手：") == 0)
			{
				mKifHeader.mSente = ws.substr(SZ(wstring(L"先手：")));
			}
			else if (ws.find(L"後手：") == 0)
			{
				mKifHeader.mGote = ws.substr(SZ(wstring(L"後手：")));
			}
		}

		mBoard->Init();
		const string tejun = mBoard->GetTejunFromKif(vws);
		{
			vector <string> vs;
			Split1(tejun, vs);

			for (const string& te : vs)
			{
				wstring	teJap = mBoard->MoveByTe(te);
				AddLink(te, &teJap);
			}
		}
		mBoard->SetKifHeader(mKifHeader);
	}

	void PasteKif(const wstring& rawKif)
	{
		vector <wstring> vws;
		Split1(rawKif, vws, '\n');
		InitKif(vws);
	}

	// 今は使用していません！（これだとUTF-8が読めないので、tree_siv3D内でTextReaderクラスを使うことで対応した）
	void LoadKif_Deprecated(const wstring& path)
	{
		wfstream wfs;
		wfs.open(path, std::fstream::in);

		vector <wstring> vws;
		{
			wstring ws;
			while (getline(wfs, ws))
			{
				vws.push_back(ws);
			}
		}

		InitKif(vws);

		wfs.close();
	}

protected:
	Tree() {};

	int GetRootNodeID();
	float DfsWidth(int nodeID);
	void DfsVisualPos(int nodeID, float y, float x);
	void DfsState(int nodeID);
	void DeleteSelectedAncientNode();
	void ResetSelectedScore();
	void ResetSelectedAncientScore();
	bool IsBestRoute(int nodeID) const
	{
		for (int n : mBestRouteNodeIDs)
		{
			if (nodeID == n)
			{
				return true;
			}
		}

		return false;
	}

	const vector <int>& GetBestRouteNodeIDs() const
	{
		return mBestRouteNodeIDs;
	}

	vector <int> GetBestRouteScores() const
	{
		vector <int> ret;
		for (int nodeID :  mBestRouteNodeIDs)
		{
			ret.push_back(mNodes[nodeID].mScore);
		}
		return ret;
	}

	vector <bool> GetBestRouteBranch() const
	{
		vector <bool> ret;
		for (int nodeID : mBestRouteNodeIDs)
		{
			ret.push_back(mNodes[nodeID].IsBranch());
		}
		return ret;
	}


	// セーブする
	int mVersion;
	vector <Node> mNodes;
	KifHeader mKifHeader;

	// セーブしない
	Board* mBoard;

private:
	void DfsCalcNewNodeIDExceptSelected(int nodeID, vector <int>& newNodeIDs);
	void DfsCalcNewNodeID(int nodeID, vector <int>& newNodeIDs);
	void UpdateBestRouteNodeIDs();

	int mSelectedNodeID;
	int mTesu = 0;
	vector <int> mBestRouteNodeIDs;
};
