#pragma once
#include "GameObject.h"

class Map
{
public:
	char	MapChar;
	int		block_num;
public:
	std::vector<Block> Map_Block;

public:
	Map();
	~Map();
};