#include "Player.h"
#include "Shader.h"
#include "Network.h"

CPlayer::CPlayer() : CGameObject()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_fPitch = 0.0f;
	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	m_fPlayerGravityTime = 0;
	xmf3JumpShift = {0,0,0};
	m_bPlayerGravity = false;
	m_bPlayerJump = false;

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);

	m_fMaxVelocityXZ = 125.0f;
	m_fMaxVelocityY = 0.0f;

	m_fFriction = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

	m_fPlayerBoundingRadius = 5.0f;

	m_playerNetworkId = GetNetworkPlayerId();

	m_bActive = true;

	m_ani_state = ANIMATION_IDLE;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();

	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

/*플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를
누를 때 변경된다. 플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동한다.*/
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//화살표 키 ‘↑’를 누르면 로컬 z-축 방향으로 이동(전진)한다. ‘↓’를 누르면 반대 방향으로 이동한다.
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);

		//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다.
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//‘Page Up’을 누르면 로컬 y-축 방향으로 이동한다. ‘Page Down’을 누르면 반대 방향으로 이동한다.
		//if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_UP) { 
			//이미 공중에서 중력을 받는 상태
			if (m_bPlayerGravity) {
				//cout << m_fBoosterMount << endl;
				m_fJumpTime += m_fEtime;

				if (m_fJumpTime > 0.2f && m_fBoosterMount > 0) {
					xmf3JumpShift.y += 400.0f * m_fEtime;
					m_fPlayerGravityTime = 0;
					m_fBoosterMount -= m_fEtime * 120;
					if (m_fBoosterMount < 0) {
						m_fBoosterMount -= 30.0f;
					}
				}

				if (xmf3JumpShift.y > 50.f) {
					xmf3JumpShift.y  = 50.0f;
				}
			}
			else {
				m_fJumpTime = 0;
				m_bPlayerGravity = true;
				xmf3JumpShift.y = 50.0f;
			}
		}

		if (GetIsShoot()) {//dwDirection & KEY_SHOOT
			//m_fKeyDownTime += m_fEtime;
			if (!m_bReloading) {
				if (m_fKeyDownTime > 0.1f) {
					int b_id = GetBulletId();
					XMFLOAT3 BP = m_xmf3Position;
					XMFLOAT3 LockVec = GetLookVector();
					XMFLOAT3 RightVec = GetRightVector();
					BP.y += 9.0f;
					BP.x += LockVec.x * 10.0f + RightVec.x * 3.0f;
					BP.z += LockVec.z * 10.0f + RightVec.z * 3.0f;

					m_pScene->AddObjects(0, BP, LockVec, GetPlayerId(), b_id);
					//cout << GetPlayerId() << "가 " << b_id <<  "를 쏴서 서버에 보내고, 저장했습니다." << endl;
					XMFLOAT3 send_p = BP;
					XMFLOAT3 send_v = LockVec;

					send_bullet_add_packet(send_p, send_v, b_id);
					SetBulletId(b_id + 1);
					m_nBullet -= 1;
					//cout << m_nBullet << endl;
					m_fKeyDownTime = 0.f;

					if (m_nBullet == 0) m_bReloading = true;
				}
			}
		}
		
		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다.
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다.
	if (bUpdateVelocity)
	{
		//플레이어의 속도 벡터를 xmf3Shift 벡터만큼 변경한다.
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다.
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

		//플레이어의 위치가 변경되었으므로 카메라의 위치도 xmf3Shift 벡터만큼 이동한다.
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Jump(float fTimeElapsed)
{
	if (m_bPlayerGravity) {
		
		XMFLOAT3 shiftY = { 0,0,0 };

		xmf3JumpShift.y -= 150.f * m_fPlayerGravityTime * fTimeElapsed;
		shiftY.y = xmf3JumpShift.y * fTimeElapsed;

		m_fPlayerGravityTime += fTimeElapsed;

		if (shiftY.y < -2.5f) {
			shiftY.y = -2.5f;
		}

		m_xmf3Position = Vector3::Add(m_xmf3Position, shiftY);
		
		if (m_pCamera) m_pCamera->Move(shiftY);
	}
}

bool CPlayer::BSCollisionCheck(XMFLOAT3 Position,float Radius)
{
	float x = Position.x - m_xmf3Position.x;
	float y = Position.y - m_xmf3Position.y;
	float z = Position.z - m_xmf3Position.z;

	if (Radius + m_fBlockBoundingRadius - 2.0f > sqrt(x*x + y*y + z*z)) return true;

	return false;
}

bool CPlayer::SBCollisionCheck(XMFLOAT3 Position)
{
	float bx = Position.x;
	float by = Position.y;
	float bz = Position.z;

	float size = 6.0f;

	XMFLOAT3 BoxPoint;

	if (m_xmf3Position.x < bx - size) 	BoxPoint.x = bx - size;
	else if (m_xmf3Position.x > bx + size) 	BoxPoint.x = bx + size;
	else BoxPoint.x = m_xmf3Position.x;

	if (m_xmf3Position.y + 5.0f < by - size) 	BoxPoint.y = by - size;
	else if (m_xmf3Position.y + 5.0f > by + size) 	BoxPoint.y = by + size;
	else BoxPoint.y = m_xmf3Position.y + 5.0f;

	if (m_xmf3Position.z < bz - size) 	BoxPoint.z = bz - size;
	else if (m_xmf3Position.z > bz + size) 	BoxPoint.z = bz + size;
	else BoxPoint.z = m_xmf3Position.z;

	XMFLOAT3 Dist;
	XMFLOAT3 bbPosition = m_xmf3Position;
	bbPosition.y += 5.0f; // 바운딩 스피어 위치 조정
	Dist = Vector3::Subtract(bbPosition, BoxPoint);
	float length = Vector3::Length(Dist);

	if (m_fPlayerBoundingRadius > length) { 
		if (by + size > m_xmf3Position.y + 2.5f) return false;

		m_xmf3Position.y = by + size + 0.01f;

		return true; 
	}

	else return false;
}

XMFLOAT3 CPlayer::SBCollisionMoveXZ(XMFLOAT3 Position, XMFLOAT3 Velocity) {

	float bx = Position.x;
	float by = Position.y;
	float bz = Position.z;

	float size = 6.0f;

	XMFLOAT3 BoxPoint;

	if (m_xmf3Position.x < bx - size) 	BoxPoint.x = bx - size;
	else if (m_xmf3Position.x > bx + size) 	BoxPoint.x = bx + size;
	else BoxPoint.x = m_xmf3Position.x;

	if (m_xmf3Position.y + 8.6f < by - size) 	BoxPoint.y = by - size;
	else if (m_xmf3Position.y + 8.6f > by + size) 	BoxPoint.y = by + size;
	else BoxPoint.y = m_xmf3Position.y + 8.6f;

	if (m_xmf3Position.z < bz - size) 	BoxPoint.z = bz - size;
	else if (m_xmf3Position.z > bz + size) 	BoxPoint.z = bz + size;
	else BoxPoint.z = m_xmf3Position.z;

	XMFLOAT3 Dist;
	XMFLOAT3 BackVector;

	Dist = Vector3::Subtract(m_xmf3Position, BoxPoint);
	Dist.y += 8.6f;
	float length = Vector3::Length(Dist);

	if (m_fPlayerBoundingRadius + 3.4f > length) {
		BackVector = Velocity;
		BackVector = Vector3::Normalize(BackVector);
		BackVector.y = 0;

		XMFLOAT3 DistXZ;
		DistXZ = Dist;
		DistXZ.y = 0;
		float XZlength = Vector3::Length(DistXZ);
		XZlength = m_fPlayerBoundingRadius + 3.4f - XZlength + 0.02f;

		BackVector.x *= -XZlength;
		BackVector.z *= -XZlength;

		return BackVector;
	}

	return { 0,0,0 };
	
}

//플레이어를 로컬 x-축, y-축, z-축을 중심으로 회전한다.
void CPlayer::Rotate(float x, float y, float z)
{
	// 3인칭 카메라의 경우 플레이어의 회전은 약간의 제약이 따른다.
	
	/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 그러므로 x-축을 중심으로 회전하는
	각도는 -89.0~+89.0도 사이로 제한한다. x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음
	Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}

	if (y != 0.0f)
	{
		//로컬 y-축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없다.
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}

	if (z != 0.0f)
	{
		/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된다.
		z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다
		작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}

	//카메라를 x, y, z 만큼 회전한다. 플레이어를 회전하면 카메라가 회전하게 된다.
	//m_pCamera->Rotate(x, y, z);

	/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다.
	플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다.
	기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로 z-축(Look 벡터)을 기준으로
	하여 서로 직교하고 단위벡터가 되도록 한다.*/
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//이 함수는 매 프레임마다 호출된다. 플레이어의 속도 벡터에 중력과 마찰력 등을 적용한다.
void CPlayer::Update(float fTimeElapsed)
{
	m_fEtime = fTimeElapsed;
	m_fKeyDownTime += fTimeElapsed;
	//cout << m_bPlayerGravity << " ";
	if (m_fKeyDownTime > 2.0f) {
		m_bReloading = false;
		m_fKeyDownTime = 0.f;
		m_nBullet = 30;
	}

	if (m_fBoosterMount < 100.0f) {
		m_fBoosterMount += fTimeElapsed * 40;
	}

	/*플레이어의 속도 벡터를 중력 벡터와 더한다. 중력 벡터에 fTimeElapsed를 곱하는 것은 중력을 시간에 비례하도록
	적용한다는 의미이다.*/
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	/*플레이어의 속도 벡터의 XZ-성분의 크기를 구한다. 이것이 XZ-평면의 최대 속력보다 크면 속도 벡터의 x와 z-방향
	성분을 조정한다.*/
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ){
		m_xmf3Velocity.x *= (m_fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (m_fMaxVelocityXZ / fLength);
	}

	/*플레이어의 속도 벡터의 y-성분의 크기를 구한다. 이것이 y-축 방향의 최대 속력보다 크면 속도 벡터의 y-방향 성
	분을 조정한다.*/
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//플레이어를 속도 벡터 만큼 실제로 이동한다(카메라도 이동될 것이다)
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	xmf3Velocity.x *= 0.5f;
	xmf3Velocity.y *= 0.5f;
	xmf3Velocity.z *= 0.5f;

	Move(xmf3Velocity, false);
	Jump(fTimeElapsed);

	if (m_ppObjects != NULL) {
		int cnt = 0;

		for (int i = 0; i < m_nBlock; ++i) {
			if (m_ppObjects[i]) {
				if (m_ppObjects[i]->GetIsActive()) {
					if (BSCollisionCheck(m_ppObjects[i]->GetPosition(),
						m_ppObjects[i]->GetBoundingRadius())) {

						XMFLOAT3 Box = m_ppObjects[i]->GetPosition();
						if (SBCollisionCheck(Box)) {
							m_fPlayerGravityTime = 0;
							xmf3JumpShift.y = 0;
							m_bPlayerGravity = false;
							break;
						}
					}
				}
			}
			cnt++;
		}


		for (int i = 0; i < m_nBlock; ++i) {
			if (m_ppObjects[i]) {
				if (m_ppObjects[i]->GetIsActive()) {
					if (BSCollisionCheck(m_ppObjects[i]->GetPosition(),
						m_ppObjects[i]->GetBoundingRadius())) {

						XMFLOAT3 Box = m_ppObjects[i]->GetPosition();
						XMFLOAT3 MoveBack = SBCollisionMoveXZ(Box, xmf3Velocity);

						if (Vector3::Length(MoveBack) > 0) {
							Move(MoveBack, false);
							break;
						}
					}
				}
			}
		}

		if (cnt == m_nBlock) { 
			m_bPlayerGravity = true; 
		}
	}
	
	fLength = sqrtf(xmf3JumpShift.y * xmf3JumpShift.y);
	if (fLength > 0.3f) {
		m_bOnAir = true;
	}
	else m_bOnAir = false;
	//cout << fLength << endl;
	/*플레이어의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 플레이어의 새로운 위치가 유효한 위치가 아닐 수도
	있고 또는 플레이어의 충돌 검사 등을 수행할 필요가 있다. 이러한 상황에서 플레이어의 위치를 유효한 위치로 다시
	변경할 수 있다.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	//플레이어의 위치가 변경되었으므로 3인칭 카메라를 갱신한다.
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//카메라의 위치가 변경될 때 추가로 수행할 작업을 수행한다.
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	//3인칭 카메라가 변경된 플레이어 위치를 바라보도록 한다.
	m_pCamera->SetLookAt(m_xmf3Position);
	
	//카메라의 카메라 변환 행렬을 다시 생성한다.
	m_pCamera->RegenerateViewMatrix();

	/*플레이어의 속도 벡터가 마찰력 때문에 감속이 되어야 한다면 감속 벡터를 생성한다. 속도 벡터의 반대 방향 벡터를
	구하고 단위 벡터로 만든다. 마찰 계수를 시간에 비례하도록 하여 마찰력을 구한다. 단위 벡터에 마찰력을 곱하여 감속 벡터를 구한다.
	속도 벡터에 감속 벡터를 더하여 속도 벡터를 줄인다. 마찰력이 속력보다 크면 속력은 0이 될 것이다.*/
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);

	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

/*카메라를 변경할 때 CreateCamera() 함수에서 호출되는 함수이다.
nCurrentCameraMode는 현재 카메라의 모드이고 nNewCameraMode는 새로 설정할 카메라 모드이다.*/
CCamera* CPlayer::OnCreateCamera()
{
	//새로운 카메라의 모드에 따라 카메라를 새로 생성한다.
	CCamera *pNewCamera = NULL;

	pNewCamera = new CThirdPersonCamera(m_pCamera);

	if (pNewCamera)
	{
		//현재 카메라를 사용하는 플레이어 객체를 설정한다.
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	XMMATRIX scaleMatrix = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	m_xmf4x4Transform = Matrix4x4::Multiply(scaleMatrix, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);

	//std::cout << "이 플레이어의 네트워크 ID : " << m_playerNetworkId << std::endl;
}

void CPlayer::SetIsRotate(bool tf)
{
	isRotate = tf;
}

bool CPlayer::GetIsRotate()
{
	return isRotate;
}

CCubePlayer::CCubePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z, int id)
{
	//플레이어 메쉬를 생성한다.
	CMesh* pCubePlayerMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f);
	SetMesh(pCubePlayerMesh);

	//플레이어의 카메라를 생성한다.
	m_pCamera = CreateCamera(0.0f);

	//플레이어를 위한 셰이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//플레이어의 네트워크 ID 설정
	m_playerNetworkId = id;

	//플레이어의 위치를 설정한다.
	SetPosition(XMFLOAT3(x, y, z));

	//플레이어 메쉬를 렌더링할 때 사용할 셰이더를 생성한다.
	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}

