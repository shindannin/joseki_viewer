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
#include "gui_siv3d.h"
#include "util.h"


using namespace std;
using namespace s3d;

void Main()
{
	Window::SetTitle(VERSION_TITLE);
    Window::SetStyle(WindowStyle::Sizeable);
	Window::Resize(WINDOW_W, WINDOW_H);

	GuiSiv3D* guiSiv3D = new GuiSiv3D();
	Board* board = new BoardSiv3D(*guiSiv3D);
	Tree* tree = new TreeSiv3D(board, *guiSiv3D);

	while (System::Update())
	{
		tree->Update();
		tree->Draw();
	}

	delete tree;
	delete board;
	delete guiSiv3D;
}


