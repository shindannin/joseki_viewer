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
		mPonderNodes = 0LL;
		mPonderTime = 0LL;
	}

	~Evaluator()
	{
		Close();
	}

	void Open();
	void OpenOption();
	void Close();
	void Update();
	void RequestCancel();

	bool IsActive() const { return mServer!=nullptr; }
	bool IsOptionRead() const { return !mOptions.empty(); }
	long long GetPonderNodes() const { return mPonderNodes; }
	long long GetPonderTime() const { return mPonderTime; }
	const string& GetName() const { return mName; }

	int GetDurationSec() const { return mDurationMilliSec/1000; }
	void SetDurationSec(int sec) { mDurationMilliSec = sec * 1000; }

private:
	bool Go();
	void ReceiveBestMoveAndScore();
	void WaitAndCancel();

	int mEvaludatingNodeID;
	Server* mServer;
	EStateEvaluation mEStateEvaluation;
	Tree* mTree;
	Stopwatch mStopwatch;
	vector <string> mOptions;
	long long mPonderNodes;
	long long mPonderTime;
	string mName;

	unsigned int mDurationMilliSec		            = 5000;
	const unsigned int mDurationMilliSecStartMargin = 2000;
	const unsigned int mDurationMilliSecMargin	    =  100;
};


class TreeSiv3D : public Tree
{
public:
	enum EGUICheckBox
	{
		SHOW_SCORE,			// "評価値" 
		SHOW_TE,			// "指し手"
		SHOW_TAG,			// "タグ"
		FIX_SELECTED_NODE,	// "選択ノードの固定"
		SHOW_DEBUG,			// "デバッグ"
	};


	TreeSiv3D(Board* board) : Tree(board), mEvaluator(this)
	{
		mOffsetX = RIGHT_CENTER_X;
		mOffsetY = RIGHT_CENTER_Y;
		mNodeRadius = 8.f;
		mGridScale = 40.f;
		mFont = Font(10, L"メイリオ");
		mFontScore = Font(10, L"Segoe WP Black");
		mFontGuiDefault = GUIManager::GetDefaultFont();

		GUIStyle style = GUIStyle::Default;
		style.font = mFont;
		style.background.color = Color(0, 0, 255, 64);
		style.padding.bottom = 0;
		style.padding.top = 0;
		style.padding.left = 0;
		style.padding.right = 0;

		WidgetStyle widgetStyle;
		widgetStyle.font = mFont;
		widgetStyle.color = Color(255, 255, 255, 255);


		GUIStyle style2 = style;
		style2.background.color = Color(255, 255, 255, 196);

		WidgetStyle widgetStyle2 = widgetStyle;
		widgetStyle2.color = Color(0, 0, 0, 255);

		GUIStyle style3 = style;
		style3.background.color = Color(255, 0, 0, 64);

		mGuiFile = GUI(style2);
//		mGuiFile.setTitle(L"メニュー");
		mGuiFile.addln(L"kifu_load", GUIButton::Create(L"定跡ファイルを開く", widgetStyle2));
		mGuiFile.addln(L"kifu_save", GUIButton::Create(L"定跡ファイルを保存", widgetStyle2));

		mGuiEvaluator = GUI(style3);
		mGuiEvaluator.setPos(200, 0);
		mGuiEvaluator.add(L"evaluator_load", GUIButton::Create(L"評価ソフトを開く", widgetStyle2));
		mGuiEvaluator.addln(L"evaluator_name", GUIText::Create(L"", widgetStyle));
		mGuiEvaluator.add(L"option_load", GUIButton::Create(L"オプションを開く", widgetStyle2));
		mGuiEvaluator.add(L"option_name", GUIText::Create(L"", widgetStyle));
		mGuiEvaluator.add(L"time_prefix", GUIText::Create(L"　　思考時間", widgetStyle));
		mGuiEvaluator.add(L"time_sec", GUITextField::Create(5));
		mGuiEvaluator.addln(L"time_suffix", GUIText::Create(L"秒", widgetStyle));


		String x = CharacterSet::Widen(to_string(mEvaluator.GetDurationSec()));
		mGuiEvaluator.textField(L"time_sec").setText(x);


		mGuiNode = GUI(GUIStyle::Default);
		mGuiNode.setPos(0, 570);
		mGuiNode.addln(L"summary", GUITextField::Create(30));
		mGuiNode.addln(L"comment", GUITextArea::Create(4, 30));

		mGuiScore = GUI(style);
		mGuiScore.setPos(630, 100);
		mGuiScore.addln(L"score", GUIText::Create(L"", widgetStyle));
		mGuiScore.addln(L"tejunJap", GUIText::Create(L"", widgetStyle));
		
		mGuiSettings = GUI(style2);
		mGuiSettings.setPos(1137, 0);
		mGuiSettings.add(L"settings", GUICheckBox::Create({ L"評価値", L"指し手", L"タグ", L"選択ノードの固定", L"デバッグ" }, { SHOW_SCORE, SHOW_TE, SHOW_TAG, FIX_SELECTED_NODE }, true, widgetStyle2));
	}

	virtual void Draw() override;
	virtual void Update() override;
	virtual void OnSelectedNodeIDChanged() override;

	long long GetPonderNodes() const { return mEvaluator.GetPonderNodes();}
	long long GetPonderTime() const { return mEvaluator.GetPonderTime(); }


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
	s3d::RoundRect GetNodeShape(float centerX, float centerY);

	Font mFont;
	Font mFontScore;
	Font mFontGuiDefault;
	GUI mGuiFile;
	GUI mGuiEvaluator;
	GUI mGuiNode;
	GUI mGuiScore;
	GUI mGuiSettings;


	float mOffsetX;
	float mOffsetY;
	float mNodeRadius;
	float mGridScale;

	Evaluator	mEvaluator; // TODO : 評価ソフトなので、treeに移動したほうが良いのでは。ただファイル読み込みとかがちがちにSIV3D使っているので、ちょっと移動は大変かも。
};

