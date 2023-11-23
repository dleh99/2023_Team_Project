#include "Player.h"
#include "Shader.h"

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

	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;

	m_fFriction = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

	m_fPlayerBoundingRadius = 5.0f;
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
				
			}
			else {
				m_bPlayerGravity = true;
				xmf3JumpShift.y = 2.0f;
			}
		}

		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		
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
	//std::cout << m_xmf3Position.x << " " << m_xmf3Position.y << std::endl;

	if (m_bPlayerGravity) {
		
		m_fPlayerGravityTime += fTimeElapsed;
		xmf3JumpShift.y -= 0.15f * m_fPlayerGravityTime;

		if (xmf3JumpShift.y < -2.5f) {
			xmf3JumpShift.y = -2.5f;
		}

		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3JumpShift);
		//std::cout << m_xmf3Gravity.y << "\n";
		
		if (m_pCamera) m_pCamera->Move(xmf3JumpShift);

		//if (m_xmf3Position.y < 0.f) {
		//	m_xmf3Position.y = 0;
		//	m_fPlayerGravityTime = 0;
		//	m_bPlayerGravity = false;
		//}
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

	if (m_xmf3Position.y < by - size) 	BoxPoint.y = by - size;
	else if (m_xmf3Position.y > by + size) 	BoxPoint.y = by + size;
	else BoxPoint.y = m_xmf3Position.y;

	if (m_xmf3Position.z < bz - size) 	BoxPoint.z = bz - size;
	else if (m_xmf3Position.z > bz + size) 	BoxPoint.z = bz + size;
	else BoxPoint.z = m_xmf3Position.z;

	XMFLOAT3 Dist;
	Dist = Vector3::Subtract(m_xmf3Position, BoxPoint);
	float length = Vector3::Length(Dist);

	if (m_fPlayerBoundingRadius > length) { 
		m_xmf3Position.y = Position.y + size + m_fPlayerBoundingRadius + 0.01f;
		//std::cout << m_xmf3Position.y << "\n";
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

	if (m_xmf3Position.y + 3.6f < by - size) 	BoxPoint.y = by - size;
	else if (m_xmf3Position.y + 3.6f > by + size) 	BoxPoint.y = by + size;
	else BoxPoint.y = m_xmf3Position.y + 3.6f;

	if (m_xmf3Position.z < bz - size) 	BoxPoint.z = bz - size;
	else if (m_xmf3Position.z > bz + size) 	BoxPoint.z = bz + size;
	else BoxPoint.z = m_xmf3Position.z;

	XMFLOAT3 Dist;
	XMFLOAT3 BackVector;

	Dist = Vector3::Subtract(m_xmf3Position, BoxPoint);
	Dist.y += 3.6f;
	float length = Vector3::Length(Dist);

	if (m_fPlayerBoundingRadius + 3.5f > length) {
		BackVector = Velocity;
		BackVector = Vector3::Normalize(BackVector);
		BackVector.y = 0;

		XMFLOAT3 DistXZ;
		DistXZ = Dist;
		DistXZ.y = 0;
		float XZlength = Vector3::Length(DistXZ);
		XZlength = m_fPlayerBoundingRadius + 3.5f - XZlength + 0.02f;

		BackVector.x *= -XZlength;
		BackVector.z *= -XZlength;

		std::cout << XZlength << "\n";
		return BackVector;
	}

	return { 0,0,0 };
	
	{
		//if (bx + 6.0f > m_xmf3Position.x + 5.0f && m_xmf3Position.x - 5.0f > bx - 6.0f) {}
		//else return false;
		//if (bz + 6.0f > m_xmf3Position.z + 5.0f && m_xmf3Position.z - 5.0f > bz - 6.0f) {}
		//else return false;


		//3���� �� ���� �ִ� �� 3��
		XMFLOAT3 Check[3][3] = {};

		// ��� ������
		float A[3] = {};
		float B[3] = {};
		float C[3] = {};
		float D[3] = {};


		if (bx > m_xmf3Position.x) {
			Check[0][0].x = bx + size; Check[0][0].y = by + size; Check[0][0].z = bz + size;
			Check[0][1].x = bx + size; Check[0][1].y = by - size; Check[0][1].z = bz - size;
			Check[0][2].x = bx + size; Check[0][2].y = by - size; Check[0][2].z = bz + size;

			A[0] = Check[0][0].y * (Check[0][1].z - Check[0][2].z) +
				Check[0][1].y * (Check[0][2].z - Check[0][0].z) +
				Check[0][2].y * (Check[0][0].z - Check[0][1].z);

			B[0] = Check[0][0].z * (Check[0][1].x - Check[0][2].x) +
				Check[0][1].z * (Check[0][2].x - Check[0][0].x) +
				Check[0][2].z * (Check[0][0].x - Check[0][1].x);

			C[0] = Check[0][0].x * (Check[0][1].y - Check[0][2].y) +
				Check[0][1].x * (Check[0][2].y - Check[0][0].y) +
				Check[0][2].x * (Check[0][0].y - Check[0][1].y);

			D[0] = -Check[0][0].x * (Check[0][1].y * Check[0][2].z - Check[0][2].y * Check[0][1].z) -
				Check[0][1].x * (Check[0][2].y * Check[0][0].z - Check[0][0].y * Check[0][2].z) -
				Check[0][2].x * (Check[0][0].y * Check[0][1].z - Check[0][1].y * Check[0][0].z);
		}
		else {
			Check[0][0].x = bx - size; Check[0][0].y = by + size; Check[0][0].z = bz + size;
			Check[0][1].x = bx - size; Check[0][1].y = by - size; Check[0][1].z = bz - size;
			Check[0][2].x = bx - size; Check[0][2].y = by - size; Check[0][2].z = bz + size;

			A[0] = Check[0][0].y * (Check[0][1].z - Check[0][2].z) +
				Check[0][1].y * (Check[0][2].z - Check[0][0].z) +
				Check[0][2].y * (Check[0][0].z - Check[0][1].z);

			B[0] = Check[0][0].z * (Check[0][1].x - Check[0][2].x) +
				Check[0][1].z * (Check[0][2].x - Check[0][0].x) +
				Check[0][2].z * (Check[0][0].x - Check[0][1].x);

			C[0] = Check[0][0].x * (Check[0][1].y - Check[0][2].y) +
				Check[0][1].x * (Check[0][2].y - Check[0][0].y) +
				Check[0][2].x * (Check[0][0].y - Check[0][1].y);

			D[0] = -Check[0][0].x * (Check[0][1].y * Check[0][2].z - Check[0][2].y * Check[0][1].z) -
				Check[0][1].x * (Check[0][2].y * Check[0][0].z - Check[0][0].y * Check[0][2].z) -
				Check[0][2].x * (Check[0][0].y * Check[0][1].z - Check[0][1].y * Check[0][0].z);
		}

		if (by > m_xmf3Position.y) {
			Check[1][0].x = bx + size; Check[1][0].y = by + size; Check[1][0].z = bz + size;
			Check[1][1].x = bx + size; Check[1][1].y = by + size; Check[1][1].z = bz - size;
			Check[1][2].x = bx - size; Check[1][2].y = by + size; Check[1][2].z = bz + size;

			A[1] = Check[1][0].y * (Check[1][1].z - Check[1][2].z) +
				Check[1][1].y * (Check[1][2].z - Check[1][0].z) +
				Check[1][2].y * (Check[1][0].z - Check[1][1].z);

			B[1] = Check[1][0].z * (Check[1][1].x - Check[1][2].x) +
				Check[1][1].z * (Check[1][2].x - Check[1][0].x) +
				Check[1][2].z * (Check[1][0].x - Check[1][1].x);

			C[1] = Check[1][0].x * (Check[1][1].y - Check[1][2].y) +
				Check[1][1].x * (Check[1][2].y - Check[1][0].y) +
				Check[1][2].x * (Check[1][0].y - Check[1][1].y);

			D[1] = -Check[1][0].x * (Check[1][1].y * Check[1][2].z - Check[1][2].y * Check[1][1].z) -
				Check[1][1].x * (Check[1][2].y * Check[1][0].z - Check[1][0].y * Check[1][2].z) -
				Check[1][2].x * (Check[1][0].y * Check[1][1].z - Check[1][1].y * Check[1][0].z);
		}
		else {
			Check[1][0].x = bx + size; Check[1][0].y = by - size; Check[1][0].z = bz + size;
			Check[1][1].x = bx + size; Check[1][1].y = by - size; Check[1][1].z = bz - size;
			Check[1][2].x = bx - size; Check[1][2].y = by - size; Check[1][2].z = bz + size;

			A[1] = Check[1][0].y * (Check[1][1].z - Check[1][2].z) +
				Check[1][1].y * (Check[1][2].z - Check[1][0].z) +
				Check[1][2].y * (Check[1][0].z - Check[1][1].z);

			B[1] = Check[1][0].z * (Check[1][1].x - Check[1][2].x) +
				Check[1][1].z * (Check[1][2].x - Check[1][0].x) +
				Check[1][2].z * (Check[1][0].x - Check[1][1].x);

			C[1] = Check[1][0].x * (Check[1][1].y - Check[1][2].y) +
				Check[1][1].x * (Check[1][2].y - Check[1][0].y) +
				Check[1][2].x * (Check[1][0].y - Check[1][1].y);

			D[1] = -Check[1][0].x * (Check[1][1].y * Check[1][2].z - Check[1][2].y * Check[1][1].z) -
				Check[1][1].x * (Check[1][2].y * Check[1][0].z - Check[1][0].y * Check[1][2].z) -
				Check[1][2].x * (Check[1][0].y * Check[1][1].z - Check[1][1].y * Check[1][0].z);
		}

		if (bz > m_xmf3Position.z) {
			Check[2][0].x = bx + size; Check[2][0].y = by + size; Check[2][0].z = bz + size;
			Check[2][1].x = bx + size; Check[2][1].y = by - size; Check[2][1].z = bz + size;
			Check[2][2].x = bx - size; Check[2][2].y = by - size; Check[2][2].z = bz + size;

			A[2] = Check[2][0].y * (Check[2][1].z - Check[2][2].z) +
				Check[2][1].y * (Check[2][2].z - Check[2][0].z) +
				Check[2][2].y * (Check[2][0].z - Check[2][1].z);

			B[2] = Check[2][0].z * (Check[2][1].x - Check[2][2].x) +
				Check[2][1].z * (Check[2][2].x - Check[2][0].x) +
				Check[2][2].z * (Check[2][0].x - Check[2][1].x);

			C[2] = Check[2][0].x * (Check[2][1].y - Check[2][2].y) +
				Check[2][1].x * (Check[2][2].y - Check[2][0].y) +
				Check[2][2].x * (Check[2][0].y - Check[2][1].y);

			D[2] = -Check[2][0].x * (Check[2][1].y * Check[2][2].z - Check[2][2].y * Check[2][1].z) -
				Check[2][1].x * (Check[2][2].y * Check[2][0].z - Check[2][0].y * Check[2][2].z) -
				Check[2][2].x * (Check[2][0].y * Check[2][1].z - Check[2][1].y * Check[2][0].z);
		}
		else {
			Check[2][0].x = bx + size; Check[2][0].y = by + size; Check[2][0].z = bz - size;
			Check[2][1].x = bx - size; Check[2][1].y = by - size; Check[2][1].z = bz - size;
			Check[2][2].x = bx + size; Check[2][2].y = by - size; Check[2][2].z = bz - size;

			A[2] = Check[2][0].y * (Check[2][1].z - Check[2][2].z) +
				Check[2][1].y * (Check[2][2].z - Check[2][0].z) +
				Check[2][2].y * (Check[2][0].z - Check[2][1].z);

			B[2] = Check[2][0].z * (Check[2][1].x - Check[2][2].x) +
				Check[2][1].z * (Check[2][2].x - Check[2][0].x) +
				Check[2][2].z * (Check[2][0].x - Check[2][1].x);

			C[2] = Check[2][0].x * (Check[2][1].y - Check[2][2].y) +
				Check[2][1].x * (Check[2][2].y - Check[2][0].y) +
				Check[2][2].x * (Check[2][0].y - Check[2][1].y);

			D[2] = -Check[2][0].x * (Check[2][1].y * Check[2][2].z - Check[2][2].y * Check[2][1].z) -
				Check[2][1].x * (Check[2][2].y * Check[2][0].z - Check[2][0].y * Check[2][2].z) -
				Check[2][2].x * (Check[2][0].y * Check[2][1].z - Check[2][1].y * Check[2][0].z);
		}

		float distance[3] = {};

		for (int i = 0; i < 3; ++i) {

			distance[i] = fabs(A[i] * m_xmf3Position.x + B[i] * m_xmf3Position.y + C[i] * m_xmf3Position.z + D[i]) /
				sqrt(A[i] * A[i] + B[i] * B[i] + C[i] * C[i]);
		}

		for (int i = 0; i < 3; ++i) {

			if (distance[i] < m_fPlayerBoundingRadius + 1.0f) {
				std::cout << "�浹üũ" << std::endl;
			}
		}
	}
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
	/*�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ���
	�����Ѵٴ� �ǹ��̴�.*/
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	/*�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-����
	������ �����Ѵ�.*/
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}

	/*�÷��̾��� �ӵ� ������ y-������ ũ�⸦ ���Ѵ�. �̰��� y-�� ������ �ִ� �ӷº��� ũ�� �ӵ� ������ y-���� ��
	���� �����Ѵ�.*/
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//�÷��̾ �ӵ� ���� ��ŭ ������ �̵��Ѵ�(ī�޶� �̵��� ���̴�)
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	Move(xmf3Velocity, false);
	Jump(fTimeElapsed);

	if (m_ppObjects != NULL) {
		int cnt = 0;
		for (int i = 0; i < 1008; ++i) {
			if (BSCollisionCheck(m_ppObjects[i]->GetPosition(),
				m_ppObjects[i]->GetBoundingRadius())) {

				XMFLOAT3 Box = m_ppObjects[i]->GetPosition();
				XMFLOAT3 MoveBack = SBCollisionMoveXZ(Box, xmf3Velocity);

				if (SBCollisionCheck(Box)) {
					m_fPlayerGravityTime = 0;
					xmf3JumpShift.y = 0;
					m_bPlayerGravity = false;
					//std::cout << m_xmf3Position.x << " " << m_xmf3Position.y << " " << m_xmf3Position.z << "\n";
					break;
				}

				if (Vector3::Length(MoveBack) > 0) {
					Move(MoveBack,false);
					break;
				}
			}
			cnt++;
		}

		if (cnt == 1008) m_bPlayerGravity = true;
		//std::cout << m_ppObjects[0]->GetPosition().x << " " << m_ppObjects[0]->GetPosition().y << " " << m_ppObjects[0]->GetPosition().z << "\n";
	}
	
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

