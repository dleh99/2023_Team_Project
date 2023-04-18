#pragma once
#include "Mesh.h"

class CGameObject
{
protected:
	UINT m_nMesh = 0;
	CMesh* m_MeshList = NULL;

public:
	int LoadMeshFromFile(string fileName);
};

