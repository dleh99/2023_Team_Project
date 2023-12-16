#pragma once
#include "stdafx.h"

#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define KEY_SHOOT 0x20

#include "GameObject.h"
#include "Scene.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
protected:
	//�÷��̾��� ��ġ ����, x-��(Right), y-��(Up), z-��(Look) �����̴�.
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//�÷��̾ ���� x-��(Right), y-��(Up), z-��(Look)���� �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����.
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	float m_fKeyDownTime = 9999;
	//
	float m_fEtime = 0;
	//�÷��̾� �߷�����(y��ǥ ����) ����
	bool m_bPlayerGravity = false;
	//�÷��̾� ����Ű
	bool m_bPlayerJump= false;
	//�÷��̾� �߷����� �ð�
	float m_fPlayerGravityTime;
	// �÷��̾� ���� �ӵ�
	XMFLOAT3 xmf3JumpShift;
	// �÷��̾� �ٿ�� ������ ������
	float m_fPlayerBoundingRadius;

	//�÷��̾��� �̵� �ӵ��� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xmf3Velocity;

	//�÷��̾ �ۿ��ϴ� �߷��� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xmf3Gravity;

	//xz-��鿡�� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float m_fMaxVelocityXZ;

	//y-�� �������� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float m_fMaxVelocityY;

	//�÷��̾ �ۿ��ϴ� �������� ��Ÿ����.
	float m_fFriction;

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdateCallback() �Լ����� ����ϴ� �������̴�.
	LPVOID m_pPlayerUpdatedContext;

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdateCallback() �Լ����� ����ϴ� �������̴�.
	LPVOID m_pCameraUpdatedContext;

	//�÷��̾ ���� ������ ī�޶��̴�.
	CCamera *m_pCamera = NULL;

	int m_playerNetworkId = -1;

	bool isRotate = false;

	int bullet_id = 0;

public:
	CGameObject** m_ppObjects = NULL;
	CScene* m_pScene = NULL;

	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	float GetPitch() { return(m_fPitch); }
	float GetYaw() { return(m_fYaw); }
	float GetRoll() { return(m_fRoll); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3 xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }

	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }

	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }

	void SetScale(float scale) {
		/*m_xmf4x4World._11 *= scale;
		m_xmf4x4World._22 *= scale;
		m_xmf4x4World._33 *= scale;*/

		m_xmf3Right.x *= scale;
		m_xmf3Up.y *= scale;
		m_xmf3Look.z *= scale;
	}
	/*�÷��̾��� ��ġ�� xmf3Position ��ġ�� �����Ѵ�.
	xmf3Position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� xmf3Position ���������� ���Ͱ� �ȴ�.
	���� �÷��̾��� ��ġ���� �� ���� ��ŭ �̵��Ѵ�.*/
	void SetPosition(XMFLOAT3 xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),
			false);
	}

	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	int GetPlayerId() { return m_playerNetworkId; }

	//�÷��̾ �̵��ϴ� �Լ��̴�.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Jump(float fTimeElapsed);
	//�÷��̾� �ٿ�����Ǿ� �浹üũ
	bool BSCollisionCheck(XMFLOAT3 Position, float Radius);
	bool SBCollisionCheck(XMFLOAT3 Position);
	XMFLOAT3 SBCollisionMoveXZ(XMFLOAT3 Position, XMFLOAT3 Velocity);

	//�÷��̾ ȸ���ϴ� �Լ��̴�.
	void Rotate(float x, float y, float z);

	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�.
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//ī�޶� �����ϱ� ���Ͽ� ȣ���ϴ� �Լ��̴�.
	CCamera *OnCreateCamera();
	virtual CCamera* CreateCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(NULL);
	}

	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	virtual void OnPrepareRender();

	//�÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾�(�޽�)�� �������Ѵ�.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void SetIsRotate(bool tf);
	bool GetIsRotate();

	void SetBulletId(int x) { bullet_id = x; };
	int GetBulletId() { return bullet_id; };
};

class CCubePlayer : public CPlayer
{
public:
	CCubePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z, int id);
	virtual ~CCubePlayer();

	virtual CCamera* CreateCamera(float fTimeElapsed);
	virtual void OnPrepareRender();
};

class CMainPlayer : public CPlayer
{
public:
	CMainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z);
	virtual ~CMainPlayer();

	virtual CCamera* CreateCamera(float fTimeElapsed);
	virtual void OnPrepareRender();

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	virtual void Update(float fTimeElapsed, ULONG nDirection);

	void OtherPlayerAnimationUpdate(DWORD dwOtherPlayerDirection);
};