#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4Transform, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	
	m_fBlockBoundingRadius = sqrt(144.f * 3.f) / 2;
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}

	if (m_pChild) delete m_pChild;
	if (m_pSibling) delete m_pSibling;
}

void CGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh)m_pMesh->AddRef();
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::Animate(float fTimeElapsed)
{

}

void CGameObject::OnPrepareRender()
{

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pShader)
	{
		m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		m_pShader->Render(pd3dCommandList, pCamera);
	}

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		m_pMesh->Render(pd3dCommandList);
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	
}

void CGameObject::ReleaseShaderVariables()
{
}

XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

// 게임 객체의 로컬 z-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetLook()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33));
}

// 게임 객체의 로컬 y-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetUp()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23));
}

XMFLOAT3 CGameObject::GetRight()
{
	return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13));
}

int CGameObject::GetObjectType()
{
	return m_ObjType;
}

bool CGameObject::GetIsActive()
{
	return m_bActive;
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetObjectType(int type)
{
	m_ObjType = type;
}

void CGameObject::SetScale(float scale)
{
	m_xmf4x4Transform._11 *= scale;
	m_xmf4x4Transform._22 *= scale;
	m_xmf4x4Transform._33 *= scale;

	UpdateTransform(NULL);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject::SetIsActive(bool Active)
{
	m_bActive = Active;
}

// 게임 객체를 로컬 x-축 방향으로 이동한다.
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();

	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

// 게임 객체를 로컬 y-축 방향으로 이동한다.
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();

	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

// 게임 객체를 로컬 z-축 방향으로 이동한다.
void CGameObject::moveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();

	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

// 게임 객체를 주어진 각도로 회전한다.
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject::SetChild(CGameObject* pChild)
{
	if (m_pChild)
	{
		if (pChild)
			pChild->m_pSibling = m_pChild->m_pSibling;

		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}

	if (pChild) pChild->m_pParent = this;
}

CGameObject* CGameObject::FindFrame(_TCHAR* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (!_tcsncmp(m_pstrFrameName, pstrFrameName, _tcslen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

CGameObject* CGameObject::LoadHierarchyModelFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
	const char* pstrFileName)
{
	std::ifstream fPlayerModelFile{ pstrFileName, std::ios::binary };

	if (!fPlayerModelFile) {
		std::cout << "플레이어 모델 파일 읽기 실패" << std::endl;
		exit(-1);
	}

	CGameObject* pGameObject = NULL;

	pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		fPlayerModelFile);

	return pGameObject;
}

CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, std::ifstream& fileStream)
{
	CGameObject* pGameObject = NULL;
	pGameObject = new CGameObject();

	//fileStream.read((char*)&token, sizeof(char));

	while (true) {
		char token;
		fileStream >> token;

		if ('F' == token)
		{
			fileStream.read((char*)&pGameObject->m_nFrames, sizeof(int));
			//fileStream.read((char*)&m_pstrFrameName, sizeof(int));

			// Transform
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			fileStream.read((char*)&xmf3Position, sizeof(XMFLOAT3));
			fileStream.read((char*)&xmf3Rotation, sizeof(XMFLOAT3));
			fileStream.read((char*)&xmf3Scale, sizeof(XMFLOAT3));
			fileStream.read((char*)&xmf4Rotation, sizeof(XMFLOAT4));

			// Transform Matrix
			fileStream.read((char*)&pGameObject->m_xmf4x4Transform, sizeof(XMFLOAT4X4));
		}
		else if ('M' == token)
		{
			// Mesh Info
			CMeshLoadInfo* pMeshInfo = pGameObject->LoadMeshInfoFromFile(fileStream, 20.0f);

			if (pMeshInfo)
			{
				CPlayerMesh* pMesh = NULL;
				pMesh = new CPlayerMesh(pd3dDevice, pd3dCommandList, pMeshInfo);
				if (pMesh) pGameObject->SetMesh(pMesh);

				delete pMeshInfo;
			}
		}
		else if ('C' == token)
		{
			int nChildren = 0;
			fileStream.read((char*)&nChildren, sizeof(int));

			if (nChildren > 0)
			{
				for (int i = 0; i < nChildren; ++i) {
					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList,
						pd3dGraphicsRootSignature, fileStream);

					if (pChild) pGameObject->SetChild(pChild);
				}
			}
		}
		else if ('H' == token)
			break;
	}

	return pGameObject;
}

CMeshLoadInfo* CGameObject::LoadMeshInfoFromFile(std::ifstream& fileStream, float modelScaleFactor)
{
	CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

	fileStream.read((char*)&pMeshInfo->m_nPositions, sizeof(int));
	pMeshInfo->m_pxmf3Positions = new XMFLOAT3[pMeshInfo->m_nPositions];
	fileStream.read((char*)pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3) * pMeshInfo->m_nPositions);

	/*for (int i = 0; i < pMeshInfo->m_nPositions; ++i) {
		pMeshInfo->m_pxmf3Positions[i].x *= modelScaleFactor;
		pMeshInfo->m_pxmf3Positions[i].y *= modelScaleFactor;
		pMeshInfo->m_pxmf3Positions[i].z *= modelScaleFactor;
	}*/

	fileStream.read((char*)&pMeshInfo->m_nNormals, sizeof(int));
	pMeshInfo->m_pxmf3Normals = new XMFLOAT3[pMeshInfo->m_nNormals];
	fileStream.read((char*)pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3) * pMeshInfo->m_nNormals);

	fileStream.read((char*)&pMeshInfo->m_nTangents, sizeof(int));
	pMeshInfo->m_pxmf4Tangents = new XMFLOAT4[pMeshInfo->m_nTangents];
	fileStream.read((char*)pMeshInfo->m_pxmf4Tangents, sizeof(XMFLOAT4) * pMeshInfo->m_nTangents);

	fileStream.read((char*)&pMeshInfo->m_nUvs, sizeof(int));
	pMeshInfo->m_pxmf2Uvs = new XMFLOAT2[pMeshInfo->m_nUvs];
	fileStream.read((char*)pMeshInfo->m_pxmf2Uvs, sizeof(XMFLOAT2) * pMeshInfo->m_nUvs);

	fileStream.read((char*)&pMeshInfo->m_nIndices, sizeof(int));
	pMeshInfo->m_pIndices = new UINT[pMeshInfo->m_nIndices];
	fileStream.read((char*)pMeshInfo->m_pIndices, sizeof(UINT) * pMeshInfo->m_nIndices);

	return pMeshInfo;
}

CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 90.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

int CBlockObject::GetBlockType()
{
	return m_iBlockType;
};

void CBulletObject::Animate(float fTimeElapsed)
{
	XMFLOAT3 position = GetPosition();
	XMFLOAT3 velocity;

	float speed = m_fSpeed * fTimeElapsed;
	velocity = m_xmf3Vector;
	velocity.x *= speed;
	velocity.y *= speed;
	velocity.z *= speed;

	position = Vector3::Add(position, velocity);

	if (position.x > 200.0f || position.x < -200.0f) {
		m_bActive = false;
		return;
	}

	if (position.z > 200.0f || position.z < -200.0f) {
		m_bActive = false;
		return;
	}

	if (position.y > 200.0f || position.y < -200.0f) {
		m_bActive = false;
		return;
	}

	CGameObject::SetPosition(position);
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}