// のこりの実装項目

// 重要なやつ
// - 評価ソフトの時間変更
// - 再評価
// - Undo
// - デザイン
// - コメントとタイトルを分ける？
// - 移動チェック



#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "tree.h"

// siv3dではなくwindows依存のコード。
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
namespace s3d
{
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
	EStateEvaluation_FindingNode,
	EStateEvaluation_WaitingScore,
	EStateEvaluation_WaitingCancel,
};

class Tree;

class Evaluator
{
public:
	Evaluator()
	{
	}

	Evaluator(Tree* tree) : mTree(tree)
	{
		mEvaludatingNodeID = NG;
		mServer = nullptr;
		mEStateEvaluation = EStateEvaluation_FindingNode;
	}

	~Evaluator()
	{
		Close();
	}

	void Open();
	void Close();
	void Update();
	void RequestCancel();

private:
	bool Go();
	void ReceiveBestMoveAndScore();
	void WaitAndCancel();

	int mEvaludatingNodeID;
	Server* mServer;
	EStateEvaluation mEStateEvaluation;
	Tree* mTree;
	s3d::legacy::TimerMillisec	mTimerMilliSec;
	const unsigned int mDurationMilliSec		= 4000;
	const unsigned int mDurationMilliSecMargin	=  100;
};


class TreeSiv3D : public Tree
{
public:
	TreeSiv3D(Board* board) : Tree(board), mEvaluator(this)
	{
		mOffsetX = WINDOW_W * 0.75f;
		mOffsetY = WINDOW_H * 0.5f;
		mNodeRadius = 8.f;
		mGridScale = 40.f;
		mFont = Font(10, L"メイリオ");
		mFontScore = Font(10, L"Segoe WP Black");
		mGui = GUI(GUIStyle::Default);
		mGui.setTitle(L"メニュー");
		mGui.add(L"kifu_load", GUIButton::Create(L"定跡ファイルを開く"));
		mGui.add(L"kifu_save", GUIButton::Create(L"定跡ファイルを保存"));
		mGui.add(L"evaluator_load", GUIButton::Create(L"評価ソフトを開く"));

		mGuiNode = GUI(GUIStyle::Default);
		mGuiNode.setPos(0, 570);
		mGuiNode.addln(L"comment", GUITextField::Create(30));
		mGuiNode.addln(L"score", GUIText::Create(L"", 640));
		mGuiNode.addln(L"tejunJap", GUIText::Create(L"", 640));
	}

	virtual void Draw() override;
	virtual void Update() override;
	virtual void OnSelectedNodeIDChanged() override;


private:
	TreeSiv3D()	{};

	float ScaleX(float x) const
	{
		return mOffsetX + x * mGridScale;
	}

	float ScaleY(float y) const
	{
		return mOffsetY + y * mGridScale;
	}

	float InvScaleX(float scaledX) const
	{
		assert(mGridScale > 0.f);
		return (scaledX - mOffsetX) / mGridScale;
	}

	float InvScaleY(float scaledY) const
	{
		assert(mGridScale > 0.f);
		return (scaledY - mOffsetY) / mGridScale;;
	}

	void DrawScoreBar(int score, int maxScore, float cx, float cy, float w, float h);

	Font mFont;
	Font mFontScore;
	GUI mGui;
	GUI mGuiNode;
	
	float mOffsetX;
	float mOffsetY;
	float mNodeRadius;
	float mGridScale;

	Evaluator	mEvaluator;
};

