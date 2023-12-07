#pragma once
#include "Mesh.h"
#include "Camera.h"

#define TYPE_PLAYER 0
#define TYPE_BLOCK 1
#define TYPE_BULLET 2

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	TCHAR m_pstrFrameName[64];

	CMesh* m_pMesh = NULL;
	CShader* m_pShader = NULL;

	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4X4 m_xmf4x4World;	

	float m_fBlockBoundingRadius;
	//| 0 - 주인공 | 1 - 블럭 | 2 - 총알 |
	int m_ObjType = TYPE_BLOCK;
	bool m_bActive = false;

	CGameObject* m_pParent = NULL;
	CGameObject* m_pChild = NULL;
	CGameObject* m_pSibling = NULL;

public:
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void ReleaseUploadBuffers();

	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// 상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// 상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// 게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	int GetObjectType();
	bool GetIsActive();

	float GetBoundingRadius() { return m_fBlockBoundingRadius; };

	// 게임 객체의 워치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float scale);
	void SetScale(float x, float y, float z);
	void SetObjectType(int type);
	void SetIsActive(bool Active);

	// 게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void moveForward(float fDistance = 1.0f);

	// BS-> Bounding Sphere 바운딩 스피어끼리 충돌체크
	void BSCollisionCheck(XMFLOAT3 Position, float Radius) {};

	// 게임 객체를 회전(x-축, y-축, z-축)한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void SetChild(CGameObject* pChild);
	CGameObject* GetParent() { return m_pParent; }
	CGameObject* FindFrame(_TCHAR* pstrFrameName);
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	static CGameObject* LoadHierarchyModelFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, std::ifstream& fileStream);

	static CMeshLoadInfo* LoadMeshInfoFromFile(std::ifstream& fileStream, float modelScaleFactor);
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed);
};

class CBlockObject : public CGameObject
{
public:
	CBlockObject() 
	{
		m_iBlockType = 0; 
		m_iBlockHP = 1;
	};
	virtual ~CBlockObject() {};

private:
	int m_iBlockType;
	int m_iBlockHP;

public:
	int GetBlockType();
	int GetBlockHP() { return m_iBlockHP; };
	void Animate(float fTimeElapsed) {};
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject()
	{
		m_iBulletType = 0;
		m_iBulletDamage = 1;
		m_xmf3Vector = { 0,0,0 };
		m_fSpeed = 200.0f;
		m_fBoundingSph = 2.0f;
		m_fBlockBoundingRadius = 2.0f;
	};
	virtual ~CBulletObject() {};

	void SetPlayerId(int x) { player_id = x; };
	void SetBulletId(int x) { bullet_id = x; };

	int GetPlayerId() { return player_id; };
	int GetBulletId() { return bullet_id; };

private:
	int m_iBulletType;
	int m_iBulletDamage;
	float m_fSpeed;
	float m_fBoundingSph;
	XMFLOAT3 m_xmf3Vector;

	int player_id;
	int bullet_id;


public:
	int GetBulletType() { return m_iBulletType; };
	int GetBulletDamage() { return m_iBulletDamage; };

	void SetBulletVector(XMFLOAT3 vector) { m_xmf3Vector = vector; };
	XMFLOAT3 GetBulletVector() { return m_xmf3Vector; };
	virtual void Animate(float fTimeElapsed);
};

class CTexture
{
private:
	int m_nTextures = 0;
	ComPtr<ID3D12Resource> m_pd3dTexture = NULL;
	UINT m_nRootParameterIndex = -1;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGpuHandle;

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
};