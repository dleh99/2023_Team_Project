﻿#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "Network.h"

extern int gameMode;

ID3D12DescriptorHeap* CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
	TitleUI[ID] = {700, 575, 700 + 250, 575 + 30};
	TitleUI[PW] = {700, 625, 700 + 250, 625 + 30};
	TitleUI[RoomNumber] = { 700, 675, 700 + 250, 675 + 30};

	m_xmf4GlobalAmbient = XMFLOAT4{};
	::ZeroMemory(m_sTitleTexts, sizeof(std::wstring*) * 3);
}

CScene::~CScene()
{

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char mapkey)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 5);

	// 조명
	BuildLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	m_pSkyBox->SetIsActive(true);

	// 가로 x 세로 x 깊이가 12 x 12 x 12인 정육면체 메쉬 생성
	CCubeMeshTextured* pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);
	CCubeMeshDiffused* BulletMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 2.0f);
	m_pBlockMesh = pCubeMesh;

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Textures/rock1.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetTexture(pTexture);

	m_nObjects = 50 * 50 * 10 + 2000;
	m_ppObjects = new CGameObject * [m_nObjects + 4000];
	m_nBlock = m_nObjects;

	m_pInstanceShader = new CInstancingShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get(), m_pBlockMesh, sizeof(Instance), m_nBlock);
	m_pInstanceShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	m_pInstanceShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//m_pInstanceShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 3);
	//m_pInstanceShader->CreateShaderResourceViews(pd3dDevice, pTexture, 0, 2);
	m_pInstanceShader->SetTexture(pTexture);
	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 2);

	AddBlocksByMapData(0, mapkey,true);

	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	m_pSceneShader = pShader;

	pBulletMesh = BulletMesh;

	CParticle* pParticle = new CParticle();

	pParticle->SetMesh(pBulletMesh);
	pParticle->SetShader(m_pSceneShader);

	pParticle->SetPosition(20, 20, 30);

	pParticle->SetIsActive(true);

	testobj = pParticle;


	//AddBlocksByMapData(pCubeMesh, pTShader, pMaterial, 0, mapkey);

	m_pDepthRenderShader = new CDepthRenderShader(this, m_pLights->m_pLights);
	DXGI_FORMAT pdxgiRtvFormats[1] = { DXGI_FORMAT_R32_FLOAT };
	m_pDepthRenderShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	m_pDepthRenderShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);

	m_pShadowShader = new CShadowMapShader(this);
	m_pShadowShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_pShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pDepthRenderShader->GetDepthTexture());

	m_pShadowMapToViewport = new CTextureToViewportShader();
	m_pShadowMapToViewport->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_pShadowMapToViewport->BuildObjects(pd3dDevice, pd3dCommandList, m_pDepthRenderShader->GetDepthTexture());

	m_pInstanceShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pDepthRenderShader->GetDepthTexture());
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if(m_pSkyBox) m_pSkyBox->Release();
		
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; ++j)
			if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

	if (m_pLights) delete m_pLights;
	if (m_pMaterials) delete m_pMaterials;
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; ++j)
			if (m_ppObjects[j])
				m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CScene::DisableObject(int bullet_id, int block_id, int p_id)
{
	DisableBlock(block_id);
	DisableBullet(bullet_id, p_id);

	//if (m_ppObjects[block_id]) {
	//	m_ppObjects[block_id]->SetIsActive(false);
	//	m_ppObjects[block_id]->SetPosition(99999, 99999, 99999);
	//	m_ppObjects[block_id]->m_bParticleActive = true;
	//	auto tmp = m_ppObjects[block_id]->GetWorldMatrix();
	//	XMFLOAT4X4 result;
	//	XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&tmp)));
	//	m_pInstance[block_id].worldMatrix = result;
	//}
	//for (int i{}; i < m_nObjects; ++i) {
	//	if (m_ppObjects[i]->GetObjectType() != TYPE_BULLET) continue;
	//	if (((CBulletObject*)m_ppObjects[i])->GetPlayerId() == p_id && ((CBulletObject*)m_ppObjects[i])->GetBulletId() == bullet_id) {
	//		//std::cout << "충돌 처리 및 삭제 완료" << std::endl;
	//		if(m_ppObjects[i])
	//			m_ppObjects[i]->SetIsActive(false);
	//		break;
	//	}
	//}
}

