
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

#include "board_siv3d.h"
#include "tree_siv3d.h"


using namespace std;
using namespace s3d;

void Main()
{
	Window::Resize(1280, 720);
	const Font font(30);


	// 空の動的テクスチャ
	DynamicTexture texture;
	Image img;

	Board* board = new BoardSiv3D();
	Tree* tree = new TreeSiv3D(board);

	while (System::Update())
	{
		Circle(Mouse::Pos(), 50).draw({ 255, 0, 0, 127 });

		tree->Update();
		tree->Draw();

		if (Input::KeyEscape.clicked)
		{
			System::Exit();
		}

	}
	
	delete tree;
	delete board;
}


