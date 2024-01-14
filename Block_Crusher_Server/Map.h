#pragma once
#include "GameObject.h"

class Map
{
public:
	char				MapChar;
	int					block_num;
	std::atomic_int		player_num;
	bool				isActive;
public:
	std::vector<Block> Map_Block;

public:
	Map();
	~Map();
};