void CScene::DisableBlock(int block_id)
{
	if (m_ppObjects[block_id]) {
		m_ppObjects[block_id]->SetIsActive(false);
		m_ppObjects[block_id]->SetPosition(99999, 99999, 99999);
		m_ppObjects[block_id]->m_bParticleActive = true;

		auto pos = m_ppObjects[block_id]->m_pParticles[0]->GetPosition();
		cout << pos.x << " " << pos.y << " " << pos.z << " " << endl;

		auto tmp = m_ppObjects[block_id]->GetWorldMatrix();
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&tmp)));
		m_pInstance[block_id].worldMatrix = result;
	}
}

void CScene::DisableBullet(int bullet_id, int p_id)
{
	for (int i{}; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->GetObjectType() != TYPE_BULLET) continue;
		if (((CBulletObject*)m_ppObjects[i])->GetPlayerId() == p_id && ((CBulletObject*)m_ppObjects[i])->GetBulletId() == bullet_id) {
			//std::cout << "찾았다" << std::endl;
			if (m_ppObjects[i])
				m_ppObjects[i]->SetIsActive(false);
			//if (m_ppObjects[i]->GetIsActive() == false) std::cout << bullet_id << " 번 총알 삭제함" << endl;
			break;
		}
	}
}

ComPtr<ID3D12RootSignature> CScene::GetGraphicsRootSignature()
{
	return m_pd3dGraphicsRootSignature;
}

void CScene::BuildLightsAndMaterials()
{
	m_nLights = 1;
	m_pLights = new LIGHTS[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHTS) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, -0.5f, 1.0f);

	/*m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 1000.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(20.0f, 20.0f, 40.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);*/

	m_pMaterials = new MATERIALS;
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

