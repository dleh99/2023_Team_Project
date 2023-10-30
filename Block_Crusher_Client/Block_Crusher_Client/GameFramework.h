#pragma once
#include "Timer.h"
#include "Scene.h"

class CGameFramework
{
private:
	HINSTANCE m_hInstance;				// 주 윈도우의 인스턴스 핸들
	HWND m_hWnd;						// 주 윈도우 핸들

	int m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	int m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	// MSAA 멤버들
	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;

	// 후면 버퍼 멤버들
	static const UINT m_nSwapChainBuffers = 2;
	UINT m_nSwapChainBufferIndex;

	// COM 객체 컨테이너들
	std::vector<ComPtr<IDXGIAdapter>> m_adapters;
	std::vector<ComPtr<IDXGIOutput>> m_outputs;

	// COM 객체들
	ComPtr<IDXGIFactory4> m_pdxgiFactory;
	ComPtr<IDXGIAdapter> m_pdxgiMainAdapter;
	ComPtr<IDXGIOutput> m_pdxgiOutput;
	ComPtr<ID3D12Device> m_pd3dDevice;

	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;

	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;

	// 리소스 멤버들
	ComPtr<ID3D12Resource> m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;

	ComPtr<ID3D12Resource> m_pd3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;

	// PSO 멤버
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;

	// Fence 멤버들
	ComPtr<ID3D12Fence> m_pd3dFence;
	UINT64 m_nFenceValue;
	HANDLE m_hFenceEvent;

	// 게임 프레임워크에서 사용할 타이머
	CGameTimer m_GameTimer;

	// 프레임 레이트를 주 윈도우의 캡션에 출력하기 위한 문자열
	_TCHAR m_pszFrameRate[50];

public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hWnd);
	void OnDestroy();

	void EnumOutputs();
	void CreateDirect3DDevice();
	void CreateCommandQueue();
	void CreateSwapChain();

	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ChangeSwapChainState();

public:
	void MoveToNextFrame();
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	CScene* m_pScene;
	CCamera* m_pCamera = NULL;
};

