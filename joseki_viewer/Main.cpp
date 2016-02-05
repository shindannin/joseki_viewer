
// AIはつくりません！


//--- 将棋盤 ---
// 済) 駒を動かす
// 済) 駒を取る
// 済) 駒が成る
// 済) 持ち駒を貼る

//--- 将棋ルール （やらないかも）---
// 駒の移動範囲チェック
// 二歩
// 千日手・詰みは後で（やらないかも）

//--- Aperyとの連携で、評価値をとれるようにする---
// 現在の盤面をSFEN方式で
// 初期盤面＋棋譜

//--- 棋譜のデータ構造
// 分岐を入れる
// セーブ・ロード
// 木のグラフを表示

//--- ビジュアライザ
// 画面の拡縮
// ノード（局面）とエッジ（手）の、それぞれにコメントや固有データ





#define NO_S3D_USING // using namespace s3dを無視。これがないとOpenCVのint64とかぶる	

#define _CRT_SECURE_NO_WARNINGS

#include <Siv3D.hpp>

#include <iostream>

#include <vector>
#include <string>
#include <assert.h>
#include <ctype.h>

#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

PROCESS_INFORMATION piProcInfo;

void CreateChildProcess(void);
void WriteToPipe(void);
void ReadFromPipe(void);
void ErrorExit(PTSTR);

int testMain(int argc, TCHAR *argv[])
{
	SECURITY_ATTRIBUTES saAttr;

	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit(TEXT("StdoutRd CreatePipe"));

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit(TEXT("Stdin CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdin SetHandleInformation"));

	// Create the child process. 

	CreateChildProcess();

	// Get a handle to an input file for the parent. 
	// This example assumes a plain text file and uses string output to verify data flow. 

	if (argc == 1)
		ErrorExit(TEXT("Please specify an input file.\n"));

	g_hInputFile = CreateFile(
		argv[1],
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);

	if (g_hInputFile == INVALID_HANDLE_VALUE)
		ErrorExit(TEXT("CreateFile"));

	// Write to the pipe that is the standard input for a child process. 
	// Data is written to the pipe's buffers, so it is not necessary to wait
	// until the child process is running before writing data.

	WriteToPipe();
	printf("\n->Contents of %s written to child STDIN pipe.\n", argv[1]);

	// Read from pipe that is the standard output for child process. 

	printf("\n->Contents of child process STDOUT:\n\n", argv[1]);
	ReadFromPipe();

	printf("\n->End of parent execution.\n");

	// The remaining open handles are cleaned up when this process terminates. 

	// To avoid resource leaks in a larger application, close handles explicitly. 

	if (TerminateProcess(piProcInfo.hProcess, 0))
	{
		printf("Succeeded.\n");
	}
	else
	{
		printf("Failed.\n");
	}

	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);


	return 0;
}

void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	TCHAR szCmdline[] = TEXT("apery_nosse.exe");
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

					   // If an error occurs, exit the application. 
	if (!bSuccess)
		ErrorExit(TEXT("CreateProcess"));
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

//		CloseHandle(piProcInfo.hProcess);
//		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;

	for (;;)
	{
		bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}

	// Close the pipe handle so the child process stops reading. 

	if (!CloseHandle(g_hChildStd_IN_Wr))
		ErrorExit(TEXT("StdInWr CloseHandle"));
}

void ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(hParentStdOut, chBuf,
			dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}
}

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

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

			m_connected = !!::CreateProcessW(path.c_str(), L"", nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, L"./", &si, &m_pi);
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

using namespace std;

using namespace s3d;

template<class T> bool INRANGE(T x, T a, T b) { return a <= x&&x <= b; }
#define SZ(a) ((int)a.size())
const int NG = -1;

enum ESengo
{
	E_SEN,
	E_GO,
	NUM_SEN_GO,
};