void CScene::PrepareLightingAndRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());

	// 조명
	UpdateShaderVariables(pd3dCommandList);

	// 조명 리소스의 상수 버퍼 뷰를 쉐이더 변수에 연결(바인딩)
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(8, d3dcbLightsGpuVirtualAddress);

	// 재질 리소스의 상수 버퍼 뷰를 쉐이더 변수에 연결
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dcbMaterialsGpuVirtualAddress);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHTS) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
	
	int n = 0;
	::memcpy(m_pcbMappedMaterials, &n, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}

	if (m_pd3dcbMaterials)
	{
		m_pd3dcbMaterials->Unmap(0, NULL);
		m_pd3dcbMaterials->Release();
	}
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (static_cast<unsigned __int64>(::gnCbvSrvDescriptorIncrementSize) * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (static_cast<unsigned __int64>(::gnCbvSrvDescriptorIncrementSize) * nConstantBufferViews);

	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (static_cast<unsigned __int64>(::gnCbvSrvDescriptorIncrementSize) * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (static_cast<unsigned __int64>(::gnCbvSrvDescriptorIncrementSize) * nDescriptorHeapIndex);

	//m_ptexture = pTexture;

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
}

ComPtr<ID3D12RootSignature> CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRange[4];
	::ZeroMemory(pd3dDescriptorRange, sizeof(D3D12_DESCRIPTOR_RANGE) * 4);

	pd3dDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRange[0].NumDescriptors = 1;
	pd3dDescriptorRange[0].BaseShaderRegister = 0; //t0: gtxtTexture
	pd3dDescriptorRange[0].RegisterSpace = 0;
	pd3dDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRange[1].NumDescriptors = 1;
	pd3dDescriptorRange[1].BaseShaderRegister = 1; //t1: gtxtSkyCubeTexture
	pd3dDescriptorRange[1].RegisterSpace = 0;
	pd3dDescriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRange[2].NumDescriptors = 1;
	pd3dDescriptorRange[2].BaseShaderRegister = 2; //t2: AlbedoTexture
	pd3dDescriptorRange[2].RegisterSpace = 0;
	pd3dDescriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRange[3].NumDescriptors = MAX_DEPTH_TEXTURES;
	pd3dDescriptorRange[3].BaseShaderRegister = 3; //t3: Depth Buffer
	pd3dDescriptorRange[3].RegisterSpace = 0;
	pd3dDescriptorRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameter[11];
	::ZeroMemory(&pd3dRootParameter, sizeof(pd3dRootParameter));

	pd3dRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameter[0].Constants.Num32BitValues = 32;
	pd3dRootParameter[0].Constants.ShaderRegister = 0;		// GameObject
	pd3dRootParameter[0].Constants.RegisterSpace = 0;
	pd3dRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[1].Descriptor.ShaderRegister = 1;		// Camera
	pd3dRootParameter[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameter[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRange[0];
	pd3dRootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameter[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRange[1];
	pd3dRootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameter[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameter[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRange[2];
	pd3dRootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[5].Descriptor.ShaderRegister = 2; //Skinned Bone Offsets
	pd3dRootParameter[5].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[6].Descriptor.ShaderRegister = 3; //Skinned Bone Transforms
	pd3dRootParameter[6].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[7].Descriptor.ShaderRegister = 4; //Materials
	pd3dRootParameter[7].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameter[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[8].Descriptor.ShaderRegister = 5; //Lights
	pd3dRootParameter[8].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameter[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameter[9].Descriptor.ShaderRegister = 6; //ToLight
	pd3dRootParameter[9].Descriptor.RegisterSpace = 0;
	pd3dRootParameter[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameter[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameter[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameter[10].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRange[3];	// Depth Buffer
	pd3dRootParameter[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		//|D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[4];
	::ZeroMemory(d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC) * 4);

	d3dSamplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc[0].MipLODBias = 0;
	d3dSamplerDesc[0].MaxAnisotropy = 1;
	d3dSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc[0].MinLOD = 0;
	d3dSamplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc[0].ShaderRegister = 0;
	d3dSamplerDesc[0].RegisterSpace = 0;
	d3dSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDesc[1].MipLODBias = 0;
	d3dSamplerDesc[1].MaxAnisotropy = 1;
	d3dSamplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc[1].MinLOD = 0;
	d3dSamplerDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc[1].ShaderRegister = 1;
	d3dSamplerDesc[1].RegisterSpace = 0;
	d3dSamplerDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[2].MipLODBias = 0.0f;
	d3dSamplerDesc[2].MaxAnisotropy = 1;
	d3dSamplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	d3dSamplerDesc[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	d3dSamplerDesc[2].MinLOD = 0;
	d3dSamplerDesc[2].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc[2].ShaderRegister = 2;
	d3dSamplerDesc[2].RegisterSpace = 0;
	d3dSamplerDesc[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDesc[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDesc[3].MipLODBias = 0.0f;
	d3dSamplerDesc[3].MaxAnisotropy = 1;
	d3dSamplerDesc[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	d3dSamplerDesc[3].MinLOD = 0;
	d3dSamplerDesc[3].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc[3].ShaderRegister = 3;
	d3dSamplerDesc[3].RegisterSpace = 0;
	d3dSamplerDesc[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameter);
	d3dRootSignatureDesc.pParameters = pd3dRootParameter;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDesc);
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);

	if (pd3dErrorBlob != nullptr)
	{
		std::cout << (char*)pd3dErrorBlob->GetBufferPointer() << std::endl;
	}

	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	return pd3dGraphicsRootSignature;
}

bool CScene::OnPrecessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnPrecessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
			if (gameMode == 1)
			{
		case 0x30:
			m_pPlayer->UpgradePlayerSpeed();
			break;
		case 0x35:
			m_pPlayer->UpgradePlayerDamage();
			break;
		case 0x37:
			m_pPlayer->UpgradePlayerBulletSpeed();
			break;
		case 0x38:
			m_pPlayer->UpgradePlayerBullet();
			break;
		case 0x39:
			m_pPlayer->UpgradePlayerHp();
			break;
		case 0x4D:
			m_pPlayer->ConfirmPlayerMoney();
			break;
			}
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return false;
}

bool CScene::ProcessInput(UCHAR* pKeyBuffer)
{
	return false;
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]->GetIsActive())
			m_ppObjects[j]->Animate(fTimeElapsed);
		else
			if (m_ppObjects[j]->m_bParticleActive) {
				m_ppObjects[j]->AnimateParticles(fTimeElapsed);
				//cout << "파티클 움직임" << endl;
			}
	}


	// 조명 위치를 플레이어 위치와 방향에 맞게 조정
	//if (m_pLights)
	//{
	//	// 플레이어
	//	m_pLights->m_pLights[0].m_xmf3Position = m_pPlayer->GetPosition();
	//	m_pLights->m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();

	//	// 타 플레이어 1
	//	m_pLights->m_pLights[0].m_xmf3Position = m_pPlayer->GetPosition();
	//	m_pLights->m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();

	//	// 타 플레이어 2
	//	m_pLights->m_pLights[0].m_xmf3Position = m_pPlayer->GetPosition();
	//	m_pLights->m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();
	//}

	//for (int i = 0; i < m_nObjects; i++)
	//	for (int j = 0; j < m_nObjects; j++) {
	//		if (m_ppObjects[i]->GetIsActive()&& m_ppObjects[j]->GetIsActive()) {
	//			if (m_ppObjects[i]->GetObjectType() != m_ppObjects[j]->GetObjectType()){
	//				if (BSCollisionCheck(m_ppObjects[i]->GetPosition(), m_ppObjects[j]->GetPosition(),
	//					m_ppObjects[i]->GetBoundingRadius(), m_ppObjects[j]->GetBoundingRadius())) {

	//					m_ppObjects[i]->SetIsActive(false);
	//					m_ppObjects[j]->SetIsActive(false);
	//					//std::cout << i << " " << j << std::endl;
	//				}
	//			}
	//		}
	//	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{ 
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);
	
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pSkyBox)
		m_pSkyBox->Render(pd3dCommandList, pCamera);

	if (m_pInstanceShader)
		m_pInstanceShader->Render(pd3dCommandList, pCamera);

	if (m_pShadowShader) m_pShadowShader->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nBlock; i++) {
		if (m_ppObjects[i] && m_ppObjects[i]->m_bParticleActive) {
			m_ppObjects[i]->RenderParticles(pd3dCommandList, pCamera);
		}
	}

	/*if (m_pShadowMapToViewport) m_pShadowMapToViewport->Render(pd3dCommandList, pCamera);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);*/
}

void CScene::AddObjects(int type,XMFLOAT3 BulletPosition, XMFLOAT3 BulletVector, int p_id, int b_id, float bulletSpeed)
{
	CBulletObject* pBulletObject = new CBulletObject();
	pBulletObject->SetMesh(pBulletMesh);
	pBulletObject->SetShader(m_pSceneShader);

	XMFLOAT3 bullet_vector = BulletVector;
	/*bullet_vector.x *= (1.0f + m_pPlayer->GetUpgradeBulletSpeed());
	bullet_vector.y *= (1.0f + m_pPlayer->GetUpgradeBulletSpeed());
	bullet_vector.z *= (1.0f + m_pPlayer->GetUpgradeBulletSpeed());*/

	bullet_vector.x *= (1.0f + bulletSpeed);
	bullet_vector.y *= (1.0f + bulletSpeed);
	bullet_vector.z *= (1.0f + bulletSpeed);

	//bullet_vector = Vector3::ScalarProduct(bullet_vector, -1.f, false);
	pBulletObject->SetBulletVector(bullet_vector);
	pBulletObject->SetObjectType(TYPE_BULLET);	
	//pBulletObject->SetBoundingRadius(2.0f);

	pBulletObject->SetPlayerId(p_id);
	//pBulletObject->SetUpgradeBulletSpeed(m_vPlayers[p_id]->GetUpgradeBulletSpeed());
	pBulletObject->SetBulletId(b_id);

	//int index = FindEmptySlot();
	m_ppObjects[m_nObjects] = pBulletObject;
	m_ppObjects[m_nObjects]->SetPosition(BulletPosition);
	m_ppObjects[m_nObjects]->SetIsActive(true);

	//std::cout << "총알 생성" << std::endl;
	
	m_nObjects++;
}

int CScene::FindEmptySlot()
{
	//for (int i = 0; i < MAX_OBJ_COUNT; ++i) {
	//	if (m_ppObjects[i]->GetIsActive() == false) {
	//		return i;
	//		//std::cout << i << std::endl;
	//	}
	//}
	return 0;
}

bool CScene::BSCollisionCheck(XMFLOAT3 Position1, XMFLOAT3 Position2,float Radius1, float Radius2)
{
	float x = Position1.x - Position2.x;
	float y = Position1.y - Position2.y;
	float z = Position1.z - Position2.z;

	if (Radius1 + Radius2 - 4.0f> sqrt(x * x + y * y + z * z)) return true;

	return false;
}

void CScene::BuildText(ComPtr<ID2D1DeviceContext2> const m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory)
{
	m_dWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_CONDENSED,
		50,
		L"",
		&pTextFormat[0]
	);
	pTextFormat[0]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat[0]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_dWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_OBLIQUE,
		DWRITE_FONT_STRETCH_NORMAL,
		100,
		L"",
		&pTextFormat[1]
	);
	pTextFormat[1]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat[1]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_dWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_CONDENSED,
		13,
		L"",
		&pTextFormat[2]
	);
	pTextFormat[2]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat[2]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_dWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_MEDIUM,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_CONDENSED,
		20,
		L"",
		&pTextFormat[3]
	);
	pTextFormat[3]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat[3]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_dWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_MEDIUM,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_CONDENSED,
		20,
		L"",
		&pTextFormat[4]
	);
	pTextFormat[4]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	pTextFormat[4]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), SolidColorBrush[0].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), SolidColorBrush[1].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), SolidColorBrush[2].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), SolidColorBrush[3].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PaleVioletRed), SolidColorBrush[4].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), SolidColorBrush[5].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepSkyBlue), SolidColorBrush[6].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), SolidColorBrush[7].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray,0.6f), SolidColorBrush[8].GetAddressOf());
	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGreen), SolidColorBrush[9].GetAddressOf());

}

