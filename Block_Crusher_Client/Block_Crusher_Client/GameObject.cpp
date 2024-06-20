#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Scene.h"
#include "Player.h"

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters)
{
	m_nTextureType = nTextureType;

	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
		m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_ppd3dTextureUploadBuffers[i] = m_ppd3dTextures[i] = NULL;

		m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
		for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';

		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;

		m_pnResourceTypes = new UINT[m_nTextures];
		m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
		m_pnBufferElements = new int[m_nTextures];
	}
	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0) m_pnRootParameterIndices = new int[nRootParameters];
	for (int i = 0; i < m_nRootParameters; i++) m_pnRootParameterIndices[i] = -1;

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;

	if (m_pnResourceTypes) delete[] m_pnResourceTypes;
	if (m_pdxgiBufferFormats) delete[] m_pdxgiBufferFormats;
	if (m_pnBufferElements) delete[] m_pnBufferElements;

	if (m_pnRootParameterIndices) delete[] m_pnRootParameterIndices;
	if (m_pd3dSrvGpuDescriptorHandles) delete[] m_pd3dSrvGpuDescriptorHandles;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootParameterIndex(int nIndex, UINT nRootParameterIndex)
{
	m_pnRootParameterIndices[nIndex] = nRootParameterIndex;
}

void CTexture::SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nRootParameters == m_nTextures)
	{
		/*for (int i = 0; i < m_nRootParameters; i++)
		{
			if (m_pd3dSrvGpuDescriptorHandles[i].ptr && (m_pnRootParameterIndices[i] != -1)) pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
		}*/
		for (int i = 0; i < m_nRootParameters; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
		}
	}
	else
	{
		//if (m_pd3dSrvGpuDescriptorHandles[0].ptr) pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE SrvHandle)
{
	if (m_nRootParameters == m_nTextures)
	{
		/*for (int i = 0; i < m_nRootParameters; i++)
		{
			if (m_pd3dSrvGpuDescriptorHandles[i].ptr && (m_pnRootParameterIndices[i] != -1)) pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
		}*/
		for (int i = 0; i < m_nRootParameters; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], SrvHandle);
		}
	}
	else
	{
		//if (m_pd3dSrvGpuDescriptorHandles[0].ptr) pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[nParameterIndex], m_pd3dSrvGpuDescriptorHandles[nTextureIndex]);
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, 1, 0, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);

	return m_ppd3dTextures[nIndex];
}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,const wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	ID3D12Resource* pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	::ZeroMemory(&d3dShaderResourceViewDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_pdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_pnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CMaterial::SetTexture(CTexture* pTexture)
{
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CMaterial::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
	/*std::cout << this << std::endl;
	std::cout << m_xmf4Ambient.x << " " << m_xmf4Ambient.y << " " << m_xmf4Ambient.z << std::endl;
	std::cout << m_xmf4Albedo.x << " " << m_xmf4Albedo.y << " " << m_xmf4Albedo.z << std::endl;
	std::cout << m_xmf4Specular.x << " " << m_xmf4Specular.y << " " << m_xmf4Specular.z << std::endl;
	std::cout << m_xmf4Emissive.x << " " << m_xmf4Emissive.y << " " << m_xmf4Emissive.z << std::endl << std::endl;*/

	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4Ambient, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4Albedo, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4Specular, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4Emissive, 28);
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) m_pShader->ReleaseShaderVariables();
	if (m_pTexture) m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4Transform, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	
	m_fBlockBoundingRadius = sqrt(144.f * 3.f) / 2;

	SetPosition(99999, 99999, 99999);

	for (int i = 0; i < 64; ++i)
		m_pstrFrameName[i] = '\0';
	m_d3dCbvGPUDescriptorHandle.ptr = NULL;
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}

	if (m_pMaterial) m_pMaterial->Release();

	if (m_pChild) delete m_pChild;
	if (m_pSibling) delete m_pSibling;
}

