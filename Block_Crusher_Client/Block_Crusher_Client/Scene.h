#pragma once
#include "Timer.h"
#include "Shader.h"

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

	void ReleaseUploadBuffers();

	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();

protected:
	/*CShader** m_ppShaders = NULL;
	int m_nShaders = 0;*/
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;
};
