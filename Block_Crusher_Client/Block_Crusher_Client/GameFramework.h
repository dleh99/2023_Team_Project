#pragma once
#include "Timer.h"
#include "Scene.h"
#include "Player.h"
#include "Network.h"

class CGameFramework
{
private:
	HINSTANCE m_hInstance;				// �� �������� �ν��Ͻ� �ڵ�
	HWND m_hWnd;						// �� ������ �ڵ�

	int m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	int m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	// MSAA �����
	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;

	// �ĸ� ���� �����
	static const UINT m_nSwapChainBuffers = 2;
	UINT m_nSwapChainBufferIndex;

	// COM ��ü �����̳ʵ�
	std::vector<ComPtr<IDXGIAdapter>> m_adapters;
	std::vector<ComPtr<IDXGIOutput>> m_outputs;

	// COM ��ü��
	ComPtr<IDXGIFactory4> m_pdxgiFactory;
	ComPtr<IDXGIAdapter> m_pdxgiMainAdapter;
	ComPtr<IDXGIOutput> m_pdxgiOutput;
	ComPtr<ID3D12Device> m_pd3dDevice;

	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;

	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;

	//Directx11on12 for 2D UI
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
	ComPtr<IDWriteFactory> m_dWriteFactory;

	ComPtr<ID3D11Resource> m_WrappedBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[m_nSwapChainBuffers];

	UINT m_CurrBackbufferIndex = 0;

	// ���ҽ� �����
	ComPtr<ID3D12Resource> m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;

	ComPtr<ID3D12Resource> m_pd3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;

	// PSO ���
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;

	// Fence �����
	ComPtr<ID3D12Fence> m_pd3dFence;
	UINT64 m_nFenceValue;
	HANDLE m_hFenceEvent;

	// ���� �����ӿ�ũ���� ����� Ÿ�̸�
	CGameTimer m_GameTimer;

	// ������ ����Ʈ�� �� �������� ĸ�ǿ� ����ϱ� ���� ���ڿ�
	_TCHAR m_pszFrameRate[50];

	long long frame_num = 0;
	bool isEnd_framework = false;
	int m_flag = ID;

	bool m_Buildingflag = false;

public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hWnd);
	void OnDestroy();

	void EnumOutputs();
	void CreateDirect3DDevice();
	void CreateCommandQueue();
	void CreateSwapChain();

	void CreateD3D11On12Device();
	void CreateD2DDevice();

	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void BuildObjects();
	void BuildPlayer();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();
	void Render2D();
	void UpdateTitleText(int index);

	void WaitForGpuComplete();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ChangeSwapChainState();

	long long GetFrame() { return frame_num; };

	void SoundSetup();

public:
	void MoveToNextFrame();
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	CScene* m_pScene;
	CCamera* m_pCamera = NULL;
	CMainPlayer* m_pPlayer = NULL;
	vector<CMainPlayer*> m_vEnemyPlayers;
	
	CGameObject* m_pBackgroundObjects = NULL;
	int m_nBackgroundObjects = 0;

	wstring* m_sTitleTexts[3];
	POINT m_ptOldCursorPos; 
	POINT m_ptWinCursorMouse; 

};

