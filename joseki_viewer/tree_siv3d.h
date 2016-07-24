#pragma once

#include <Siv3D.hpp>
#include <vector>
#include "tree.h"

class TreeSiv3D : public Tree
{
public:
	virtual ~TreeSiv3D() override {};
	virtual void Draw() override;
	virtual void Update() override;
};

