#include "stdafx.h"
#include "GameFramework.h"
#include "Mesh.h"
#include "GameObject.h"

void CGameFramework::OnCreate()
{
	cout << "������ ��ũ ȣ��!" << endl;

	BuildObjects();
}

void CGameFramework::FrameAdvance()
{
	//cout << "������ ���� �� . . ." << endl;
}

void CGameFramework::BuildObjects()
{
	CGameObject* player = new CGameObject();
	player->LoadMeshFromFile("player model data.bin");
}
