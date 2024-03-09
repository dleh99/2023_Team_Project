#pragma once
#include "Timer.h"
#include "Shader.h"
#include "..\..\Block_Crusher_Server\protocol.h"
//#include "Network.h"

#define MAX_OBJ_COUNT 3000

struct LIGHT
{
	XMFLOAT4 m_xmf4Ambient;
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular;
	XMFLOAT3 m_xmf3Position;
	float m_fFalloff;
	XMFLOAT3 m_xmf3Direction;
	float m_fTheta;
	XMFLOAT3 m_xmf3Attenuation;
	float m_fPhi;
	bool m_bEnable;
	int m_nType;
	float m_fRange;
	float padding;
};

struct LIGHTS
{
	LIGHT m_pLights[MAX_LIGHTS];
	XMFLOAT4 m_xmf4GlobalAmbient;
	int m_nLights;
};

struct MATERIALS
{
	MATERIAL m_pReflections[MAX_MATERIALS];
};

class CScene
{
public:
	CScene();
	~CScene();

	bool OnPrecessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnPrecessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char mapkey);
	void ReleaseObjects();

	bool ProcessInput(UCHAR* pKeyBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void AddObjects(int type, XMFLOAT3 BulletPosition, XMFLOAT3 BulletVector, int p_id, int b_id);
	void ReleaseUploadBuffers();

	void DisableObject(int id_1, int id_2, int p_id);
	void DisableBullet(int bullet_id, int p_id);

	void Render2D(const ComPtr<ID2D1DeviceContext2>& m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory, float fTimeElapsed);
	void RenderTitle(const ComPtr<ID2D1DeviceContext2>& m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory, float fTimeElapsed);
	//void UpdateTitleTexts();
	void BuildText(ComPtr<ID2D1DeviceContext2> const m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory);

	int FindEmptySlot();
	bool BSCollisionCheck(XMFLOAT3 Position1, XMFLOAT3 Position2, float Radius1, float Radius2);
	int AddBlocksByMapData(int nindex, char mapkey, bool first);
	int CCTitleUI();
	bool IsPointInRectangle(POINT pt, RECT rect) {
		return (pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top && pt.y <= rect.bottom);
	}

	//BoundingBox CalculateBoundingBox();

	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();
	
	int m_nObjects = 0;
	int m_nBlock = 0;
	int m_nActiveBlock = 0;
	CMesh* m_pBlockMesh = NULL;
	CGameObject** m_ppObjects = NULL;

	CPlayer* m_pPlayer= NULL;
	std::vector<CMainPlayer*> m_vPlayers;
	std::wstring* m_sTitleTexts[3];
	POINT m_ptWinCursorMouse;
	// 0 - tile 1 - main
	int m_SceneState = 0;

	bool GetisEnd() { return isEnd; };

protected:
	// Scene의 조명
	LIGHTS* m_pLights = NULL;
	int m_nLights = 0;

	XMFLOAT4 m_xmf4GlobalAmbient;
	
	// 조명을 나타내는 리소스와 리소스에 대한 포인터
	ID3D12Resource *m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	// Scene에 있는 객체들에 적용되는 재질
	MATERIALS* m_pMaterials = NULL;

	//재질을 나타내는 리소스와 리소스에 대한 포인터
	ID3D12Resource *m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;

public:
	void BuildLightsAndMaterials();

	// Scene에 있는 조명과 재질의 리소스를 생성하고 갱신
	void PrepareLightingAndRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

protected:
	/*CShader** m_ppShaders = NULL;
	int m_nShaders = 0;*/

	int m_nRandObject = 0;

	float m_fPlayTime = 120.0f;
	float m_fBlinkTime = 0;

	CShader* m_pSceneShader = NULL;
	CCubeMeshDiffused* pBulletMesh = NULL;

	CSkyBox* m_pSkyBox = NULL;

	ComPtr<IDWriteTextFormat> pTextFormat[5];
	ComPtr<ID2D1SolidColorBrush> SolidColorBrush[10];

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;

	Instance* m_pInstance = NULL;

	RECT TitleUI[3];
	int m_flag = ID;

	bool isEnd = false;

public:
	CInstancingShader* m_pInstanceShader = NULL;
	CDepthRenderShader* m_pDepthRenderShader = NULL;
	CShadowMapShader* m_pShadowShader = NULL;
	CTextureToViewportShader* m_pShadowMapToViewport = NULL;
};

