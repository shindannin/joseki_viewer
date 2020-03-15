// evaluator.h : �]���\�t�g�ւ̃A�N�Z�X������B

#pragma once

#include <Siv3D.hpp>
#include "util.h"

// siv3d�ł͂Ȃ�windows�ˑ��̃R�[�h�B
// TODO : ���̕����́ASiv3D���C�u�����ɁA�@�\�ǉ����ꂽ�͂��Ȃ̂ŁA��������g�p����悤�ɂ���B
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
namespace s3d
{
	// class Server : �W�����o��
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
	EStateEvaluation_FindingNode,	// ���ɕ]������ǖʂ�T���Ă���Œ�
	EStateEvaluation_WaitingScore,	// �]����
};

class Tree;

// class Evaluator : �]���\�t�g�ƕ]���ݒ�
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

		mPollingStopwatch.restart();
	}

	void Open();
	void OpenOption();
	void Close();
	bool Update();
	void RequestCancel();

	bool IsActive() const { return mServer!=nullptr; }				// �]���\�t�g���N���������H
	bool IsOptionRead() const { return !mOptions.empty(); }			// �]���\�t�g�̏����ݒ���A�t�@�C�����炷�łɓǂݍ��񂾂��H
	long long GetPonderNodes() const { return mPonderNodes; }	
	long long GetPonderTime() const { return mPonderTime; }
	const string& GetName() const { return mName; }
	bool IsNodeEvaluating(int nodeID) const { return mEStateEvaluation == EStateEvaluation_WaitingScore && mEvaludatingNodeID==nodeID; } // ���ݕ]�����Ă���m�[�h�ł���Ȃ�true

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

	int mEvaludatingNodeID;				// ���ݕ]�����̃m�[�hID
	Server* mServer;					// �W�����o��
	EStateEvaluation mEStateEvaluation;	//
	Tree* mTree;						// Siv3D�Ɉˑ����Ȃ��A�����̖�
	Stopwatch mStopwatch;				// �]���̌o�ߎ��Ԃ��v������X�g�b�v�E�H�b�`
	Stopwatch mPollingStopwatch;		// �]���̌o�ߎ��Ԃ��v������X�g�b�v�E�H�b�`
	vector <string> mOptions;			// �]���\�t�g�̏����ݒ�I�v�V����
	long long mPonderNodes;				// �őP������߂�̂ɁA�]�������萔
	long long mPonderTime;				// �őP������߂�̂ɁA�]���������ԁB�P�ʂ̓~���b
	string mName;						// �]���\�t�g�̖��O�iApery�Ƃ��j
	FilePath mEvaluatorPath;			// �]���\�t�g�̃t�@�C���p�X
	vector <string> mReadLogs;			// �]���\�t�g����󂯎�������O

	int mDurationMilliSec		            = DEFAULT_EVALUATION_SEC * 1000; // �]�����ԁB�P�ʂ̓~���b
	const int mDurationMilliSecPolling	    =  500;	// ����I�ɕ]���\�t�g�ɖ₢���킹�鎞�ԊԊu�B�P�ʂ̓~���b
};
