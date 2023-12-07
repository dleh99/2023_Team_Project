#include "Map.h"

Map::Map()
{
	int cnt = 0;
	for (int i{}; i < 10; ++i)
		for (int j{}; j < 10; ++j)
			for (int k{}; k < 10; ++k) {
				float x = -(float)i * 12.f + 20.f;
				float y = -(float)j * 12.f;
				float z = -(float)k * 12.f + 40.f;
				Map_Block[cnt].SetPosition(x, y, z);
				Map_Block[cnt].SetId(cnt);
				cnt++;
			}
	
	for (int i{}; i < 2; ++i)
		for (int j{}; j < 2; ++j)
			for (int k{}; k < 2; ++k) {
				float x = 20.f - 12.f * (i + 4);
				float y = 12.f + 12.f * j;
				float z = 40.f - 12.f * (k + 4);
				Map_Block[cnt].SetPosition(x, y, z);
				Map_Block[cnt].SetId(cnt);
				cnt++;
			}
}

Map::~Map()
{
}