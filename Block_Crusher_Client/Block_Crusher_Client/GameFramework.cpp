#include "stdafx.h"
#include "GameFramework.h"
#include "Mesh.h"
#include "GameObject.h"

void CGameFramework::OnCreate()
{
	cout << "프레임 워크 호출!" << endl;

	BuildObjects();
}

void CGameFramework::FrameAdvance()
{
	//cout << "프레임 진행 중 . . ." << endl;
}

void CGameFramework::BuildObjects()
{
	CGameObject* player = new CGameObject();
	player->LoadMeshFromFile("player model data.bin");
}
