#pragma once

class CVertex
{
protected:
	XMFLOAT3 m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {  }
};

class CDiffusedVertex :public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf4Diffuse = xmf4Diffuse;
	}

	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position = xmf3Position;
		m_xmf4Diffuse = xmf4Diffuse;
	}

	~CDiffusedVertex() {  }
};

class CTexturedVertex : public CVertex
{
public:
	XMFLOAT2						m_xmf2TexCoord;

public:
	CTexturedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f); }
	CTexturedVertex(float x, float y, float z, XMFLOAT2 xmf2TexCoord) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf2TexCoord = xmf2TexCoord; }
	CTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2TexCoord = XMFLOAT2(0.0f, 0.0f)) { m_xmf3Position = xmf3Position; m_xmf2TexCoord = xmf2TexCoord; }
	~CTexturedVertex() { }
};

class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();

private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0)delete this; }

	void ReleaseUploadBuffers();

protected:
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer = NULL;
	ComPtr<ID3D12Resource> m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

	ComPtr<ID3D12Resource> m_pd3dIndexBuffer = NULL;
	ComPtr<ID3D12Resource> m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_nSlot = 0;
	UINT m_nVertices = 0;
	UINT m_nStride = 0;
	UINT m_nOffset = 0;

	UINT m_nIndices = 0;
	UINT m_nStartIndex = 0;
	int m_nBaseVertex = 0;

public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};

class CMeshLoadInfo
{
public:
	CMeshLoadInfo() { }
	~CMeshLoadInfo();

public:
	int								m_nPositions = 0;
	XMFLOAT3*						m_pxmf3Positions = NULL;

	int								m_nNormals = 0;
	XMFLOAT3*						m_pxmf3Normals = NULL;

	int								m_nTangents = 0;
	XMFLOAT4*						m_pxmf4Tangents = NULL;

	int								m_nUvs = 0;
	XMFLOAT2*						m_pxmf2Uvs = NULL;

	int								m_nIndices = 0;
	UINT*							m_pIndices = NULL;
};

class CTriangleMesh :public CMesh
{
public:
	CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTriangleMesh() {  }
};

class CCubeMeshDiffused : public CMesh
{
public:
	CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshDiffused();
};

class CPlayerMesh :public CMesh
{
	BoundingBox					m_BoundingBox;

	ComPtr<ID3D12Resource>		m_pd3dVertexBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dVertexUploadBuffer = NULL;

	ComPtr<ID3D12Resource>		m_pd3dNormalBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dNormalUploadBuffer = NULL;

	ComPtr<ID3D12Resource>		m_pd3dTangentBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dTangentUploadBuffer = NULL;

	ComPtr<ID3D12Resource>		m_pd3dUvBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dUvUploadBuffer = NULL;

	UINT						m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW*	m_pd3dPlayerVertexBufferViews = NULL;

	ComPtr<ID3D12Resource>		m_pd3dIndexBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dIndexUploadBuffer = NULL;

	D3D12_INDEX_BUFFER_VIEW		m_pd3dPlayerIndexBufferView;

public:
	CPlayerMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CPlayerMesh();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
};

class CCubeMeshTextured : public CMesh
{
public:
	CCubeMeshTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured() { };
};