void CScene::Render2D(const ComPtr<ID2D1DeviceContext2>& m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory,
	float fTimeElapsed)
{
	// 제한시간
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(410, 0));
	m_fPlayTime -= fTimeElapsed;
	std::wstring min = std::to_wstring(int(m_fPlayTime) / 60) + L":";
	std::wstring sec = std::to_wstring(int(m_fPlayTime) % 60);
	if (int(m_fPlayTime) % 60 < 10) sec = L"0" + std::to_wstring(int(m_fPlayTime) % 60);
	if (int(m_fPlayTime) < 0) {
		min = L" 0:";
		sec = L"00";
		if (false == isEnd) {

			isEnd = true;
		}
	}
	std::wstring str = min + sec;
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[0].Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush[0].Get());

	// 점수
	str = std::to_wstring(m_pPlayer->GetPlayerScore());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 00));
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[0].Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush[0].Get());

	// 체력
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 675));
	if (!m_pPlayer->GetDeath()) {
		float portion = float(m_pPlayer->GetPlayerHP()) / float(100 + m_pPlayer->GetPlayerUpgradeHp());
		m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 300.0f * portion, 40), SolidColorBrush[6].Get());
		m_d2dDeviceContext->DrawRectangle(D2D1::RectF(0, 0, 300.0f, 40), SolidColorBrush[0].Get(), 2.0f);
	}
	else {
		m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 0, 40), SolidColorBrush[6].Get());
		m_d2dDeviceContext->DrawRectangle(D2D1::RectF(0, 0, 300.0f, 40), SolidColorBrush[0].Get(), 2.0f);
	}

	// 총알
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 650));

	if (m_pPlayer->GetBulletNum() < 10)
		str = L"0" + std::to_wstring(m_pPlayer->GetCurrentBulletNum()) + L"/" + std::to_wstring(30 + m_pPlayer->GetUpgradeBulletNum()); // L"/30";
	else str = std::to_wstring(m_pPlayer->GetCurrentBulletNum()) + L"/" + std::to_wstring(30 + m_pPlayer->GetUpgradeBulletNum()); // L"/30";
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[0].Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush[6].Get());

	//장전 UI
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(830, 650));
	if (m_pPlayer->GetPlayerReloading()) {
		float portion = m_pPlayer->m_fKeyDownTime / 2.0f;
		m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 135.0f * portion, 15), SolidColorBrush[6].Get());
	}
	//부스터 UI
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(525, 275));

	float deg = 5.5;
	float portion = m_pPlayer->m_fBoosterMount / 100.0f;
	for (int i = 0; i < 50; i++) {
		D2D1_POINT_2F p1 = { 100 * static_cast<float>(cos(deg)), 100 * static_cast<float>(sin(deg)) };
		D2D1_POINT_2F p2 = { 100 * static_cast<float>(cos(deg + 0.03)), 100 * static_cast<float>(sin(deg + 0.03)) };
		m_d2dDeviceContext->DrawLine(p1, p2, SolidColorBrush[8].Get(), 7.0f);
		float limit = 50.0f * m_pPlayer->m_fBoosterMount / 100.0f;
		if (limit > 50 - i) {
			m_d2dDeviceContext->DrawLine(p1, p2, SolidColorBrush[0].Get(), 7.0f);
		}
		deg += 0.03f;
	}
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(560, 300));
	str = L"연료";
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[2].Get(), D2D1::RectF(0, 0, 50, 50), SolidColorBrush[6].Get());
	//승리 패배 알림

	if (m_fPlayTime <= -1.5f) {
		if (GetGameResult()) {
			m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(400, 200));
			str = L"승리";
			m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
				pTextFormat[1].Get(), D2D1::RectF(0, 0, 200, 300), SolidColorBrush[0].Get());
		}
		else {
			m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(400, 200));
			str = L"패배";
			m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
				pTextFormat[1].Get(), D2D1::RectF(0, 0, 200, 300), SolidColorBrush[0].Get());
		}
	}
}