//先手の玉：K、後手の玉：k （Kingの頭文字）
//先手の飛車：R、後手の飛車：r （Rookの頭文字）
//先手の角：B、後手の角：b （Bishopの頭文字）
//先手の金：G、後手の金：g （Goldの頭文字）
//先手の銀：S、後手の銀：s （Silverの頭文字）
//先手の桂馬：N、後手の桂馬：n （kNightより）
//先手の香車：L、後手の香車：l （Lanceの頭文字）
//先手の歩：P、後手の歩：p （Pawnの頭文字）

enum EKomaType
{
	E_OU,
	E_HI,
	E_KAKU,
	E_KIN,
	E_GIN,
	E_KEI,
	E_KYO,
	E_FU,
	NUM_NARAZU_KOMA_TYPE,

	E_RYU = NUM_NARAZU_KOMA_TYPE,
	E_UMA,
	E_NGIN,
	E_NKEI,
	E_NKYO,
	E_TO,
	NUM_KOMA_TYPE,

	E_EMPTY = -1,
};

enum EInputState
{
	E_IDLE,
	E_GRABBED,
	E_HARU,
	E_CHOICE,
};

struct Masu
{
	Masu() : type(E_EMPTY) , sengo(E_SEN) {}
	EKomaType type;
	ESengo sengo;
};


const int BOARD_SIZE = 9;

struct Koma
{
	string notation[NUM_SEN_GO];
	wstring fileName[NUM_SEN_GO];
	EKomaType narigoma;
	EKomaType motogoma;
};

vector <Koma> koma =
{
	{ { "K", "k" },   {L"Sou", L"Gou"}, E_EMPTY, E_OU },
	{ { "R", "r" },   {L"Shi", L"Ghi"}, E_RYU  , E_HI },
	{ { "B", "b" },   {L"Skaku", L"Gkaku"}, E_UMA, E_KAKU },
	{ { "G", "g" },   {L"Skin", L"Gkin"}, E_EMPTY, E_KIN },
	{ { "S", "s" },   {L"Sgin", L"Ggin"}, E_NGIN, E_GIN },
	{ { "N", "n" },   {L"Skei", L"Gkei"}, E_NKEI, E_KEI },
	{ { "L", "l" },   {L"Skyo", L"Gkyo"}, E_NKYO, E_KYO },
	{ { "P", "p" },   {L"Sfu", L"Gfu"}, E_TO, E_FU },

	{ { "+R", "+r" }, {L"Sryu", L"Gryu"}, E_EMPTY, E_HI },
	{ { "+B", "+b" }, {L"Suma", L"Guma"}, E_EMPTY, E_KAKU },
	{ { "+S", "+s" }, {L"Sngin", L"Gngin"}, E_EMPTY, E_GIN },
	{ { "+N", "+n" }, {L"Snkei", L"Gnkei"}, E_EMPTY, E_KEI },
	{ { "+L", "+l" }, {L"Snkyo", L"Gnkyo"}, E_EMPTY, E_KYO },
	{ { "+P", "+p" }, {L"Sto", L"Gto"}, E_EMPTY, E_FU },
};



void split1(const string& str, vector<string>& out, const char splitter = ' ')
{
	out.clear();
	string::size_type st = 0;
	string::size_type next = 0;
	string tmp = str;
	do
	{
		next = tmp.find(splitter, st); // tmp.find_first_of("+-",st); 複数の文字で分けたいとき
		string word = tmp.substr(st, next - st);
		if (word.length() >= 1) // 空文字列ありのときは消す
		{
			out.push_back(word);
		}
		st = next + 1;
	} while (next != string::npos);
}

struct GridPos
{
	GridPos() : x(0), y(0) {}
	bool operator==(const GridPos & n) const
	{
		return this->x == n.x && this->y == n.y;
	}

	bool operator!=(const GridPos & n) const
	{
		return !(n==*this);
	}
	int x;
	int y;
};

struct Move
{
	GridPos from;
	GridPos to;
	bool naru;
};