void CGameObject::SetShader(CShader* pShader)
{
	//if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh)m_pMesh->AddRef();
}

void CGameObject::SetMaterial(CMaterial* pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial)m_pMaterial->AddRef();
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
	OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::AnimateParticles(float fTimeElapsed)
{
	for (int i = 0; i < m_nParticle; i++) {
		reinterpret_cast<CParticle*>(m_pParticles[i])->Animate(fTimeElapsed);
	}
}

void CGameObject::OnPrepareRender()
{

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (GetIsActive() == true)
	{
		OnPrepareRender();

		if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

		if (m_pShader)
		{
			//m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			//if (!m_bShadow)
			m_pShader->Render(pd3dCommandList, pCamera);
		}

		if (m_pMaterial)
		{
			if (m_pMaterial->m_pTexture)
			{
				m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			}

			if (m_pMaterial->m_pShader)
			{
				m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
				m_pMaterial->m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			}

			// 재질 업데이트
			m_pMaterial->UpdateShaderVariable(pd3dCommandList);
		}

		if (m_pMesh)
		{
			if (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)
			{

			}

			UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			m_pMesh->Render(pd3dCommandList);
		}
	}

	if (m_pSibling)
	{
		//m_pSibling->m_bShadow = true;
		m_pSibling->Render(pd3dCommandList, pCamera);
		//m_pSibling->m_bShadow = false;
	}
	if (m_pChild)
	{
		//m_pChild->m_bShadow = true;
		m_pChild->Render(pd3dCommandList, pCamera);
		//m_pChild->m_bShadow = false;
	}
}

void CGameObject::RenderParticles(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < m_nParticle; i++)
		m_pParticles[i]->Render(pd3dCommandList, pCamera);

	//std::cout << "파티클 랜더" << std::endl;
}

void CGameObject::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (GetIsActive() == true)
	{
		OnPrepareRender();

		if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

		if (m_pShader)
		{
			//m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			//if (!m_bShadow)
			m_pShader->Render(pd3dCommandList, pCamera);
		}

		if (m_pMesh)
		{
			UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			m_pMesh->Render(pd3dCommandList);
		}
	}

	if (m_pSibling) m_pSibling->ShadowRender(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->ShadowRender(pd3dCommandList, pCamera);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);

	//m_pMaterial.
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
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

XMFLOAT4X4 CGameObject::GetWorldMatrix()
{
	return m_xmf4x4World;
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

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

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

void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh*)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void CGameObject::SearchRifle(CGameObject** pRifle)
{
	if (m_nFrames == 32)		// Rifle
	{
		*pRifle = this;
		return;
	}

	if (m_pSibling) m_pSibling->SearchRifle(pRifle);
	if (m_pChild) m_pChild->SearchRifle(pRifle);
}

void CGameObject::SearchShotgun(CGameObject** pShotgun)
{
	if (m_nFrames == 33)		// Shotgun
	{
		*pShotgun = this;
		SetIsActive(false);
		return;
	}

	if (m_pSibling) m_pSibling->SearchShotgun(pShotgun);
	if (m_pChild) m_pChild->SearchShotgun(pShotgun);
}

void CGameObject::SearchPistol(CGameObject** pPistol)
{
	if (m_nFrames == 34)		// Pistol
	{
		*pPistol = this;
		SetIsActive(false);
		return;
	}

	if (m_pSibling) m_pSibling->SearchPistol(pPistol);
	if (m_pChild) m_pChild->SearchPistol(pPistol);
}