void CScene::RenderTitle(const ComPtr<ID2D1DeviceContext2>& m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory,
	float fTimeElapsed)
{
	m_fBlinkTime += fTimeElapsed * 2.0f;

	std::wstring karrotstr = *m_sTitleTexts[m_flag];
	wchar_t lastChar = karrotstr.back();
	if ((int)m_fBlinkTime % 2 && lastChar != '|') {
		karrotstr += '|';
	}
	else {
		if (!karrotstr.empty()) {
			if (lastChar == '|') {
				karrotstr.pop_back();
			}
		}
	}

	// BackGround
	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0, 0));
	//m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT), SolidColorBrush[6].Get());

	std::wstring str;

	// ID 입력란
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(590, 565));
	str = L"ID";
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[3].Get(), D2D1::RectF(0, 0, 100, 50), SolidColorBrush[0].Get());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(700, 575));
	m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 250, 30), SolidColorBrush[0].Get());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(705, 565));
	str = *m_sTitleTexts[ID];
	if (m_flag == ID) {
		m_d2dDeviceContext->DrawText(karrotstr.c_str(), static_cast<UINT32>(karrotstr.size()),
			pTextFormat[4].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[7].Get());
	}
	else {
		m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
			pTextFormat[4].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[7].Get());
	}

	// PW 입력란
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(590, 615));
	str = L"PW";
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[3].Get(), D2D1::RectF(0, 0, 100, 50), SolidColorBrush[0].Get());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(700, 625));
	m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 250, 30), SolidColorBrush[0].Get());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(705, 615));
	str = *m_sTitleTexts[PW];
	if (m_flag == PW) {
		m_d2dDeviceContext->DrawText(karrotstr.c_str(), static_cast<UINT32>(karrotstr.size()),
			pTextFormat[4].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[7].Get());
	}
	else {
		m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
			pTextFormat[4].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[7].Get());
	}

	//타이틀
	str = L"Block Crusher";
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(300, 150));
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[1].Get(), D2D1::RectF(0, 0, 450, 300), SolidColorBrush[0].Get());

	//팝업 메세지창
	str = L"  ";
	if (m_TitleError == LS_OUTOFROOM) {
		str = L"룸 번호는 0이상, 332 이하여야 합니다";
	}
	else if (m_TitleError == LS_FULLROOM) {
		str = L"선택하신 룸에서 게임이 진행중입니다";
	}
	else if (m_TitleError == LS_LOGIN_FAIL) {
		str = L"입력하신 ID 또는 비밀번호가 틀렸습니다";
	}
	else if (m_TitleError == LS_ALREADY_INGAME) {
		str = L"입력하신 ID의 계정이 이미 게임 실행 중입니다";
	}
	else if (m_TitleError == LS_SIGNUP) {
		str = L"새로 입력하신 ID. 자동으로 회완가입 되어 게임에 진입합니다";
	}
	else if (m_TitleError == LS_LOGIN_SUCCESS) {
		str = L"로그인 성공";
	}
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(705, 715));
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[4].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[1].Get());
}