class Board
{
public:
	Board()
	{
		for (int s = 0; s < NUM_SEN_GO; ++s)
		{
			mTexture[s].clear();
			for (int k = 0; k < NUM_KOMA_TYPE; ++k)
			{
				String fileName(L"pictures/");
				fileName += koma[k].fileName[s];
				fileName += L".png";
				mTexture[s].push_back(Texture(fileName));
			}
		}
		mKomaTextureWidth = mTexture[0][0].width;
		mKomaTextureHeight = mTexture[0][0].height;

		mTextureBoard = Texture(L"pictures/ban_kaya_a.png");
		mTextureGrid = Texture(L"pictures/masu_dot.png");

		mBoardTextureWidth = mTextureBoard.width;
		mBoardTextureHeight = mTextureBoard.height;

		mFont = Font(15, L"メイリオ");
		SetSFEN("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");

		mOffsetX = 0;
		mOffsetY = 0;
	}

	bool IsTekijin(int y, ESengo teban) const
	{
		if (teban == E_SEN && INRANGE(y, 0, 2)) return true;
		if (teban == E_GO  && INRANGE(y, BOARD_SIZE - 3, BOARD_SIZE-1)) return true;

		return false;
	}

	bool IsBanjyo(const GridPos& gp) const
	{
		return IsBanjyo(gp.y,gp.x);
	}

	bool IsBanjyo(int y, int x) const
	{
		return INRANGE(y, 0, BOARD_SIZE - 1) && INRANGE(x, 0, BOARD_SIZE - 1);
	}



	bool IsNareru(const GridPos& from, const GridPos& to, ESengo teban) const
	{
		const EKomaType k = mGrid[from.y][from.x].type;

		// 盤上じゃないとダメ
		if (!IsBanjyo(from)) return false;
		if (!IsBanjyo(to)) return false;

		// つかんでいる駒がなれる駒でないとダメ
		if (koma[k].narigoma == E_EMPTY)
		{
			return false;
		}

		// 移動元か移動先が敵陣でないとダメ
		if (!IsTekijin(from.y, teban) && !IsTekijin(to.y, teban))
		{
			return false;
		}

		// TODO : 移動可能

		return true;
	}

	bool GetGridPosFromMouse(GridPos& gridPos) const
	{
		const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
		const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
		const int x = BOARD_SIZE - 1 - (Mouse::Pos().x - leftX) / mKomaTextureWidth;
		const int y = (Mouse::Pos().y - topY) / mKomaTextureHeight;

		if (!(INRANGE(x, 0, BOARD_SIZE - 1) && INRANGE(y, 0, BOARD_SIZE - 1)))
		{
			return false;
		}

		// こまだいチェックはあとで実装！
		gridPos.x = x;
		gridPos.y = y;
		return true;
	}

	bool GetHarukomaType(EKomaType& harukomaType) const
	{
		int y, x;
		getXYNaruNarazuChoice(y, x);
		for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
		{
			if (mMochigoma[mTeban][k] >= 1)
			{
				int my, mx;
				GetMochigomaPos(mTeban, k, my, mx);
				if (my == y && mx == x)
				{
					harukomaType = static_cast<EKomaType>(k);
					return true;
				}
			}
		}
		
		return false;
	}

	void getXYNaruNarazuChoice(int& y, int& x) const
	{
		const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
		const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

		if (Mouse::Pos().x - leftX >= 0)
		{
			x = BOARD_SIZE - 1  - static_cast<int>(Mouse::Pos().x - leftX) / mKomaTextureWidth;
		}
		else
		{
			x = BOARD_SIZE + static_cast<int>(leftX - Mouse::Pos().x) / mKomaTextureWidth;
		}

		y = (Mouse::Pos().y - topY) / mKomaTextureHeight;
	}

	void getXYNaruNarazuChoice(float& y, float& x) const
	{
		const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
		const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

		if (Mouse::Pos().x - leftX >= 0)
		{
			x = BOARD_SIZE - static_cast<float>(Mouse::Pos().x - leftX) / mKomaTextureWidth;
		}
		else
		{
			x = BOARD_SIZE + static_cast<float>(leftX - Mouse::Pos().x) / mKomaTextureWidth;
		}

		y = (Mouse::Pos().y - topY) / mKomaTextureHeight;
	}

