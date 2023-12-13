#pragma once
#include "Timer.h"
#include "Shader.h"

#define MAX_OBJ_COUNT 3000

class CScene
{
public:
	CScene();
	~CScene();

	bool OnPrecessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnPrecessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR* pKeyBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void AddObjects(int type, XMFLOAT3 BulletPosition, XMFLOAT3 BulletVector, int p_id, int b_id);
	void ReleaseUploadBuffers();

	void DisableObject(int id_1, int id_2, int p_id);
	void DisableBullet(int bullet_id, int p_id);

	int FindEmptySlot();
	bool BSCollisionCheck(XMFLOAT3 Position1, XMFLOAT3 Position2, float Radius1, float Radius2);
	int AddBlocksByMapData(CMesh* pMesh, CShader* pShader, CMaterial* pMaterial,int nindex);

	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();
	
	CGameObject** m_ppObjects = NULL;

	CPlayer* m_pPlayer= NULL;

protected:
	/*CShader** m_ppShaders = NULL;
	int m_nShaders = 0;*/
	
	int m_nblock = 0;
	int m_nObjects = 0;
	int m_nRandObject = 0;

	CShader* m_pSceneShader;
	CCubeMeshDiffused* pBulletMesh = NULL;

	CSkyBox* m_pSkyBox = NULL;

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;

};

