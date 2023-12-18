#pragma once
#include "Timer.h"
#include "Shader.h"
#include "protocol.h"
//#include "Network.h"

#define MAX_OBJ_COUNT 3000

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
	void Render2D(const ComPtr<ID2D1DeviceContext2>&  m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory,ComPtr<IDWriteFactory> m_dWriteFactory, float fTimeElapsed);
	void AddObjects(int type, XMFLOAT3 BulletPosition, XMFLOAT3 BulletVector, int p_id, int b_id);
	void ReleaseUploadBuffers();

	void DisableObject(int id_1, int id_2, int p_id);
	void DisableBullet(int bullet_id, int p_id);
	void BuildText(ComPtr<ID2D1DeviceContext2> const m_d2dDeviceContext, ComPtr<ID2D1Factory3> m_d2dFactory, ComPtr<IDWriteFactory> m_dWriteFactory);

	int FindEmptySlot();
	bool BSCollisionCheck(XMFLOAT3 Position1, XMFLOAT3 Position2, float Radius1, float Radius2);
	int AddBlocksByMapData(CMesh* pMesh, CShader* pShader, CMaterial* pMaterial,int nindex, char mapkey);

	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();
	
	CGameObject** m_ppObjects = NULL;
	int m_nBlock = 0;
	CPlayer* m_pPlayer= NULL;

	bool GetisEnd() { return isEnd; };

protected:
	/*CShader** m_ppShaders = NULL;
	int m_nShaders = 0;*/
	
	int m_nblock = 0;
	int m_nObjects = 0;
	int m_nRandObject = 0;

	float m_fPlayTime = 180.0f;

	CShader* m_pSceneShader;
	CCubeMeshDiffused* pBulletMesh = NULL;

	CSkyBox* m_pSkyBox = NULL;

	ComPtr<IDWriteTextFormat> pTextFormat[3];
	ComPtr<ID2D1SolidColorBrush> SolidColorBrush[10];

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;

	bool isEnd = false;
};