CCubePlayer::~CCubePlayer()
{
}


void CCubePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

CCamera* CCubePlayer::CreateCamera(float fTimeElapsed)
{
	//플레이어의 특성을 3인칭 카메라 모드에 맞게 변경한다. 지연 효과와 카메라 오프셋을 설정한다.
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));

	SetMaxVelocityXZ(125.0f);
	SetMaxVelocityY(400.0f);

	m_pCamera = OnCreateCamera();

	//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25f 대신에 0.0f와 1.0f로 설정한 결과를 비교하기 바란다.
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	//플레이어를 시간의 경과에 따라 갱신(위치와 방향을 변경: 속도, 마찰력, 중력 등을 처리)한다.
	Update(fTimeElapsed);

	return(m_pCamera);
}

CMainPlayer::CMainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z,
	CShader* pPlayerShader, CShader* pSkinnedPlayerShader, CMaterial* pMaterial) : CPlayer()
{
	CLoadedModelInfo* pPlayerModel = CGameObject::LoadModelAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"Models/player model data.bin", "Models/player animation data.bin", pPlayerShader, pSkinnedPlayerShader, pMaterial);
	SetChild(pPlayerModel->m_pModelRootObject);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 12, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);	// Idle
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);	// Run_Forward
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);	// Run_Left
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);	// Run_Right
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);	// Run_Back
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);	// Fire_Bullet
	m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);	// Damaged_Motion
	m_pSkinnedAnimationController->SetTrackAnimationSet(7, 7);	// Death
	m_pSkinnedAnimationController->SetTrackAnimationSet(8, 8);	// Forward + Fire
	m_pSkinnedAnimationController->SetTrackAnimationSet(9, 9);	// Left + Fire
	m_pSkinnedAnimationController->SetTrackAnimationSet(10, 10);	// Right + Fire
	m_pSkinnedAnimationController->SetTrackAnimationSet(11, 11);	// Back + Fire
	m_pSkinnedAnimationController->m_pAnimationTracks[6].m_nType = ANIMATION_TYPE_ONCE;
	m_pSkinnedAnimationController->m_pAnimationTracks[7].m_nType = ANIMATION_TYPE_ONCE;
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackEnable(11, false);

	//SetMaterial(pMaterial);

	SetPosition(XMFLOAT3(x, y, z));

	m_pCamera = CreateCamera(0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CMainPlayer::~CMainPlayer()
{
}

CCamera* CMainPlayer::CreateCamera(float fTimeElapsed)
{
	//플레이어의 특성을 3인칭 카메라 모드에 맞게 변경한다. 지연 효과와 카메라 오프셋을 설정한다.
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));

	SetMaxVelocityXZ(125.0f);
	SetMaxVelocityY(400.0f);

	m_pCamera = OnCreateCamera();

	//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25f 대신에 0.0f와 1.0f로 설정한 결과를 비교하기 바란다.
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	//플레이어를 시간의 경과에 따라 갱신(위치와 방향을 변경: 속도, 마찰력, 중력 등을 처리)한다.
	Update(fTimeElapsed, NULL);

	return(m_pCamera);
}

void CMainPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

void CMainPlayer::OtherPlayerAnimationUpdate(Animation dwOtherPlayerDirection)
{
	switch (dwOtherPlayerDirection) {
	case ANIMATION_IDLE: {
		m_pSkinnedAnimationController->SetTracksEnable(0);
		//cout << "IDLE 출력중" << endl;
		break;
	}
	case ANIMATION_WALK_FORAWRRD: {
		m_pSkinnedAnimationController->SetTracksEnable(1);
		//cout << "FORWORD 출력중" << endl;
		break;
	}
	case ANIMATION_WALK_LEFT: {
		m_pSkinnedAnimationController->SetTracksEnable(2);
		break;
	}
	case ANIMATION_WALK_RIGHT: {
		m_pSkinnedAnimationController->SetTracksEnable(3);
		break;
	}
	case ANIMATION_WALK_BACKWARD: {
		m_pSkinnedAnimationController->SetTracksEnable(4);
		break;
	}
	case ANIMATION_SHOOT: {
		m_pSkinnedAnimationController->SetTracksEnable(5);
		break;
	}
	case ANIMATION_DAMAGED: {
		m_pSkinnedAnimationController->SetTracksEnable(6);
		break;
	}
	case ANIMATION_DEATH: {
		m_pSkinnedAnimationController->SetTracksEnable(7);
		break;
	}
	case ANIMATION_SHOOT_FORWARD: {
		m_pSkinnedAnimationController->SetTracksEnable(8);
		break;
	}
	case ANIMATION_SHOOT_LEFT: {
		m_pSkinnedAnimationController->SetTracksEnable(9);
		break;
	}
	case ANIMATION_SHOOT_RIGHT: {
		m_pSkinnedAnimationController->SetTracksEnable(10);
		break;
	}
	case ANIMATION_SHOOT_BACKWARD: {
		m_pSkinnedAnimationController->SetTracksEnable(11);
		break;
	}
	}
}

