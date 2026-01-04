// evaluator.h : 評価ソフトへのアクセスをする。

#pragma once

#include <Siv3D.hpp>
#include "util.h"

// siv3dではなくwindows依存のコード。
// TODO : この部分は、Siv3Dライブラリに、機能追加されたはずなので、そちらを使用するようにする。
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
namespace s3d
{
	// class Server : 標準入出力
	class Server
	{
	private:

		HANDLE m_wo, m_ro, m_wi, m_ri;

		PROCESS_INFORMATION m_pi;

		bool m_connected = false;

	public:

		Server() = default;

		Server(FilePath path, bool show = true)
		{
			SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
			sa.bInheritHandle = true;
			::CreatePipe(&m_ro, &m_wo, &sa, 0);
			::SetHandleInformation(m_ro, HANDLE_FLAG_INHERIT, 0);
			::CreatePipe(&m_ri, &m_wi, &sa, 0);
			::SetHandleInformation(m_wi, HANDLE_FLAG_INHERIT, 0);

			STARTUPINFO si = { sizeof(STARTUPINFO) };
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = m_ri;
			si.hStdOutput = m_wo;

			if (!show)
			{
				si.dwFlags |= STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
			}

			FilePath currentPath = path.substr(0, path.lastIndexOf('/') + 1);
			m_connected = !!::CreateProcessW(path.c_str(), L"", nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, currentPath.c_str(), &si, &m_pi);
		}

		~Server()
		{
			::WaitForSingleObject(m_pi.hProcess, INFINITE);
			::CloseHandle(m_pi.hThread);
			::CloseHandle(m_pi.hProcess);
			::CloseHandle(m_wi);
			::CloseHandle(m_ri);
			::CloseHandle(m_wo);
			::CloseHandle(m_ro);
		}

		explicit operator bool() const
		{
			return isConnected();
		}

		bool isConnected() const
		{
			return m_connected;
		}

		bool write(const std::string& cmd)
		{
			if (!m_connected)
			{
				return false;
			}

			DWORD written;
			return !!::WriteFile(m_wi, cmd.c_str(), static_cast<DWORD>(cmd.length()), &written, nullptr);
		}

		size_t available() const
		{
			if (!m_connected)
			{
				return 0;
			}

			DWORD n = 0;

			if (!::PeekNamedPipe(m_ro, 0, 0, 0, &n, 0))
			{
				return 0;
			}

			return n;
		}

		bool read(std::string& dst)
		{
			dst.clear();

			const size_t n = available();

			if (!n)
			{
				return false;
			}

			dst.resize(n);

			DWORD size;

			::ReadFile(m_ro, &dst[0], static_cast<DWORD>(n), &size, 0);

			return true;
		}
	};
}

enum EStateEvaluation
{
	EStateEvaluation_FindingNode,	// 次に評価する局面を探している最中
	EStateEvaluation_WaitingScore,	// 評価中
};

class Tree;

// class Evaluator : 評価ソフトと評価設定
class Evaluator
{
public:
	Evaluator()
	{
	}

	Evaluator(Tree* tree) : mTree(tree)
	{
		Init();
	}

	~Evaluator()
	{
		Close();
	}

	void Init()
	{
		mEvaludatingNodeID = NG;
		mServer = nullptr;
		mEStateEvaluation = EStateEvaluation_FindingNode;
		mStopwatch.reset();
		mPollingStopwatch.reset();
		mPonderNodes = 0LL;
		mPonderTime = 0LL;
		mReadLogs.clear();
		mMultiPVNum = DEFAULT_MULTIPV_NUM;
		mStopSent = false;

		mPollingStopwatch.restart();
	}

	void Open();
	void OpenOption();
	void Close();
	bool Update();
	void RequestCancel();
	void SetMultiPVNum(int multiPVNum);
	int GetMultiPVNum() const { return mMultiPVNum; }

	bool IsActive() const { return mServer!=nullptr; }				// 評価ソフトを起動したか？
	bool IsOptionRead() const { return !mOptions.empty(); }			// 評価ソフトの初期設定を、ファイルからすでに読み込んだか？
	long long GetPonderNodes() const { return mPonderNodes; }	
	long long GetPonderTime() const { return mPonderTime; }
	const string& GetName() const { return mName; }
	bool IsNodeEvaluating(int nodeID) const { return mEStateEvaluation == EStateEvaluation_WaitingScore && mEvaludatingNodeID==nodeID; } // 現在評価しているノードであるならtrue

	int GetDurationSec() const { return mDurationMilliSec/1000; }
	void SetDurationSec(int sec) { mDurationMilliSec = sec * 1000; }
	const vector <string>& GetReadLogs() const { return mReadLogs;  }

private:
	void OpenSub(const FilePath& newEvaluatorPath);
	bool Go();
	bool ReceiveBestMoveAndScore();
	bool UpdateReadFromStdio();
	bool IsBestMoveReceived() const { return SZ(mReadLogs)>=1 && mReadLogs.back().find("bestmove") != string::npos; }
	bool IsUSIOKReceived() const { return SZ(mReadLogs) >= 1 && mReadLogs.back().find("usiok") != string::npos; }
	bool IsReadyOKReceived() const { return SZ(mReadLogs) >= 1 && mReadLogs.back().find("readyok") != string::npos; }

	int mEvaludatingNodeID;				// 現在評価中のノードID
	Server* mServer;					// 標準入出力
	EStateEvaluation mEStateEvaluation;	//
	Tree* mTree;						// Siv3Dに依存しない、棋譜の木
	Stopwatch mStopwatch;				// 評価の経過時間を計測するストップウォッチ
	Stopwatch mPollingStopwatch;		// 評価の経過時間を計測するストップウォッチ
	vector <string> mOptions;			// 評価ソフトの初期設定オプション
	long long mPonderNodes;				// 最善手を求めるのに、評価した手数
	long long mPonderTime;				// 最善手を求めるのに、評価した時間。単位はミリ秒
	string mName;						// 評価ソフトの名前（Aperyとか）
	FilePath mEvaluatorPath;			// 評価ソフトのファイルパス
	vector <string> mReadLogs;			// 評価ソフトから受け取ったログ

	bool mStopSent = false;				// go infinite に対して stop を送ったかどうか
	int mDurationMilliSec		            = DEFAULT_EVALUATION_SEC * 1000; // 評価時間。単位はミリ秒
	const int mDurationMilliSecPolling	    =  500;	// 定期的に評価ソフトに問い合わせる時間間隔。単位はミリ秒
	int mMultiPVNum						= DEFAULT_MULTIPV_NUM;
};
