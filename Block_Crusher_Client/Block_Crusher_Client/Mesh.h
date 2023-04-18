#pragma once
class CMesh
{
public:
	~CMesh();

protected:
	UINT m_nVertex = 0;
	XMFLOAT3* m_positionList = NULL;
	XMFLOAT3* m_normalList = NULL;
	XMFLOAT3* m_tangentList = NULL;
	XMFLOAT2* m_textureCoordList = NULL;

public:
	int LoadMeshFromFile(ifstream& in);
	void PrintMeshInformation();
};