void CScene::RenderLobby(const ComPtr<ID2D1DeviceContext2>& m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory,
	ComPtr<IDWriteFactory> m_dWriteFactory, float fTimeElapsed)
{
	// BackGround
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0, 0));
	m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT), SolidColorBrush[6].Get());

	// Matching Button
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 650));
	m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 300, 75), SolidColorBrush[0].Get());

	std::wstring str = L"Matching Start";
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 660));
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat[3].Get(), D2D1::RectF(0, 0, 300, 50), SolidColorBrush[6].Get());

	// Ranking
	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 50));
	//m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 200, 75), SolidColorBrush[0].Get());

	//str = L"Ranking";
	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 60));
	//m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
	//	pTextFormat[3].Get(), D2D1::RectF(0, 0, 200, 50), SolidColorBrush[6].Get());

	//Weapon Select
	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 200));
	//m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 200, 100), SolidColorBrush[0].Get());

	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 350));
	//m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 200, 100), SolidColorBrush[0].Get());

	//m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 500));
	//m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 200, 100), SolidColorBrush[0].Get());

}

int CScene::CCTitleUI()
{
	//cout << m_ptWinCursorMouse.x << " ";
	//cout << m_ptWinCursorMouse.y << endl;

	for (int i = 0; i < 3; i++) {
		if (IsPointInRectangle(m_ptWinCursorMouse, TitleUI[i])) {
			m_flag = i;
			m_fBlinkTime = 1;
			return i;
		}
	}
	return m_flag;
}