CLoadedModelInfo* CGameObject::LoadModelAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
	const char* pstrModelFileName, const char* pstrAnimationFileName, CShader* pPlayerMeshShader, CShader* pPlayerSkinnedMeshShader, CMaterial* pMaterial)
{
	std::ifstream fPlayerModelFile{ pstrModelFileName, std::ios::binary };

	if (!fPlayerModelFile) {
		std::cout << "플레이어 모델 파일 읽기 실패" << std::endl;
		exit(-1);
	}

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		fPlayerModelFile, pPlayerMeshShader, pPlayerSkinnedMeshShader, &pLoadedModel->m_nSkinnedMeshes, pMaterial);

	if (pstrAnimationFileName)
	{
		std::ifstream fPlayerAnimationFile{ pstrAnimationFileName, std::ios::binary };

		if (!fPlayerAnimationFile) {
			std::cout << "플레이어 애니메이션 파일 읽기 실패" << std::endl;
			exit(-1);
		}

		CGameObject::LoadAnimationFromFile(fPlayerAnimationFile, pLoadedModel);
		pLoadedModel->PrepareSkinning();

		char token[64] = { 0, };
		//strcpy(token, "ForwardAndShoot");

		// Rifle
		CAnimationSet* pShootAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[5];

		for (int k = 0; k < 4; ++k)
		{
			CAnimationSet* pWalkAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[1 + k];

			float fLength = pWalkAnimationSet->m_fLength;
			int nFramesPerSecond = pWalkAnimationSet->m_nFramePerSecond;
			int nKeyFrames = pWalkAnimationSet->m_nKeyFrames;

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[15 + k] =
				new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames,
					token);

			CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[15 + k];

			for (int i = 0; i < nKeyFrames; ++i)
			{
				pAnimationSet->m_pKeyFrameTimes[i] = pWalkAnimationSet->m_pKeyFrameTimes[i];

				for (int j = 0; j < 12; ++j)
				{
					if (j == 2)			// Pelvis
					{
						if (i < pShootAnimationSet->m_nKeyFrames)
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
						}
						else
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 27][j];
						}
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pWalkAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
				}

				for (int j = 12; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++j)
				{
					if (i < pShootAnimationSet->m_nKeyFrames)
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 27][j];
					}
				}
			}
		}

		// Rifle
		CAnimationSet* pRifleShootAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[5];

		for (int k = 0; k < 4; ++k)
		{
			CAnimationSet* pWalkAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[1 + k];

			float fLength = pWalkAnimationSet->m_fLength;
			int nFramesPerSecond = pWalkAnimationSet->m_nFramePerSecond;
			int nKeyFrames = pWalkAnimationSet->m_nKeyFrames;

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[15 + k] =
				new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames,
					token);

			CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[15 + k];

			for (int i = 0; i < nKeyFrames; ++i)
			{
				pAnimationSet->m_pKeyFrameTimes[i] = pWalkAnimationSet->m_pKeyFrameTimes[i];

				for (int j = 0; j < 12; ++j)
				{
					if (j == 2)			// Pelvis
					{
						if (i < pRifleShootAnimationSet->m_nKeyFrames)
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pRifleShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
						}
						else
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pRifleShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 27][j];
						}
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pWalkAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
				}

				for (int j = 12; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++j)
				{
					if (i < pRifleShootAnimationSet->m_nKeyFrames)
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pRifleShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pRifleShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 27][j];
					}
				}
			}
		}

		// Pistol
		CAnimationSet* pPistolShootAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[14];

		for (int k = 0; k < 4; ++k)
		{
			CAnimationSet* pWalkAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[10 + k];

			float fLength = pWalkAnimationSet->m_fLength;
			int nFramesPerSecond = pWalkAnimationSet->m_nFramePerSecond;
			int nKeyFrames = pWalkAnimationSet->m_nKeyFrames;

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[19 + k] =
				new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames,
					token);

			CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[19 + k];

			for (int i = 0; i < nKeyFrames; ++i)
			{
				pAnimationSet->m_pKeyFrameTimes[i] = pWalkAnimationSet->m_pKeyFrameTimes[i];

				for (int j = 0; j < 12; ++j)
				{
					if (j == 2)			// Pelvis
					{
						if (i < pPistolShootAnimationSet->m_nKeyFrames)
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pPistolShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
						}
						else
						{
							pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
								= pPistolShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 20][j];
						}
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pWalkAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
				}

				for (int j = 12; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++j)
				{
					if (i < pPistolShootAnimationSet->m_nKeyFrames)
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pPistolShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j];
					}
					else
					{
						pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j]
							= pPistolShootAnimationSet->m_ppxmf4x4KeyFrameTransforms[i - 20][j];
					}
				}
			}
		}
	}

	return pLoadedModel;
}

CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, std::ifstream& fileStream,
	CShader* pPlayerMeshShader, CShader* pPlayerSkinnedMeshShader, int* pnSkinnedMeshes, CMaterial* pMaterial)
{
	CGameObject* pGameObject = NULL;
	pGameObject = new CGameObject();

	while (true)
	{
		char token{};
		fileStream.read((char*)&token, sizeof(char));

		if ('F' == token)
		{
			fileStream.read((char*)&pGameObject->m_nFrames, sizeof(int));

			char nStr{};
			char frameName[64] = { 0, };
			fileStream.read((char*)&nStr, sizeof(char));
			fileStream.read((char*)&pGameObject->m_pstrFrameName, sizeof(char) * nStr);
			pGameObject->m_pstrFrameName[nStr] = '\0';

			// Transform
			XMFLOAT3 xmf3Position{}, xmf3Rotation{}, xmf3Scale{};
			XMFLOAT4 xmf4Rotation{};
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

			if (pMeshInfo && pMeshInfo->m_nPositions > 0)
			{
				CPlayerMesh* pMesh = NULL;
				pMesh = new CPlayerMesh(pd3dDevice, pd3dCommandList, pMeshInfo);
				if (pMesh) pGameObject->SetMesh(pMesh);
				pGameObject->SetShader(pPlayerMeshShader);

				delete pMeshInfo;
			}
		}
		else if ('S' == token)
		{
			// Skinning Info
			if (pnSkinnedMeshes)
				(*pnSkinnedMeshes)++;

			CMeshLoadInfo* pMeshInfo = pGameObject->LoadMeshInfoFromFile(fileStream, 20.0f);

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList, pMeshInfo);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, fileStream);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			pGameObject->SetMesh(pSkinnedMesh);
			pGameObject->SetShader(pPlayerSkinnedMeshShader);
		}
		else if ('C' == token)
		{
			int nChildren = 0;
			fileStream.read((char*)&nChildren, sizeof(int));

			if (nChildren > 0)
			{
				for (int i = 0; i < nChildren; ++i) {
					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList,
						pd3dGraphicsRootSignature, fileStream, pPlayerMeshShader, pPlayerSkinnedMeshShader, pnSkinnedMeshes, pMaterial);

					if (pChild) pGameObject->SetChild(pChild);
				}
			}
		}
		else if ('m' == token)
		{
			CMaterial* pPlayerMaterial = new CMaterial;
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, fileStream, pGameObject, pPlayerMeshShader, pMaterial);
		}
		else if ('E' == token)
		{
			if (!pGameObject->m_pMaterial)
			{
				CMaterial* pMaterial = new CMaterial;
				pGameObject->SetMaterial(pMaterial);
			}

			break;
		}
	}

	pGameObject->SetIsActive(true);

	return pGameObject;
}

