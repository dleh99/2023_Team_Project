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

/*�÷��̾��� ��ġ�� �����ϴ� �Լ��̴�. �÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带
���� �� ����ȴ�. �÷��̾��� �̵� ����(dwDirection)�� ���� �÷��̾ fDistance ��ŭ �̵��Ѵ�.*/
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//ȭ��ǥ Ű ���衯�� ������ ���� z-�� �������� �̵�(����)�Ѵ�. ���顯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);

		//ȭ��ǥ Ű ���桯�� ������ ���� x-�� �������� �̵��Ѵ�. ���硯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//��Page Up���� ������ ���� y-�� �������� �̵��Ѵ�. ��Page Down���� ������ �ݴ� �������� �̵��Ѵ�.
		//if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_UP) { 
			//�̹� ���߿��� �߷��� �޴� ����
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
					//cout << GetPlayerId() << "�� " << b_id <<  "�� ���� ������ ������, �����߽��ϴ�." << endl;
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
		
		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity)
	{
		//�÷��̾��� �ӵ� ���͸� xmf3Shift ���͸�ŭ �����Ѵ�.
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xmf3Shift ���͸�ŭ �̵��Ѵ�.
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
	bbPosition.y += 5.0f; // �ٿ�� ���Ǿ� ��ġ ����
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

//�÷��̾ ���� x-��, y-��, z-���� �߽����� ȸ���Ѵ�.
void CPlayer::Rotate(float x, float y, float z)
{
	// 3��Ī ī�޶��� ��� �÷��̾��� ȸ���� �ణ�� ������ ������.
	
	/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ�
	������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ����
	Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}

	if (y != 0.0f)
	{
		//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����.
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}

	if (z != 0.0f)
	{
		/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵȴ�.
		z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������
		������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}

	//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�.
	//m_pCamera->Rotate(x, y, z);

	/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��.
	�÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��.
	�⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ� z-��(Look ����)�� ��������
	�Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//�� �Լ��� �� �����Ӹ��� ȣ��ȴ�. �÷��̾��� �ӵ� ���Ϳ� �߷°� ������ ���� �����Ѵ�.
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

	/*�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ���
	�����Ѵٴ� �ǹ��̴�.*/
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	/*�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-����
	������ �����Ѵ�.*/
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ){
		m_xmf3Velocity.x *= (m_fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (m_fMaxVelocityXZ / fLength);
	}

	/*�÷��̾��� �ӵ� ������ y-������ ũ�⸦ ���Ѵ�. �̰��� y-�� ������ �ִ� �ӷº��� ũ�� �ӵ� ������ y-���� ��
	���� �����Ѵ�.*/
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//�÷��̾ �ӵ� ���� ��ŭ ������ �̵��Ѵ�(ī�޶� �̵��� ���̴�)
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
	/*�÷��̾��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. �÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ����
	�ְ� �Ǵ� �÷��̾��� �浹 �˻� ���� ������ �ʿ䰡 �ִ�. �̷��� ��Ȳ���� �÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ�
	������ �� �ִ�.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� 3��Ī ī�޶� �����Ѵ�.
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//ī�޶��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�.
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	//3��Ī ī�޶� ����� �÷��̾� ��ġ�� �ٶ󺸵��� �Ѵ�.
	m_pCamera->SetLookAt(m_xmf3Position);
	
	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�.
	m_pCamera->RegenerateViewMatrix();

	/*�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�. �ӵ� ������ �ݴ� ���� ���͸�
	���ϰ� ���� ���ͷ� �����. ���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�. ���� ���Ϳ� �������� ���Ͽ� ���� ���͸� ���Ѵ�.
	�ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�. �������� �ӷº��� ũ�� �ӷ��� 0�� �� ���̴�.*/
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);

	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

/*ī�޶� ������ �� CreateCamera() �Լ����� ȣ��Ǵ� �Լ��̴�.
nCurrentCameraMode�� ���� ī�޶��� ����̰� nNewCameraMode�� ���� ������ ī�޶� ����̴�.*/
CCamera* CPlayer::OnCreateCamera()
{
	//���ο� ī�޶��� ��忡 ���� ī�޶� ���� �����Ѵ�.
	CCamera *pNewCamera = NULL;

	pNewCamera = new CThirdPersonCamera(m_pCamera);

	if (pNewCamera)
	{
		//���� ī�޶� ����ϴ� �÷��̾� ��ü�� �����Ѵ�.
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

	//std::cout << "�� �÷��̾��� ��Ʈ��ũ ID : " << m_playerNetworkId << std::endl;
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
	//�÷��̾� �޽��� �����Ѵ�.
	CMesh* pCubePlayerMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f);
	SetMesh(pCubePlayerMesh);

	//�÷��̾��� ī�޶� �����Ѵ�.
	m_pCamera = CreateCamera(0.0f);

	//�÷��̾ ���� ���̴� ������ �����Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//�÷��̾��� ��Ʈ��ũ ID ����
	m_playerNetworkId = id;

	//�÷��̾��� ��ġ�� �����Ѵ�.
	SetPosition(XMFLOAT3(x, y, z));

	//�÷��̾� �޽��� �������� �� ����� ���̴��� �����Ѵ�.
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
	//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�.
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));

	SetMaxVelocityXZ(125.0f);
	SetMaxVelocityY(400.0f);

	m_pCamera = OnCreateCamera();

	//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���.
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�.
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
	//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�.
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));

	SetMaxVelocityXZ(125.0f);
	SetMaxVelocityY(400.0f);

	m_pCamera = OnCreateCamera();

	//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���.
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�.
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
		//cout << "IDLE �����" << endl;
		break;
	}
	case ANIMATION_WALK_FORAWRRD: {
		m_pSkinnedAnimationController->SetTracksEnable(1);
		//cout << "FORWORD �����" << endl;
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