	bool IsNaruChoice() const
	{
		float y, x;
		getXYNaruNarazuChoice(y, x);

		float cy = mMoveToPos.y;
		float cx = mMoveToPos.x;
		NariOffsetY(cy);

		return (INRANGE(x, cx - 0.5f, cx + 0.5f) && INRANGE(y, cy, cy + 1.0f));
	}

	bool IsNarazuChoice() const
	{
		float y, x;
		getXYNaruNarazuChoice(y, x);

		float cy = mMoveToPos.y;
		float cx = mMoveToPos.x;
		NariOffsetY(cy);

		return (INRANGE(x, cx + 0.5f, cx + 1.5f) && INRANGE(y, cy, cy + 1.0f));
	}

	void DoMove(bool isNaru)
	{
		// 移動先に駒がある場合は取る
		if (mGrid[mMoveToPos.y][mMoveToPos.x].type != E_EMPTY)
		{
			const int s = 1 - mGrid[mMoveToPos.y][mMoveToPos.x].sengo;
			const int k = koma[mGrid[mMoveToPos.y][mMoveToPos.x].type].motogoma;
			mMochigoma[s][k]++;
		}

		// 移動する
		{
			mGrid[mMoveToPos.y][mMoveToPos.x] = mGrid[mGrabbedPos.y][mGrabbedPos.x];
			if (isNaru)
			{
				mGrid[mMoveToPos.y][mMoveToPos.x].type = koma[mGrid[mMoveToPos.y][mMoveToPos.x].type].narigoma;
			}

			mGrid[mGrabbedPos.y][mGrabbedPos.x].type = E_EMPTY;
		}

		mInputState = E_IDLE;
		mTeban = static_cast<ESengo>(1 - mTeban);
	}

	void Update()
	{
		if (Input::MouseL.clicked)
		{
			switch (mInputState)
			{
			case E_IDLE:
				{
					// 駒をつかむ
					GridPos gp;
					int s, k;
					if (GetGridPosFromMouse(gp))
					{
						// 自分の駒だけ！
						if (mGrid[gp.y][gp.x].sengo == mTeban && mGrid[gp.y][gp.x].type!=E_EMPTY)
						{
							mGrabbedPos = gp;
							mInputState = E_GRABBED;
						}
					}
					else if (GetHarukomaType(mHaruKomaType))
					{
						mInputState = E_HARU;
					}
				}
				break;

			case E_GRABBED:
				{
					GridPos gp;
					if (GetGridPosFromMouse(gp) && gp!=mGrabbedPos)
					{
						// TODO : 合法手のチェック
						// 成りのチェック
						mMoveToPos = gp;
						if (IsNareru(mGrabbedPos, gp, mTeban))
						{
							// 成り選択画面へ
							mInputState = E_CHOICE;
						}
						else
						{
							DoMove(false);
						}
					}


					// TODO:変なところで駒を離したらキャンセル
				}
				break;

			case E_HARU:
				{
					GridPos gp;
					if (GetGridPosFromMouse(gp) && mGrid[gp.y][gp.x].type == E_EMPTY)
					{
						mMoveToPos = gp;

						mMochigoma[mTeban][mHaruKomaType]--;
						mGrid[mMoveToPos.y][mMoveToPos.x].sengo = mTeban;
						mGrid[mMoveToPos.y][mMoveToPos.x].type = mHaruKomaType;

						mInputState = E_IDLE;
						mTeban = static_cast<ESengo>(1 - mTeban);
					}
				}
				break;

			case E_CHOICE:
				// 成り成らずを選択する
				if (IsNaruChoice() || IsNarazuChoice())
				{
					DoMove(IsNaruChoice()); 
				}
				break;

			default:
				break;
			}
		}
		else if (Input::MouseR.clicked)
		{
			switch (mInputState)
			{
			case E_IDLE:
				break;

			case E_HARU:
			case E_GRABBED:
			case E_CHOICE:
				mInputState = E_IDLE;
				mHaruKomaType = E_EMPTY;
				break;

			default:
				break;
			}
		}
		else if ((Input::KeyControl + Input::KeyV).clicked)
		{
			// 盤面をペースト
			wstring ws = Clipboard::GetText().str();
			string s(ws.begin(), ws.end());
			vector <string> vs;
			split1(s, vs, '\n');

			const string header("[SFEN");
			for (int i = 0; i < SZ(vs); ++i)
			{
				const string& line = vs[i];
				if (line.substr(0, SZ(header)) == header)
				{
					int st = NG;
					for (int k = SZ(header); k < SZ(line); ++k)
					{
						if (st == NG && line[k] == '"')
						{
							st = k + 1;
						}
						else if (line[k] == '"')
						{
							const int ed = k;
							const int len = ed - st;

							SetSFEN(line.substr(st, len));
							break;
						}
					}
					assert(st != NG);
				}
			}
		}
	}

