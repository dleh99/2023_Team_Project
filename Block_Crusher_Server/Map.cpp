#include "Map.h"

Map::Map()
{
	/*int cnt = 0;
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
			}*/

	std::ifstream in{ "Map/MapData3.bin", std::ios::binary };

	srand(static_cast<unsigned int>(time(nullptr)));
	int randomAscii = rand() % 26 + 97;
	MapChar = static_cast<char>(randomAscii);

	int mapdata[50][50];

	int x = 0;
	int y = 0;
	bool flag = false;

	int m_nblock = 0;

	while (in) {
		char c;
		in >> c;

		if (c == MapChar) {
			flag = true;
			continue;
		}

		if (flag) {
			int num = c - 48;

			mapdata[x][y] = num;

			y++;
			if (y >= 50) {
				y = 0;
				x++;
			}
			if (x >= 50) {
				break;
			}

			m_nblock += num;
		}
	}

	block_num = 50 * 50 * 10 + m_nblock;

	int cnt = 0;
	for (int i = 0; i < 50; ++i)
		for (int j = 0; j < 10; ++j)
			for (int k = 0; k < 50; ++k) {
				XMFLOAT3 position = { -(float)i * 12.0f + 20.0f, -(float)j * 12.0f , -(float)k * 12.0f + 40.0f };
				Map_Block.emplace_back(cnt, position);
				cnt++;
			}

	for (int i = 0; i < 50; ++i)
		for (int k = 0; k < 50; ++k) {
			for (int y = 0; y < mapdata[i][k]; ++y) {
				XMFLOAT3 position = { -(float)i * 12.0f + 20.0f, (float)y * 12.0f + 12.0f, -(float)k * 12.0f + 40.0f };
				Map_Block.emplace_back(cnt, position);
				cnt++;
			}
		}
}

Map::~Map()
{
}