int CScene::AddBlocksByMapData(int nindex, char mapkey,bool first)
{
	std::ifstream in{ "Map/MapData3.bin", std::ios::binary };

	int** mapdata = new int* [50];
	for (int i = 0; i < 50; i++) {
		mapdata[i] = new int[50];
	}
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++)
			mapdata[i][j] = 0;
	}

	int x = 0;
	int y = 0;
	bool flag = false;

	while (in) {
		char c;
		in >> c;
		//std::cout << c << " ";
#ifdef USE_SERVER
		if (c == mapkey) {
			flag = true;
			continue;
		}
#else
		if (c == 'd') {
			flag = true;
			continue;
		}
#endif
		if (flag) {
			int num = c - 48;

			mapdata[x][y] = num;
			//std::cout << mapdata[x][y] << " x : " << x << " y : " << y << std::endl;

			y++;
			if (y >= 50) {
				y = 0;
				x++;
			}
			if (x >= 50) {
				break;
			}
		}
	}

	int cnt = nindex;

	if (first == true) {

		for (int i = 0; i < 50; ++i)
			for (int j = 0; j < 10; ++j)
				for (int k = 0; k < 50; ++k) {
					XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
					 -(float)j * 12.0f , -(float)k * 12.0f + 40.0f };
					CBlockObject* pBlockObject = new CBlockObject();
					pBlockObject->SetMesh(m_pBlockMesh);
					pBlockObject->SetIsActive(true);
					pBlockObject->SetObjectType(TYPE_BLOCK);

					m_ppObjects[cnt] = pBlockObject;
					m_ppObjects[cnt]->SetPosition(position);

					// 파티클 빌드
					m_ppObjects[cnt]->m_pParticles = new CGameObject * [pBlockObject->m_nParticle];
					for (int p = 0; p < m_ppObjects[cnt]->m_nParticle; ++p) {
						CParticle* pParticle = new CParticle();
						pParticle->SetMesh(m_pBlockMesh);
						pParticle->SetShader(m_pSceneShader);
						pParticle->SetPosition(position);
						pParticle->SetIsActive(true);
						m_ppObjects[cnt]->m_pParticles[p] = pParticle;
					}

					cnt++;
				}

		for (int i = 0; i < 50; ++i)
			for (int k = 0; k < 50; ++k) {
				for (int y = 0; y < mapdata[i][k]; ++y) {

					XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
					(float)y * 12.0f + 12.0f, -(float)k * 12.0f + 40.0f };

					CBlockObject* pBlockObject = new CBlockObject();
					pBlockObject->SetMesh(m_pBlockMesh);
					pBlockObject->SetIsActive(true);
					pBlockObject->SetObjectType(TYPE_BLOCK);

					m_ppObjects[cnt] = pBlockObject;
					m_ppObjects[cnt]->SetPosition(position);

					// 파티클 빌드
					m_ppObjects[cnt]->m_pParticles = new CGameObject * [pBlockObject->m_nParticle];
					for (int p = 0; p < m_ppObjects[cnt]->m_nParticle; ++p) {
						CParticle* pParticle = new CParticle();
						pParticle->SetMesh(m_pBlockMesh);
						pParticle->SetShader(m_pSceneShader);
						pParticle->SetPosition(position);
						pParticle->SetIsActive(true);

						m_ppObjects[cnt]->m_pParticles[p] = pParticle;
					}

					cnt++;
				}
			}
		m_nActiveBlock = cnt;

		for (int i = cnt; i < m_nObjects; ++i) {
			CBlockObject* pBlockObject = new CBlockObject();
			pBlockObject->SetMesh(m_pBlockMesh);

			pBlockObject->SetIsActive(true);
			pBlockObject->SetObjectType(TYPE_BLOCK);

			m_ppObjects[i] = pBlockObject;
		}
	}
	else {

		for (int i = 0; i < 50; ++i)
			for (int j = 0; j < 10; ++j)
				for (int k = 0; k < 50; ++k) {
					XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
					 -(float)j * 12.0f , -(float)k * 12.0f + 40.0f };

					m_ppObjects[cnt]->SetPosition(position);
					m_ppObjects[cnt]->SetIsActive(true);
					m_ppObjects[cnt]->m_bParticleActive = false;

					for (int p = 0; p < m_ppObjects[cnt]->m_nParticle; ++p) {
						m_ppObjects[cnt]->m_pParticles[p]->SetPosition(position);
					}

					cnt++;
				}

		for (int i = 0; i < 50; ++i)
			for (int k = 0; k < 50; ++k) {
				for (int y = 0; y < mapdata[i][k]; ++y) {

					XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
					(float)y * 12.0f + 12.0f, -(float)k * 12.0f + 40.0f };

					m_ppObjects[cnt]->SetPosition(position);
					m_ppObjects[cnt]->SetIsActive(true);
					m_ppObjects[cnt]->m_bParticleActive = false;

					for (int p = 0; p < m_ppObjects[cnt]->m_nParticle; ++p) {
						m_ppObjects[cnt]->m_pParticles[p]->SetPosition(position);
					}

					cnt++;
				}
			}
		m_nActiveBlock = cnt;

		for (int i = cnt; i < m_nBlock; ++i) {
			m_ppObjects[i]->SetPosition(99999, 99999, 99999);
			m_ppObjects[i]->SetIsActive(false);
		}
	}

	m_pInstance = m_pInstanceShader->GetInstancePointer();

	for (int i = 0; i < m_nBlock; ++i) {
		auto tmp = m_ppObjects[i]->GetWorldMatrix();
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&tmp)));
		m_pInstance[i].worldMatrix = result;
	}

	for (int i = 0; i < 50; ++i) {
		delete mapdata[i];
	}
	delete[] mapdata;

	std::cout << m_nActiveBlock << "개 블럭 추가" << std::endl;

	return m_nActiveBlock;
}
