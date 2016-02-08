
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
// 通信方法
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
#include <cassert>
#include <ctype.h>

#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#include "board.h"
#include "board_siv3d.h"


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


void Main()
{


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

	Board* board = new BoardSiv3D();

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


