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
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다.
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다.
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	float m_fKeyDownTime = 9999;
	//
	float m_fEtime = 0;
	//플레이어 중력적용(y좌표 감소) 여부
	bool m_bPlayerGravity = false;
	//플레이어 점프키
	bool m_bPlayerJump= false;
	//플레이어 중력적용 시간
	float m_fPlayerGravityTime;
	// 플레이어 낙하 속도
	XMFLOAT3 xmf3JumpShift;
	// 플레이어 바운딩 볼륨의 반지름
	float m_fPlayerBoundingRadius;

	//플레이어의 이동 속도를 나타내는 벡터이다.
	XMFLOAT3 m_xmf3Velocity;

	//플레이어에 작용하는 중력을 나타내는 벡터이다.
	XMFLOAT3 m_xmf3Gravity;

	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float m_fMaxVelocityXZ;

	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float m_fMaxVelocityY;

	//플레이어에 작용하는 마찰력을 나타낸다.
	float m_fFriction;

	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID m_pPlayerUpdatedContext;

	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID m_pCameraUpdatedContext;

	//플레이어에 현재 설정된 카메라이다.
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
	/*플레이어의 위치를 xmf3Position 위치로 설정한다.
	xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다.
	현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/
	void SetPosition(XMFLOAT3 xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),
			false);
	}

	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	int GetPlayerId() { return m_playerNetworkId; }

	//플레이어를 이동하는 함수이다.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Jump(float fTimeElapsed);
	//플레이어 바운딩스피어 충돌체크
	bool BSCollisionCheck(XMFLOAT3 Position, float Radius);
	bool SBCollisionCheck(XMFLOAT3 Position);
	XMFLOAT3 SBCollisionMoveXZ(XMFLOAT3 Position, XMFLOAT3 Velocity);

	//플레이어를 회전하는 함수이다.
	void Rotate(float x, float y, float z);

	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//카메라를 변경하기 위하여 호출하는 함수이다.
	CCamera *OnCreateCamera();
	virtual CCamera* CreateCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(NULL);
	}

	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();

	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다.
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