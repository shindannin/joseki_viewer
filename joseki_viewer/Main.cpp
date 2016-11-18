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
#include "util.h"


using namespace std;
using namespace s3d;

void Main()
{
	Window::SetTitle(L"Siv3D 定跡ビューア Ver 0.01 (2016/11/14)");
	Window::Resize(WINDOW_W, WINDOW_H);
	const Font font(30);

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