void CGameObject::LoadAnimationFromFile(std::ifstream& fileStream, CLoadedModelInfo* pLoadedModel)
{
	char nStr = 0;

	int nAnimationSets = 0;

	while (true)
	{
		char token[64] = { 0, };

		fileStream.read((char*)&nStr, sizeof(char));
		fileStream.read((char*)&token, sizeof(char) * nStr);

		if (!strcmp(token, "<AnimationSets>: "))
		{
			fileStream.read((char*)&nAnimationSets, sizeof(int));
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets + 4);
		}
		else if (!strcmp(token, "<FrameNames>: "))
		{
			fileStream.read((char*)&pLoadedModel->m_pAnimationSets->m_nBoneFrames, sizeof(int));
			pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches = new CGameObject * [pLoadedModel->m_pAnimationSets->m_nBoneFrames];

			for (int i = 0; i < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++i)
			{
				char frameName[64] = { 0, };
				fileStream.read((char*)&nStr, sizeof(char));
				fileStream.read((char*)&frameName, sizeof(char) * nStr);

				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[i] = pLoadedModel->m_pModelRootObject->FindFrame(frameName);
			}
		}
		else if (!strcmp(token, "<AnimationSet>: "))
		{
			int nAnimationSet = 0;
			fileStream.read((char*)&nAnimationSet, sizeof(int));

			char token[64] = { 0, };
			fileStream.read((char*)&nStr, sizeof(char));
			fileStream.read((char*)&token, sizeof(char) * nStr);

			float fLength = 0.0f;
			int nFramesPerSecond = 0;
			int nKeyFrames = 0;

			fileStream.read((char*)&fLength, sizeof(float));
			fileStream.read((char*)&nFramesPerSecond, sizeof(int));
			fileStream.read((char*)&nKeyFrames, sizeof(int));

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] =
				new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames,
					token);

			for (int i = 0; i < nKeyFrames; ++i)
			{
				char token[64] = { 0, };
				fileStream.read((char*)&nStr, sizeof(char));
				fileStream.read((char*)&token, sizeof(char) * nStr);

				if (!strcmp(token, "<Transforms>: "))
				{
					CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];

					int nKey = 0;
					float fKeyTime = 0.0f;

					fileStream.read((char*)&nKey, sizeof(int));
					fileStream.read((char*)&fKeyTime, sizeof(float));

					pAnimationSet->m_pKeyFrameTimes[i] = fKeyTime;

					for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++j)
					{
						fileStream.read((char*)&pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i][j], sizeof(XMFLOAT4X4));
					}
				}
			}
		}
		else if (!strcmp(token, "</AnimationSets> "))
		{
			break;
		}
	}
}

CMeshLoadInfo* CGameObject::LoadMeshInfoFromFile(std::ifstream& fileStream, float modelScaleFactor)
{
	CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

	fileStream.read((char*)&pMeshInfo->m_nPositions, sizeof(int));
	pMeshInfo->m_pxmf3Positions = new XMFLOAT3[pMeshInfo->m_nPositions];
	fileStream.read((char*)pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3) * pMeshInfo->m_nPositions);

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

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	std::ifstream& fileStream, CGameObject* pObj,CShader* pShader, CMaterial* pMaterial)
{
	int nMaterial = 0;
	char c{};

	fileStream.read((char*)&nMaterial, sizeof(int));
	fileStream.read((char*)&nMaterial, sizeof(int));
	fileStream.read((char*)&c, sizeof(char));

	CMaterial* pGameObjectMaterial = new CMaterial;
	//CTexture* pTexture= new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pGameObjectMaterial->SetTexture(pMaterial->m_pTexture);

	while (true)
	{
		std::string token;
		fileStream >> token;

		char whiteSpace = 0;
		fileStream.read((char*)&whiteSpace, sizeof(char));

		if (!token.compare("albedo"))				 // Color
		{
			fileStream.read((char*)&pGameObjectMaterial->m_xmf4Albedo, sizeof(XMFLOAT4));
		}
		else if (!token.compare("emissive"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_xmf4Emissive, sizeof(XMFLOAT4));
		}
		else if (!token.compare("specular"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_xmf4Specular, sizeof(XMFLOAT4));
		}
		else if (!token.compare("glossiness"))		// Float
		{
			fileStream.read((char*)&pGameObjectMaterial->m_fGlossiness, sizeof(float));
		}
		else if (!token.compare("smoothness"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_fSmoothness, sizeof(float));
		}
		else if (!token.compare("metalic"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_fMetallic, sizeof(float));
		}
		else if (!token.compare("specularHighlights"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_fSpecularHighlight, sizeof(float));
		}
		else if (!token.compare("glossyReflections"))
		{
			fileStream.read((char*)&pGameObjectMaterial->m_fGlossyReflection, sizeof(float));
		}
		else if (!token.compare("<AlbedoMap>"))			// Texture
		{
			char c = 0;
			fileStream.read((char*)&c, sizeof(char));

			fileStream >> pGameObjectMaterial->m_strTextureName;
			
			//std::string tmp = pMaterial->m_strTextureName;
			//tmp = "Textures/" + tmp + ".dds";

			//const char* cname = tmp.c_str();

			//const size_t cSize = strlen(cname) + 1;
			//wchar_t* wc = new wchar_t[cSize];
			//mbstowcs(wc, cname, cSize);

			//pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, wc, RESOURCE_TEXTURE2D, 0);
			//pShader->CreateShaderResourceViews(pd3dDevice, pTexture, 0, 4);

			//delete[] wc;
		}
		else if (!token.compare("</Material>"))
		{
			pObj->SetMaterial(pGameObjectMaterial);
			break;
		}
	}
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
	//speed *= (1.0f + m_fUpgradeBulletSpeed);
	speed *= m_fUpgradeBulletSpeed;

	velocity = m_xmf3Vector;
	velocity.x *= speed;
	velocity.y *= speed;
	velocity.z *= speed;

	position = Vector3::Add(position, velocity);

	if (position.x > 800.0f || position.x < -800.0f) {
		m_bActive = false;
		return;
	}

	if (position.z > 800.0f || position.z < -800.0f) {
		m_bActive = false;
		return;
	}

	if (position.y > 500.0f || position.y < -200.0f) {
		m_bActive = false;
		return;
	}

	CGameObject::SetPosition(position);
}

CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject()
{
	
	CCubeMeshDiffused* pSkyBoxMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 20.0f);
	SetMesh(pSkyBoxMesh);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Space.dds", RESOURCE_TEXTURE_CUBE, 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//pSkyBoxShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	//pSkyBoxShader->CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, 3);
	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, 3);
	SetShader(pSkyBoxShader);

	CMaterial* pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	SetMaterial(pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	//SetPosition(0, 50, 0);

	CGameObject::Render(pd3dCommandList, pCamera);
}

CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramePerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrameTransforms;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pKeyFrameTimes = new float[nKeyFrameTransforms];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrameTransforms];

	for (int i = 0; i < nKeyFrameTransforms; ++i)
		m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nSkinningBones];
}

CAnimationSet::~CAnimationSet()
{
	if (m_pKeyFrameTimes) delete[] m_pKeyFrameTimes;

	for (int i = 0; i < m_nKeyFrames; ++i)
		if (m_ppxmf4x4KeyFrameTransforms[i])
			delete[] m_ppxmf4x4KeyFrameTransforms[i];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();

	for (int i = 0; i < (m_nKeyFrames - 1); ++i)
	{
		if ((m_pKeyFrameTimes[i] <= fPosition) && (fPosition < m_pKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_pKeyFrameTimes[i]) / (m_pKeyFrameTimes[i + 1] - m_pKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i][nBone], t);
			break;
		}
	}

	if (fPosition >= m_pKeyFrameTimes[m_nKeyFrames - 1])
		xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

	return xmf4x4Transform;
}

CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSets = new CAnimationSet * [nAnimationSets];
}

CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_pAnimationSets[i]) delete m_pAnimationSets[i];
	if (m_pAnimationSets) delete[] m_pAnimationSets;

	if (m_ppBoneFrameCaches) delete[] m_ppBoneFrameCaches;
}