	void SetSFEN(const string& sfen)
	{
		mInputState = E_IDLE;

		vector <string> splitted;
		split1(sfen, splitted);

		// 盤面の設定
		{
			const string& banmen = splitted[0];

			int y = 0;
			int x = 0;
			int i = 0;

			mGrid = vector < vector <Masu> >(BOARD_SIZE, vector <Masu>(BOARD_SIZE));
			while (i < SZ(banmen))
			{
				if (banmen[i] == '/')
				{
					y++;
					x = 0;
					i++;
				}
				else if (INRANGE(banmen[i], '0', '9'))
				{
					x += banmen[i] - '0';
					i++;
				}
				else
				{
					for (int s = 0; s < NUM_SEN_GO; ++s)
					{
						for (int k = 0; k < NUM_KOMA_TYPE; ++k)
						{
							const string& nt = koma[k].notation[s];
							if (banmen.substr(i, SZ(nt)) == nt)
							{
								mGrid[y][BOARD_SIZE - 1 - x].type = static_cast<EKomaType>(k);
								mGrid[y][BOARD_SIZE - 1 - x].sengo = static_cast<ESengo>(s);
								x += 1;
								i += SZ(nt);
								goto NUKE;
							}
						}
					}
				}

			NUKE:;
			}
		}

		// 手番の設定
		// 次の手番については、先手番ならb、後手番ならwと表記します。（Black、Whiteの頭文字）
		mTeban = E_SEN;
		if (splitted[1] == "b")
		{
			mTeban = E_SEN;
		}
		else if (splitted[1] == "w")
		{
			mTeban = E_GO;
		}
		else
		{
			assert(0);
		}

		// 持ち駒の設定
		{
			for (int s = 0; s < NUM_SEN_GO; ++s)
			{
				mMochigoma[s] = vector <int>(NUM_NARAZU_KOMA_TYPE);
			}

			const string& line = splitted[2];

			// 持ち駒については、先手後手のそれぞれの持ち駒の種類と、その枚数を表記します。
			// 枚数は、２枚以上であれば、駒の種類の前にその数字を表記します。
			// 先手側が銀１枚歩２枚、後手側が角１枚歩３枚であれば、S2Pb3pと表記されます。
			// どちらも持ち駒がないときは - （半角ハイフン）を表記します。
			if (line != "-")
			{
				int maisu = 1;
				for (int i = 0; i < SZ(line); ++i)
				{
					const char c = line[i];
					if (isupper(c) || islower(c))
					{
						ESengo s = E_SEN;
						if (islower(c))
						{
							s = E_GO;
						}

						for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
						{
							const string& nt = koma[k].notation[s];
							if (c == nt[0])
							{
								mMochigoma[s][k] = maisu;
								break;
							}
						}
						maisu = 1;
					}
					else if (isdigit(c))
					{
						maisu = c - '0';
					}
					else
					{
						assert(0);
					}
				}
			}
		}
	}
		
	void SetOffset(int offsetX, int offsetY)
	{
		mOffsetX = offsetX;
		mOffsetY = offsetY;
	}

	void DrawKoma(const Masu& masu, int y, int x) const
	{
		DrawKoma(masu.sengo, masu.type, y, x);
	}