void CMainPlayer::SetWalkAnimationSpeed(float fSpeed)
{
	if (m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->SetTrackSpeed(1, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(2, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(3, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(4, fSpeed);

		m_pSkinnedAnimationController->SetTrackSpeed(8, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(9, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(10, fSpeed);
		m_pSkinnedAnimationController->SetTrackSpeed(11, fSpeed);
	}
}

void CMainPlayer::Update(float fTimeElapsed, DWORD dwDirection)
{
	CPlayer::Update(fTimeElapsed);

	if (m_pSkinnedAnimationController)
	{
		if (!GetDeath())		// Alive
		{
			float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
			if (::IsZero(fLength))
			{
				m_pSkinnedAnimationController->SetTracksEnable(0);
				m_ani_state = ANIMATION_IDLE;

				if (GetIsShoot())//dwDirection& KEY_SHOOT
				{
					m_pSkinnedAnimationController->SetTrackEnable(0, false);
					m_pSkinnedAnimationController->SetTrackPosition(0, 0.0f);
					m_pSkinnedAnimationController->SetTrackEnable(5, true);
					m_ani_state = ANIMATION_SHOOT;
				}
			}
		}
		else		// Death
		{
			m_pSkinnedAnimationController->SetTracksEnable(7);
			m_ani_state = ANIMATION_DEATH;
		}
	}
}


void CMainPlayer::Move(DWORD dwDirection, float fDistance, bool bVelocity)
{
	if (!GetDeath())
	{
		float length = Vector3::Length(GetVelocity());
		float ratio = length / 117.0f;

		if (ratio < 0.1f)
			ratio = 0.4f;
		else if (ratio < 0.3f)
			ratio = 0.5f;
		else if (ratio < 0.5f)
			ratio = 0.6f;
		else if (ratio < 0.7f)
			ratio = 0.7f;

		float fAnimationSpeed = 1.4f * ratio;

		if (m_bOnAir)
		{
			SetWalkAnimationSpeed(fAnimationSpeed * 0.6f);
		}
		else
		{
			SetWalkAnimationSpeed(fAnimationSpeed);
		}

		if (dwDirection)	// Move or Shoot
		{
			if (GetIsShoot())	// Shoot
			{
				/*m_pSkinnedAnimationController->SetTracksEnable(5);
				m_ani_state = ANIMATION_SHOOT;*/

				// Shoot And Move
				if (dwDirection & DIR_FORWARD)
				{
					m_pSkinnedAnimationController->SetTracksEnable(8);
					m_ani_state = ANIMATION_SHOOT_FORWARD;
				}
				else if (dwDirection & DIR_LEFT)
				{
					m_pSkinnedAnimationController->SetTracksEnable(9);
					m_ani_state = ANIMATION_SHOOT_LEFT;
				}
				else if (dwDirection & DIR_RIGHT)
				{
					m_pSkinnedAnimationController->SetTracksEnable(10);
					m_ani_state = ANIMATION_SHOOT_RIGHT;
				}
				else if (dwDirection & DIR_BACKWARD)
				{
					m_pSkinnedAnimationController->SetTracksEnable(11);
					m_ani_state = ANIMATION_SHOOT_BACKWARD;
				}
			}
			else				// Don't Shoot
			{
				// Don't Shoot And Move
				if (dwDirection & DIR_FORWARD)
				{
					m_pSkinnedAnimationController->SetTracksEnable(1);
					m_ani_state = ANIMATION_WALK_FORAWRRD;
				}
				else if (dwDirection & DIR_LEFT)
				{
					m_pSkinnedAnimationController->SetTracksEnable(2);
					m_ani_state = ANIMATION_WALK_LEFT;
				}
				else if (dwDirection & DIR_RIGHT)
				{
					m_pSkinnedAnimationController->SetTracksEnable(3);
					m_ani_state = ANIMATION_WALK_RIGHT;
				}
				else if (dwDirection & DIR_BACKWARD)
				{
					m_pSkinnedAnimationController->SetTracksEnable(4);
					m_ani_state = ANIMATION_WALK_BACKWARD;
				}
				else			// Move & Don't Shoot
				{
					m_pSkinnedAnimationController->SetTracksEnable(0);
					m_ani_state = ANIMATION_IDLE;
				}
			}
		}
	}

	CPlayer::Move(dwDirection, fDistance, bVelocity);
}
