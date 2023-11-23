#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{

}

CScene::~CScene()
{

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT3 Cubes[1008] = {};

	int cnt = 0;
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j)
			for (int k = 0; k < 10; ++k) {
				Cubes[cnt].x = -(float)i * 12.0f + 20.0f;
				Cubes[cnt].y = -(float)j * 12.0f;
				Cubes[cnt].z = -(float)k * 12.0f + 40.0f;
				cnt++;
			}

	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k) {
				Cubes[cnt].x = 20.0f -12.0f * (i + 4);
				Cubes[cnt].y = 12.0f + 12.0f * j;
				Cubes[cnt].z = 40.0f -12.0f * (k + 4);
				cnt++;
			}

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	// 가로 x 세로 x 깊이가 12 x 12 x 12인 정육면체 메쉬를 생성한다.
	CCubeMeshDiffused* pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);

	m_nObjects = cnt;
	m_ppObjects = new CGameObject * [m_nObjects];

	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nObjects; ++i ) {
		CBlockObject* pBlockObject = new CBlockObject();
		pBlockObject->SetMesh(pCubeMesh);
		pBlockObject->SetShader(pShader);

		m_ppObjects[i] = pBlockObject;
		m_ppObjects[i]->SetPosition(Cubes[i]);
	}
}

void CScene::ReleaseObjects()
{
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

ComPtr<ID3D12RootSignature> CScene::GetGraphicsRootSignature()
{
	return m_pd3dGraphicsRootSignature;
}

ComPtr<ID3D12RootSignature> CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameter[2];
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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameter);
	d3dRootSignatureDesc.pParameters = pd3dRootParameter;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
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
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{ 
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());

	if (pCamera)
		pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}