CAnimationTrack::~CAnimationTrack()
{
	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

float CAnimationTrack::UpdatePosition(float fTrackPosition, float fElapsedTime, float fAnimationLength)
{
	float fTrackElapsedTime = fElapsedTime * m_fSpeed;

	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		if (m_fPosition < 0.0f)
			m_fPosition = 0.0f;
		else
		{
			m_fPosition = fTrackPosition + fTrackElapsedTime;

			if (m_fPosition > fAnimationLength)
			{
				m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				return fAnimationLength;
			}
		}

		break;
	}
	case ANIMATION_TYPE_ONCE:
		m_fPosition = fTrackPosition + fTrackElapsedTime;
		if (m_fPosition > fAnimationLength)
			m_fPosition = fAnimationLength;

		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	return m_fPosition;
}

void CAnimationTrack::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationTrack::SetCallbackKey(int nKeyIndex, float fTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationTrack::SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void CAnimationTrack::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; ++i)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData)
					m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

CAnimationController::CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}
	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets) m_pAnimationSets->Release();

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
}

void CAnimationController::SetTracksEnable(int nAnimationTrack)
{
	for (int i = 0; i < m_nAnimationTracks; ++i)
	{
		if (i == nAnimationTrack)
		{
			SetTrackEnable(i, true);
		}
		else
		{
			SetTrackEnable(i, false);
			SetTrackPosition(i, 0.0f);
		}
	}
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::SetCallbackKeys(int nAnimationTrack, int nCallbackKeys)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetAnimationCallbackHandler(pCallbackHandler);
}

int CAnimationController::GetTrueEnableAnimationTrack()
{
	for (int i = 0; i < m_nAnimationTracks; ++i)
	{
		if (m_pAnimationTracks[i].m_bEnable)
			return i;
	}

	return -1;
}

void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject* pRootGameObject)
{
	m_fTime += fTimeElapsed;
	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Transform = Matrix4x4::Zero();

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Transform;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Transform = xmf4x4Transform;
				}
				m_pAnimationTracks[k].HandleCallback();
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

CSatellite::CSatellite(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z,
	CShader* pPlayerShader, CShader* pSkinnedPlayerShader, CMaterial* pMaterial)
{
	CLoadedModelInfo* pSatelliteModel = LoadModelAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"Models/satellite model data.bin", NULL, pPlayerShader, pSkinnedPlayerShader, pMaterial);
	SetChild(pSatelliteModel->m_pModelRootObject);
	
	SetScale(30.0f);
	SetPosition(XMFLOAT3(x, y, z));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSatellite::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

CAlienPlanet::CAlienPlanet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z, CShader* pPlayerShader, CShader* pSkinnedPlayerShader, CMaterial* pMaterial)
{
	CLoadedModelInfo* pAlienPlanetModel = LoadModelAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"Models/alien planet model data.bin", NULL, pPlayerShader, pSkinnedPlayerShader, pMaterial);
	SetChild(pAlienPlanetModel->m_pModelRootObject);

	SetScale(200.0f);
	SetPosition(XMFLOAT3(x, y, z));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CAlienPlanet::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

CDesertPlanet::CDesertPlanet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z, CShader* pPlayerShader, CShader* pSkinnedPlayerShader, CMaterial* pMaterial)
{
	CLoadedModelInfo* pDesertPlanetModel = LoadModelAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"Models/desert planet model data.bin", NULL, pPlayerShader, pSkinnedPlayerShader, pMaterial);
	SetChild(pDesertPlanetModel->m_pModelRootObject);

	SetScale(300.0f);
	SetPosition(XMFLOAT3(x, y, z));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CDesertPlanet::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

CFrozenPlanet::CFrozenPlanet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float x, float y, float z, CShader* pPlayerShader, CShader* pSkinnedPlayerShader, CMaterial* pMaterial)
{
	CLoadedModelInfo* pFrozenPlanetModel = LoadModelAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
		"Models/frozen planet model data.bin", NULL, pPlayerShader, pSkinnedPlayerShader, pMaterial);
	SetChild(pFrozenPlanetModel->m_pModelRootObject);

	SetScale(500.0f);
	SetPosition(XMFLOAT3(x, y, z));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFrozenPlanet::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}