	template <class T>
	void NariOffsetY(T& y) const
	{
		if (y <= BOARD_SIZE / 2)
		{
			y++;
		}
		else
		{
			y--;
		}
	}

	void DrawKoma(int sengo, int type, int y, int x, int maisu=0, bool isChoice=false ) const
	{
		const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
		const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;

		if (isChoice)
		{
			const EKomaType nariType = koma[type].narigoma;

			NariOffsetY(y);

			{
				const int drawX = leftX + (BOARD_SIZE - 1.5f - x)*mKomaTextureWidth;
				const int drawY = topY + y*mKomaTextureHeight;
				Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw({ 128, 128, 0, 192 });
				mTexture[sengo][type].draw(drawX, drawY);
			}

			{
				const int drawX = leftX + (BOARD_SIZE - 0.5f - x)*mKomaTextureWidth;
				const int drawY = topY + y*mKomaTextureHeight;
				Rect(drawX, drawY, mKomaTextureWidth, mKomaTextureHeight).draw({ 255, 0, 0, 192 });
				mTexture[sengo][nariType].draw(drawX, drawY);
			}
		}
		else
		{
			mTexture[sengo][type].draw(leftX + (BOARD_SIZE - 1 - x)*mKomaTextureWidth, topY + y*mKomaTextureHeight);
		}

		if (maisu >= 1)
		{
			mFont(L"", maisu).draw(leftX + (BOARD_SIZE - 1 - x + 1)*mKomaTextureWidth, topY + y*mKomaTextureHeight);
		}
	}


	void GetMochigomaPos(int s, int k, int& y, int& x) const
	{
		const int posX[2] = { -2, 10 };
		const int posY[2] = { 1,  7 };
		const int signY[2] = { +1, -1 };

		y = k*signY[s] + posY[s];
		x = posX[s];
	}

