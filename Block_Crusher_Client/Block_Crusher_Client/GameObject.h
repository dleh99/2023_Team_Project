#pragma once
#include "Mesh.h"
#include "Camera.h"

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
	XMFLOAT4X4 m_xmf4x4World;
	CMesh* m_pMesh = NULL;
	CShader* m_pShader = NULL;
	float m_fBlockBoundingRadius = sqrt(144.f * 3.f) / 2;

	char m_pstrFrameName[64];

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

	// ��� ���۸� �����Ѵ�.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// ��� ������ ������ �����Ѵ�.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// ���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	float GetBoundingRadius() { return m_fBlockBoundingRadius; };

	// ���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void SetScale(float scale);

	// ���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void moveForward(float fDistance = 1.0f);

	// BS-> Bounding Sphere �ٿ�� ���Ǿ�� �浹üũ
	void BSCollisionCheck(XMFLOAT3 Position, float Radius) {};

	// ���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void SetChild(CGameObject* pChild);
	CGameObject* GetParent() { return m_pParent; }
	CGameObject* FindFrame(char* pstrFrameName);
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
	};
	virtual ~CBulletObject() {};

private:
	int m_iBulletType;
	int m_iBulletDamage;
	float m_fSpeed;
	XMFLOAT3 m_xmf3Vector;

public:
	int GetBulletType() { return m_iBulletType; };
	int GetBulletDamage() { return m_iBulletDamage; };

	void SetBulletVector(XMFLOAT3 vector) { m_xmf3Vector = vector; };
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