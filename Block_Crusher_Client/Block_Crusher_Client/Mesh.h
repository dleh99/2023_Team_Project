#pragma once

#include "GameObject.h"

class CGameObject;

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

class CVertex
{
protected:
	XMFLOAT3 m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {  }

	XMFLOAT3 GetPosition() { return m_xmf3Position; }
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
	XMFLOAT3						m_xmf3Normal;

public:
	CTexturedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	CTexturedVertex(float x, float y, float z, XMFLOAT2 xmf2TexCoord) {
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf2TexCoord = xmf2TexCoord;
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	CTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2TexCoord = XMFLOAT2(0.0f, 0.0f)) { 
		m_xmf3Position = xmf3Position;
		m_xmf2TexCoord = xmf2TexCoord;
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	CTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal, XMFLOAT2 xmf2TexCoord = XMFLOAT2(0.0f, 0.0f)) {
		m_xmf3Position = xmf3Position;
		m_xmf2TexCoord = xmf2TexCoord;
		m_xmf3Normal = xmf3Normal;
	}
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

public:
	char					m_pstrMeshName[64] = { 0, };

protected:
	UINT					m_nType = 0x00;

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

	XMFLOAT3* m_pxmf3Positions = NULL;

	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

public:
	UINT GetType() { return m_nType; }

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const D3D12_VERTEX_BUFFER_VIEW& instanceBufferView);
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
protected:
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

#define SKINNED_ANIMATION_BONES		256

class CSkinnedMesh : public CPlayerMesh
{
public:
	CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CSkinnedMesh();

protected:
	ComPtr<ID3D12Resource>				m_pd3dBoneIndexBuffer = NULL;
	ComPtr<ID3D12Resource>				m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW			m_d3dBoneIndexBufferView;

	ComPtr<ID3D12Resource>				m_pd3dBoneWeightBuffer = NULL;
	ComPtr<ID3D12Resource>				m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW			m_d3dBoneWeightBufferView;

protected:
	int									m_nBonesPerVertex = 4;

	XMINT4*								m_pxmn4BoneIndices = NULL;
	XMFLOAT4*							m_pxmf4BoneWeights = NULL;

public:
	int									m_nSkinningBones = 0;

	char								(*m_ppstrSkinningBoneNames)[64];
	CGameObject**						m_ppSkinningBoneFrameCaches = NULL;

	XMFLOAT4X4*							m_pxmf4x4BindPoseBoneOffsets = NULL;

	ComPtr<ID3D12Resource>				m_pd3dcbBindPoseBoneOffsets = NULL;
	XMFLOAT4X4*							m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL;

	ComPtr<ID3D12Resource>				m_pd3dcbSkinningBoneTransforms = NULL;
	XMFLOAT4X4*							m_pcbxmf4x4MappedSkinningBoneTransforms = NULL;

public:
	void PrepareSkinning(CGameObject* pModelRootObject);
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& fileStream);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};

class CCubeMeshTextured : public CMesh
{
	ComPtr<ID3D12Resource>		m_pd3dNormalBuffer = NULL;
	ComPtr<ID3D12Resource>		m_pd3dNormalUploadBuffer = NULL;

public:
	CCubeMeshTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured() { };

	void CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices);
	void CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices,
		UINT* pnIndices, UINT nIndices);
	void CalculateTriangleStripVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices,
		UINT* pnIndices, UINT nIndices);
	void CalculateVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices,
		UINT* pnIndices, int nIndices);
};

class CSkyBoxMesh : public CMesh
{
public:
	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};