/*�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
�÷��̾��� Right ���Ͱ� ���� ��ȯ ����� ù ��° �� ����, Up ���Ͱ� �� ��° �� ����, Look ���Ͱ� �� ��° �� ����,
�÷��̾��� ��ġ ���Ͱ� �� ��° �� ���Ͱ� �ȴ�.*/
void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//ī�޶� ��尡 3��Ī�̸� �÷��̾� ��ü�� �������Ѵ�.
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
	CGameObject::Render(pd3dCommandList, pCamera);
}

CCubePlayer::CCubePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z)
{
	//�÷��̾� �޽��� �����Ѵ�.
	CMesh* pCubePlayerMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f);
	SetMesh(pCubePlayerMesh);

	//�÷��̾��� ī�޶� �����Ѵ�.
	m_pCamera = CreateCamera(0.0f);

	//�÷��̾ ���� ���̴� ������ �����Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//�÷��̾��� ��ġ�� �����Ѵ�.
	std::cout << x << ", " << y << ", " << z << std::endl;
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
	ID3D12RootSignature* pd3dGraphicsRootSignature) : CPlayer()
{
	CPlayerMesh* pPlayerMesh = new CPlayerMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pPlayerMesh);

	SetScale(50.0f);
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	SetPos(0.0f, 0.0f, -50.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	/*CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);*/
}

CMainPlayer::~CMainPlayer()
{
}

void CMainPlayer::OnPrepareRender()
{
}
