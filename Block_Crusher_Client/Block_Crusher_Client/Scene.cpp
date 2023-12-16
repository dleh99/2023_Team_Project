﻿#include "stdafx.h"
#include "Scene.h"
#include "Player.h"

//#include "Network.h"

CScene::CScene()
{

}

CScene::~CScene()
{

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char mapkey)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);


	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	// 가로 x 세로 x 깊이가 12 x 12 x 12인 정육면체 메쉬 생성
	CCubeMeshTextured* pCubeMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);
	CCubeMeshDiffused* BulletMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 2.0f);

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Textures/Dia_Cube.dds", RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Lava(Diffuse).dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetTexture(pTexture);

	CTexturedShader* pTShader = new CTexturedShader();
	pTShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	pTShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	pTShader->CreateShaderResourceViews(pd3dDevice, pTexture, 0, 2);

	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	m_pSceneShader = pShader;

	pBulletMesh = BulletMesh;

	AddBlocksByMapData(pCubeMesh, pTShader, pMaterial, 0, mapkey);
	std::cout << "추가된 블럭 : " << m_nblock << std::endl;
}

void CScene::ReleaseObjects()
{
	if(m_pSkyBox) m_pSkyBox->Release();
		
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; ++j)
			if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
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
	m_ppObjects[block_id]->SetIsActive(false);
	for (int i{}; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->GetObjectType() != TYPE_BULLET) continue;
		if (((CBulletObject*)m_ppObjects[i])->GetPlayerId() == p_id && ((CBulletObject*)m_ppObjects[i])->GetBulletId() == bullet_id) {
			//std::cout << "충돌 처리 및 삭제 완료" << std::endl;
			m_ppObjects[i]->SetIsActive(false);
			break;
		}
	}
}

void CScene::DisableBullet(int bullet_id, int p_id)
{
	for (int i{}; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->GetObjectType() != TYPE_BULLET) continue;
		if (((CBulletObject*)m_ppObjects[i])->GetPlayerId() == p_id && ((CBulletObject*)m_ppObjects[i])->GetBulletId() == bullet_id) {
			//std::cout << "찾았다" << std::endl;
			m_ppObjects[i]->SetIsActive(false);
			break;
		}
	}
}

ComPtr<ID3D12RootSignature> CScene::GetGraphicsRootSignature()
{
	return m_pd3dGraphicsRootSignature;
}

ComPtr<ID3D12RootSignature> CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRange[3];
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

	D3D12_ROOT_PARAMETER pd3dRootParameter[5];
	::ZeroMemory(&pd3dRootParameter, sizeof(pd3dRootParameter));
	pd3dRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameter[0].Constants.Num32BitValues = 16;
	pd3dRootParameter[0].Constants.ShaderRegister = 0;
	pd3dRootParameter[0].Constants.RegisterSpace = 0;
	pd3dRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameter[1].Constants.Num32BitValues = 32;
	pd3dRootParameter[1].Constants.ShaderRegister = 1;
	pd3dRootParameter[1].Constants.RegisterSpace = 0;
	pd3dRootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];

	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
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
	}

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
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());

	if (pCamera)
		pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pSkyBox)
		m_pSkyBox->Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] && m_ppObjects[j]->GetIsActive()) {
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::AddObjects(int type,XMFLOAT3 BulletPosition, XMFLOAT3 BulletVector, int p_id, int b_id)
{
	CBulletObject* pBulletObject = new CBulletObject();
	pBulletObject->SetMesh(pBulletMesh);
	pBulletObject->SetShader(m_pSceneShader);

	XMFLOAT3 bullet_vector = BulletVector;
	//bullet_vector = Vector3::ScalarProduct(bullet_vector, -1.f, false);
	pBulletObject->SetBulletVector(bullet_vector);
	pBulletObject->SetObjectType(TYPE_BULLET);
	//pBulletObject->SetBoundingRadius(2.0f);

	pBulletObject->SetPlayerId(p_id);
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
		&pTextFormat
	);
	// IDWriteTextFormat의 SetXxx 호출하여 포맷 속성 지정.
	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Tan), SolidColorBrush.GetAddressOf());
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
	if(int(m_fPlayTime) < 0) sec = L"00";

	std::wstring str = min + sec;
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat.Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush.Get());
	
	// 점수
	
	str = std::to_wstring(m_pPlayer->GetPlayerScore());

	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 00));
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat.Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush.Get());

	// 총알
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(800, 650));
	str = std::to_wstring(m_pPlayer->GetBulletNum()) + L"/30";
	m_d2dDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()),
		pTextFormat.Get(), D2D1::RectF(0, 0, 200, 100), SolidColorBrush.Get());

	// 체력
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(50, 675));

	float portion = float(m_pPlayer->GetPlayerHP()) / 100.0f;
	if (m_pPlayer->GetPlayerHP() > 0) {
		m_d2dDeviceContext->FillRectangle(D2D1::RectF(0, 0, 300.0f * portion, 50), SolidColorBrush.Get());
	}
}

int CScene::AddBlocksByMapData(CMesh* pMesh, CShader* pShader,CMaterial* pMaterial, int nindex)
int CScene::AddBlocksByMapData(CMesh* pMesh, CShader* pShader,CMaterial* pMaterial, int nindex, char mapkey)
{
	std::ifstream in{ "Map/MapData3.bin", std::ios::binary };

	//if (!in) std::cout << "뭐임 ㅅㅂ" << std::endl;

	int mapdata[50][50];

	int x = 0;
	int y = 0;
	bool flag = false;

	while (in) {
		char c;
		in >> c;
		//std::cout << c << " ";

		if (c == mapkey) {
			flag = true;
			continue;
		}

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

			m_nblock += num;
		}
	}
	
	m_nObjects = 50 * 50 * 10 + m_nblock;
	m_ppObjects = new CGameObject * [m_nObjects + 2000];

	int cnt = nindex;

	for (int i = 0; i < 50; ++i)
		for (int j = 0; j < 10; ++j)
			for (int k = 0; k < 50; ++k) {
				XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
				 -(float)j * 12.0f , -(float)k * 12.0f + 40.0f };
				CBlockObject* pBlockObject = new CBlockObject();
				pBlockObject->SetMesh(pMesh);
				pBlockObject->SetShader(pShader);
				pBlockObject->SetMaterial(pMaterial);
				pBlockObject->SetIsActive(true);

				m_ppObjects[cnt] = pBlockObject;
				m_ppObjects[cnt]->SetPosition(position);

				cnt++;
			}

	for (int i = 0; i < 50; ++i)
		for (int k = 0; k < 50; ++k) {
			for (int y = 0; y < mapdata[i][k]; ++y) {
				XMFLOAT3 position = { -(float)i * 12.0f + 20.0f,
				(float)y * 12.0f + 12.0f, -(float)k * 12.0f + 40.0f };

				//std::cout << position.x << " " << position.y << " " << position.z << std::endl;

				CBlockObject* pBlockObject = new CBlockObject();
				pBlockObject->SetMesh(pMesh);
				pBlockObject->SetShader(pShader);
				pBlockObject->SetMaterial(pMaterial);
				pBlockObject->SetIsActive(true);

				m_ppObjects[cnt] = pBlockObject;
				m_ppObjects[cnt]->SetPosition(position);
				//std::cout << cnt << "번째 추가?" << std::endl;
				
				cnt++;
			}
		}

	//for (int i = 0; i < 50; ++i) {
	//	for (int j = 0; j < 50; j++) {
	//		std::cout << mapdata[i][j] << " ";
	//	}
	//	std::cout << std::endl;
	//}

	return m_nblock;
}