#include "stdafx.h"
#include "GameObject.h"

int CGameObject::LoadMeshFromFile(string fileName)
{
	ifstream in{ fileName,ios::binary };
	if (!in) {
		cout << "Mesh 정보 파일 로드 실패" << endl;
		return -1;
	}

	int nMesh;
	in.read(reinterpret_cast<char*>(&nMesh), sizeof(nMesh));		// read Mesh number

	m_MeshList = new CMesh[nMesh];

	for (int i = 0; i < nMesh; ++i) {
		m_MeshList[i].LoadMeshFromFile(in);
	}

	for (int i = 0; i < nMesh; ++i) {
		m_MeshList[i].PrintMeshInformation();
	}

	return 0;
}
