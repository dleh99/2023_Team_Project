#include "stdafx.h"
#include "Mesh.h"

CMesh::~CMesh()
{
	delete[] m_positionList;
	delete[] m_normalList;
	delete[] m_tangentList;
	delete[] m_textureCoordList;
}

int CMesh::LoadMeshFromFile(ifstream& in)
{
	in.read(reinterpret_cast<char*>(&m_nVertex), sizeof(m_nVertex));	// read vertex number
	m_positionList = new XMFLOAT3[m_nVertex];

	for (UINT i = 0; i < m_nVertex; ++i) {								// read vertex
		in.read(reinterpret_cast<char*>(&m_positionList[i]), sizeof(XMFLOAT3));
	}

	in.read(reinterpret_cast<char*>(&m_nVertex), sizeof(m_nVertex));
	m_normalList = new XMFLOAT3[m_nVertex];

	for (UINT i = 0; i < m_nVertex; ++i) {								// read normal
		in.read(reinterpret_cast<char*>(&m_normalList[i]), sizeof(XMFLOAT3));
	}

	in.read(reinterpret_cast<char*>(&m_nVertex), sizeof(m_nVertex));
	m_tangentList = new XMFLOAT3[m_nVertex];

	for (UINT i = 0; i < m_nVertex; ++i) {								// read tangent
		in.read(reinterpret_cast<char*>(&m_tangentList[i]), sizeof(XMFLOAT3));
	}

	in.read(reinterpret_cast<char*>(&m_nVertex), sizeof(m_nVertex));
	m_textureCoordList = new XMFLOAT2[m_nVertex];

	for (UINT i = 0; i < m_nVertex; ++i) {								// read UV0
		in.read(reinterpret_cast<char*>(&m_textureCoordList[i]), sizeof(XMFLOAT2));
	}

	return 0;
}

void CMesh::PrintMeshInformation()
{
	for (UINT i = 0; i < m_nVertex; ++i) {
		cout << "( "
			<< m_positionList[i].x << ", "
			<< m_positionList[i].y << ", "
			<< m_positionList[i].z << " )" << endl;
	}

	for (UINT i = 0; i < m_nVertex; ++i) {
		cout << "( "
			<< m_normalList[i].x << ", "
			<< m_normalList[i].y << ", "
			<< m_normalList[i].z << " )" << endl;
	}

	for (UINT i = 0; i < m_nVertex; ++i) {
		cout << "( "
			<< m_tangentList[i].x << ", "
			<< m_tangentList[i].y << ", "
			<< m_tangentList[i].z << " )" << endl;
	}

	for (UINT i = 0; i < m_nVertex; ++i) {
		cout << "( "
			<< m_textureCoordList[i].x << ", "
			<< m_textureCoordList[i].y << " )" << endl;
	}
}
