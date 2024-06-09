#pragma once
#include "GameObject.h"

constexpr int Map_Width = 50;
constexpr int Map_Depth = 50;
constexpr int Map_Height = 20;

class Map
{
public:
	char				MapChar;
	int					block_num;	
public:
	//std::vector<Block> Map_Block;
	std::vector<Block> Map_B[Map_Width * Map_Depth];
	//Block Map_B[Map_Width * Map_Depth][Map_Height];
	//std::vector<std::vector<Block>> Map_B;

public:
	Map();
	~Map();
	void CreateMap();
	void ClearMap();
};