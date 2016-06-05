#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "board.h"


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

			m_connected = !!::CreateProcessW(path.c_str(), L"", nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, L"C:/Users/NobuMiu/Documents/shogi/apery_wcsc26/bin/", &si, &m_pi);
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

class BoardSiv3D : public Board
{
public:
	BoardSiv3D();
	virtual ~BoardSiv3D() override;
	virtual void Draw() override;
	virtual void Update() override;
	virtual int CalcBestMoveAndScore() override;

	void DrawCursor(const GridPos& gp, const Color& color) const;
	void DrawKoma(const Masu& masu, int y, int x) const;
	void DrawKoma(int sengo, int type, int y, int x, int maisu = 0, bool isChoice = false) const;
	void SetOffset(int offsetX, int offsetY);
	bool GetGridPosFromMouse(GridPos& gridPos) const;
	void GetXYNaruNarazuChoice(int& y, int& x) const;
	void GetXYNaruNarazuChoice(float& y, float& x) const;
	template <class T> void NariOffsetY(T& y) const;
	void GetMochigomaPos(int s, int k, int& y, int& x) const;
	bool CalcHarukomaType(EKomaType& harukomaType) const;
	bool IsNaruChoice() const;
	bool IsNarazuChoice() const;
	int GetGridLeftX() const;
	int GetGridTopY() const;

private:

	const wstring mFileName[NUM_KOMA_TYPE][NUM_SEN_GO] =
	{
		{ L"Sou", L"Gou" },
		{ L"Shi", L"Ghi" },
		{ L"Skaku", L"Gkaku" },
		{ L"Skin", L"Gkin" },
		{ L"Sgin", L"Ggin" },
		{ L"Skei", L"Gkei" },
		{ L"Skyo", L"Gkyo" },
		{ L"Sfu", L"Gfu" },

		{ L"Sryu", L"Gryu" },
		{ L"Suma", L"Guma" },
		{ L"Sngin", L"Gngin" },
		{ L"Snkei", L"Gnkei" },
		{ L"Snkyo", L"Gnkyo" },
		{ L"Sto", L"Gto" },
	};

	void InitServer();

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
	EInputState mInputState = E_IDLE;
	Server* mServer;
};