	void Draw()
	{
		mTextureBoard.draw(mOffsetX, mOffsetY);
		mTextureGrid.draw(mOffsetX, mOffsetY);

		const int leftX = (mBoardTextureWidth - mKomaTextureWidth * BOARD_SIZE) / 2 + mOffsetX;
		const int topY = (mBoardTextureHeight - mKomaTextureHeight * BOARD_SIZE) / 2 + mOffsetY;
		const int tebanOffestX = 10;

		//----- カーソル -----
		// 現在
		{
			GridPos gp;
			if (GetGridPosFromMouse(gp))
			{
				Rect(leftX + (BOARD_SIZE - 1 - gp.x) * mKomaTextureWidth, topY + gp.y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw({ 128, 0, 0, 127 });
			}
		}

		if (mInputState == E_GRABBED || mInputState == E_CHOICE)
		{
			// つかんだ位置
			Rect(leftX + (BOARD_SIZE - 1 - mGrabbedPos.x) * mKomaTextureWidth, topY + mGrabbedPos.y* mKomaTextureHeight, mKomaTextureWidth, mKomaTextureHeight).draw({ 255, 0, 0, 127 });
		}

		//----- 持ち駒 -----
		{
			for (int s = 0; s < NUM_SEN_GO; ++s)
			{
				for (int k = 0; k < NUM_NARAZU_KOMA_TYPE; ++k)
				{
					int numKoma = mMochigoma[s][k];
					if (mInputState == E_HARU && s == mTeban && mHaruKomaType == k)
					{
						numKoma--;
					}

					if (numKoma >= 1)
					{
						int y, x;
						GetMochigomaPos(s, k, y, x);
						DrawKoma(s, k, y, x, numKoma);
					}
				}
			}
		}

		//----- 手番 -----
		{
			wstring name[2] = { L"先手番", L"後手番" };
			mFont(name[mTeban]).draw(leftX + tebanOffestX * mKomaTextureWidth, topY);
		}

		//----- 盤面 -----
		for (int y = 0; y < BOARD_SIZE; ++y)
		{
			for (int x = 0; x < BOARD_SIZE; ++x)
			{
				if (mInputState == E_GRABBED || mInputState == E_CHOICE)
				{
					if (y == mGrabbedPos.y && x == mGrabbedPos.x)
					{
						continue;
					}
				}

				if (mGrid[y][x].type != E_EMPTY )
				{
					DrawKoma(mGrid[y][x], y, x);
				}
			}
		}

		// つかんだ駒
		if (mInputState == E_GRABBED || mInputState == E_CHOICE)
		{
			const Masu& masu = mGrid[mGrabbedPos.y][mGrabbedPos.x];
			mTexture[masu.sengo][masu.type].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
		}
		else if (mInputState == E_HARU)
		{
			mTexture[mTeban][mHaruKomaType].draw(Mouse::Pos().x - mKomaTextureWidth / 2, Mouse::Pos().y - mKomaTextureHeight / 2);
		}
		 
		// 成り選択画面
		if (mInputState == E_CHOICE)
		{
			const Masu& grabbed	= mGrid[mGrabbedPos.y][mGrabbedPos.x];
			DrawKoma(grabbed.sengo, grabbed.type, mMoveToPos.y, mMoveToPos.x, 0, true);
		}

	}

private:

	// 文字列分割
	// 空文字列ありのときも考慮すること。

	vector < vector <Masu> > mGrid;
	vector <int> mMochigoma[NUM_SEN_GO];
	ESengo mTeban;
	vector <Move> mMoves;

	EInputState mInputState = E_IDLE;
	GridPos mGrabbedPos;
	GridPos mMoveToPos;
	EKomaType mHaruKomaType = E_EMPTY;

	vector <Texture> mTexture[NUM_SEN_GO];
	Texture mTextureBoard;
	Texture mTextureGrid;
	Font mFont;
	int mOffsetX;
	int mOffsetY;
	int mKomaTextureWidth;
	int mKomaTextureHeight;
	int mBoardTextureWidth;
	int mBoardTextureHeight;
};




void Main()
{
//	TCHAR* lpszClientPath[] = { TEXT(""), TEXT("hoge.txt") };
//	testMain(2, lpszClientPath);



/* サーバサンプル
	///////////////////////////////////
	//
	// クライアントを起動
	//
	const auto path = Dialog::GetOpen({ { L"実行ファイル (*.exe)", L"*.exe" } });

	if (!path)
	{
		return;
	}

	Server server(path.value(), false);

	if (!server)
	{
		return;
	}
	//
	///////////////////////////////////

	while (System::Update())
	{
		if (Input::KeyA.clicked)
		{
			Print(L"usi");

			server.write("usi\n");
		}

		if (server.available())
		{
			std::string str;

			server.read(str);

//			Print(str);
		}
	}

	server.write("quit\n");
*/

	// ウィンドウサイズを 幅 300, 高さ 400 にする
	Window::Resize(640, 480);

	const Font font(30);


	// ID: 0 の Web カメラを起動
	/*
	if (!Webcam::Start(0))
	{
		return;	// 失敗したら終了
		}
	*/

	// 空の動的テクスチャ
	DynamicTexture texture;
	Image img;

	Board* board = new Board();

	board->SetOffset(100, 0);
	while (System::Update())
	{
		Circle(Mouse::Pos(), 50).draw({ 255, 0, 0, 127 });

		board->Update();
		board->Draw();


/*
		// カメラが新しい画像を撮影したら
		if (Webcam::HasNewFrame())
		{

			// 盤の位置を指定する
			// 盤の画像を9*9のに切り出し、正しい形に修正する
			// 切り取った画像からSURF値

			// 正しい盤面（GroundTruth）の入力ツールを作る。
			// ランダムフォレストで学習させる

			// 動的テクスチャをカメラの画像で更新
			Webcam::GetFrame(texture);
			Webcam::GetFrame(img);

			// 切り取った画像→OpenCV

			// OpenCV→画像


			// 画像→OpenCV


 			// (3)initialize SURF class
		}

		if (texture)
		{
			texture.draw();	// テクスチャを描く
		}
*/

	}
	
	delete board;
}


