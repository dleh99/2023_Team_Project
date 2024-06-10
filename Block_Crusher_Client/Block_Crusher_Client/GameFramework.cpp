#include "stdafx.h"
#include "GameFramework.h"

extern int gameMode;

CGameFramework::CGameFramework()
{
	m_hInstance = NULL;
	m_hWnd = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_nSwapChainBufferIndex = 0;

	m_hFenceEvent = NULL;
	m_nFenceValue = 0;

	_tcscpy_s(m_pszFrameRate, _T("Block Crusher ("));

	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_nFenceValues[i] = 0;
	m_pScene = NULL;

	m_ptOldCursorPos.x = 0;
	m_ptOldCursorPos.y = 0;
	m_ptWinCursorMouse.x = 0;
	m_ptWinCursorMouse.y = 0;

	wstring* id = new wstring(L"");
	wstring* pw = new wstring(L"");
	wstring* room = new wstring(L"");

	m_sTitleTexts[ID] = id;
	m_sTitleTexts[PW] = pw;
	m_sTitleTexts[RoomNumber] = room;
}

CGameFramework::~CGameFramework()
{
	for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
		delete m_vEnemyPlayers[i];
	}
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hWnd;

	EnumOutputs();
	CreateDirect3DDevice();
	CreateCommandQueue();

	CreateD3D11On12Device();
	CreateD2DDevice();

	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();

	//CreateRenderTargetViews();
	CreateDepthStencilView();

#ifdef USE_SERVER
	NetworkInit();
	/*while (!GetGameState()) {
		do_recv();
	}*/
#endif
	BuildObjects();
	SoundSetup();

	//HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)do_recv, (LPVOID)NULL, 0, NULL);

	/*if (gameMode == 0)
		cout << "서바이벌 모드로 시작합니다." << endl;
	else if (gameMode == 1)
		cout << "RPG 모드로 시작합니다." << endl;*/

	return true;
}

void CGameFramework::OnDestroy()
{
	NetCleanup();
}

void CGameFramework::EnumOutputs()
{
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)m_pdxgiFactory.GetAddressOf()))) {
		//td::cout << "DXGI Factory COM 객체 생성 실패 . . ." << '\n';
		return;
	}

	ComPtr<IDXGIAdapter> currentDxgiAdapter = nullptr;
	DXGI_ADAPTER_DESC currentAdapterDesc;
	::ZeroMemory(&currentAdapterDesc, sizeof(DXGI_ADAPTER_DESC));

	for (UINT i = 0;
		(m_pdxgiFactory->EnumAdapters(i, currentDxgiAdapter.GetAddressOf())) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		m_adapters.push_back(currentDxgiAdapter);
		currentDxgiAdapter->GetDesc(&currentAdapterDesc);
		//std::cout << "VRAM 용량 : " << currentAdapterDesc.DedicatedVideoMemory << '\n';
		//std::cout << "시스템 용량 : " << currentAdapterDesc.DedicatedSystemMemory << '\n';
	}
	//std::cout << '\n';
	m_pdxgiMainAdapter = m_adapters[0];		// 이 응용 프로그램은 default 어댑터를 사용

	ComPtr<IDXGIOutput> currentDxgiOutput = nullptr;
	DXGI_OUTPUT_DESC currentOutputDesc;
	::ZeroMemory(&currentOutputDesc, sizeof(DXGI_OUTPUT_DESC));

	for (UINT i = 0;
		(m_pdxgiMainAdapter->EnumOutputs(i, currentDxgiOutput.GetAddressOf())) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		m_outputs.push_back(currentDxgiOutput);
		currentDxgiOutput->GetDesc(&currentOutputDesc);
		//std::cout << "모니터 이름 : " << currentOutputDesc.DeviceName << '\n';
		//std::cout << "모니터 오른쪽 좌표 : " << currentOutputDesc.DesktopCoordinates.right
		//	<< ", 모니터 바닥 좌표 : " << currentOutputDesc.DesktopCoordinates.bottom << '\n';
		//std::cout << "모니터가 데스크톱에 붙어있는가? : " << currentOutputDesc.AttachedToDesktop << '\n';
	}
	std::cout << '\n';
}

void CGameFramework::CreateD3D11On12Device()
{
	// Create an 11 device wrapped around the 12 device and share 12's command queue.
	ComPtr<ID3D11Device> d3d11Device;

	D3D11On12CreateDevice(m_pd3dDevice.Get(),D3D11_CREATE_DEVICE_BGRA_SUPPORT,nullptr,0,
		reinterpret_cast<IUnknown**>(m_pd3dCommandQueue.GetAddressOf()),1,0,&d3d11Device,&m_d3d11DeviceContext,nullptr);

	// Query the 11On12 device from the 11 device.
	d3d11Device.As(&m_d3d11On12Device);
}

void CGameFramework::CreateD2DDevice()
{
	// Create D2D/DWrite components.
	D2D1_FACTORY_OPTIONS d2dFactoryOptions{};
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory);
	ComPtr<IDXGIDevice> dxgiDevice;
	m_d3d11On12Device.As(&dxgiDevice);
	m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);

	m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory);
}

void CGameFramework::CreateDirect3DDevice()
{
	ComPtr<ID3D12Device> pDevice;
	if (FAILED(D3D12CreateDevice(m_pdxgiMainAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device), (void**)pDevice.GetAddressOf())))
	{
		//std::cout << "이 컴퓨터의 어댑터는 Feature Level 12_0을 지원하지 않습니다." << '\n';
		//std::cout << "Warp 어댑터를 사용하여 디바이스 생성을 시도하겠습니다." << '\n';

		ComPtr<IDXGIAdapter> pWarpAdapter = nullptr;
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)pWarpAdapter.GetAddressOf());
		if (FAILED(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
			__uuidof(ID3D12Device), (void**)pDevice.GetAddressOf()))) {
			//std::cout << "이 컴퓨터는 Warp 어댑터를 이용한 Feature Level 12_0도 지원하지 않습니다." << '\n';
			//std::cout << "모든 Direct3D 디바이스의 생성에 실패했습니다." << '\n';
		}
		else {
			//std::cout << "Warp 어댑터를 이용한 Direct3D 디바이스의 생성 성공!" << '\n' << '\n';
			m_pd3dDevice = pDevice;
		}
	}
	else {
		//std::cout << "Default 어댑터를 이용한 Direct3D 디바이스의 생성 성공!" << '\n' << '\n';
		m_pd3dDevice = pDevice;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	::ZeroMemory(&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;

	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다.
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;

	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	//펜스를 생성하고 펜스 값을 0으로 설정한다.
	if (FAILED(m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		__uuidof(ID3D12Fence), (void**)m_pd3dFence.GetAddressOf())))
	{
		//std::cout << "Fence 객체 생성에 실패했습니다 . . ." << '\n' << '\n';
	}
	else
		//std::cout << "Fence 객체 생성 성공!!!" << '\n' << '\n';
	m_nFenceValue = 0;

	/*펜스와 동기화를 위한 이벤트 객체를 생성한다(이벤트 객체의 초기값을 FALSE이다).
	이벤트가 실행되면(Signal) 이벤트의 값을 자동적으로 FALSE가 되도록 생성한다.*/
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void CGameFramework::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	::ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	if (SUCCEEDED(m_pd3dDevice->CreateCommandQueue(&commandQueueDesc,
		__uuidof(ID3D12CommandQueue), (void**)m_pd3dCommandQueue.GetAddressOf()))) {
		//std::cout << "Command Queue 생성 성공!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command Queue 생성 실패 . . ." << '\n' << '\n';
	}

	if (SUCCEEDED(m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)m_pd3dCommandAllocator.GetAddressOf()))) {
		//std::cout << "Command Allocator 생성 성공!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command Allocator 생성 실패 . . ." << '\n' << '\n';
	}

	if (SUCCEEDED(m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(),
		NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_pd3dCommandList.GetAddressOf()))) {
		//std::cout << "Command List 생성 성공!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command List 생성 실패 . . ." << '\n' << '\n';
	}

	m_pd3dCommandList->Close();
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다.
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	//스왑체인을 생성한다.
	if (SUCCEEDED(m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(),
		&dxgiSwapChainDesc, (IDXGISwapChain**)m_pdxgiSwapChain.GetAddressOf())))
	{
		//std::cout << "Swap Chain 생성 성공!" << '\n' << '\n';
	}
	else
	{
		//std::cout << "Swap Chain 생성 실패 . . ." << '\n' << '\n';
	}
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	//“Alt+Enter” 키의 동작을 비활성화한다.
	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	float dpiX;
	float dpiY;
#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);
	CD3DX12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle{ m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void
			**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), NULL,
			d3dRtvCPUDescriptorHandle);
		
		//d3dRtvCPUDescriptorHandle.Offset(m_nDsvDescriptorIncrementSize);
		//d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;

		// Create a wrapped 11On12 resource of this back buffer. Since we are 
		// rendering all D3D12 content first and then all D2D content, we specify 
		// the In resource state as RENDER_TARGET - because D3D12 will have last 
		// used it in this state - and the Out resource state as PRESENT. When 
		// ReleaseWrappedResources() is called on the 11On12 device, the resource 
		// will be transitioned to the PRESENT state.
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		m_d3d11On12Device->CreateWrappedResource(
			m_ppd3dRenderTargetBuffers[i].Get(),
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_WrappedBackBuffers[i])
		);

		// Create a render target for D2D to draw directly to this back buffer.
		ComPtr<IDXGISurface> surface;
		m_WrappedBackBuffers[i].As(&surface);
		m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&m_d2dRenderTargets[i]
		);



		//m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator));
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	::ZeroMemory(&d3dClearValue, sizeof(D3D12_CLEAR_VALUE));
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::WaitForGpuComplete()
{
	//m_nFenceValue++;

	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::SoundSetup()
{
	//FMOD_System_Create(&m_SoundSystem, FMOD_VERSION);
	//FMOD_System_Init(m_SoundSystem, 10, FMOD_INIT_NORMAL, NULL);
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::Render2D()
{
	// Acquire our wrapped render target resource for the current back buffer.
	m_d3d11On12Device->AcquireWrappedResources(m_WrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);

	// Render text directly to the back buffer.
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_nSwapChainBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();

	// 2D 객체 랜더링
	if (m_pScene) {
		if (m_pScene->m_SceneState == 0)
			m_pScene->RenderTitle(m_d2dDeviceContext, m_d2dFactory, m_dWriteFactory, m_GameTimer.GetTimeElapsed());
		else if(m_pScene->m_SceneState == 1)
			m_pScene->Render2D(m_d2dDeviceContext, m_d2dFactory, m_dWriteFactory, m_GameTimer.GetTimeElapsed());
		else if (m_pScene->m_SceneState == 2)
			m_pScene->RenderLobby(m_d2dDeviceContext, m_d2dFactory, m_dWriteFactory, m_GameTimer.GetTimeElapsed());
	}
		
	m_d2dDeviceContext->EndDraw();

	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	m_d3d11On12Device->ReleaseWrappedResources(m_WrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	m_d3d11DeviceContext->Flush();
}

void CGameFramework::FrameAdvance()
{
	//타이머의 시간이 갱신되도록 하고 프레임 레이트를 계산한다.
	m_GameTimer.Tick(0.0f);

#ifdef USE_SERVER
	do_recv();

	if(!m_Buildingflag)
		if (m_pScene->m_SceneState == 1) {
			m_Buildingflag = true;
			if (m_Buildingflag) BuildPlayer();
		}

	for (int i{}; i < m_vEnemyPlayers.size(); ++i) {
		if (i == m_pPlayer->GetPlayerId()) continue;
		Pos otherPlayerPos = GetOtherPlayerPos(i);
		Mouse otherPlayerMouse = GetOtherPlayerMouse(i);
		//cout << "[" << i << "] " << otherPlayerPos.x << ", " << otherPlayerPos.y << ", " << otherPlayerPos.z << ", " << otherPlayerMouse.cx << ", " << otherPlayerMouse.cy << endl;
		m_vEnemyPlayers[i]->Rotate(otherPlayerMouse.cx);
		m_vEnemyPlayers[i]->SetPosition(XMFLOAT3(otherPlayerPos.x, otherPlayerPos.y, otherPlayerPos.z));
	}
#endif

	for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
		if (m_pPlayer)
			if (m_pPlayer->GetPlayerId() == i)
				m_pPlayer->SetDeath(m_vEnemyPlayers[i]->GetDeath());
	}

	if(m_pScene->m_SceneState == 1)
		ProcessInput();

	AnimateObjects();

	//명령 할당자와 명령 리스트를 리셋한다.
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	// 그림자 맵 생성
	m_pScene->PrepareLightingAndRender(m_pd3dCommandList.Get());
	m_pScene->m_pDepthRenderShader->PrepareShadowMap(m_pd3dCommandList.Get(), m_pCamera, m_vEnemyPlayers);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex].Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	/*현재 렌더 타겟에 대한 프리젠트가 끝나기를 기다린다.
	프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태(D3D12_RESOURCE_STATE_PRESENT)에서
	렌더 타겟 상태(D3D12_RESOURCE_STATE_RENDER_TARGET)로 바뀔 것이다.*/
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들)를 계산한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (static_cast<unsigned __int64>(m_nSwapChainBufferIndex) * m_nRtvDescriptorIncrementSize);

	//깊이-스텐실 서술자의 CPU 주소를 계산한다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//원하는 색상으로 렌더 타겟(뷰)을 지운다.
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	//원하는 값으로 깊이-스텐실(뷰)을 지운다.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//렌더 타겟 뷰(서술자)와 깊이-스텐실 뷰(서술자)를 출력-병합 단계(OM)에 연결한다.
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//렌더링 코드는 여기에 추가될 것이다.
	if (m_pScene) m_pScene->Render(m_pd3dCommandList.Get(), m_pCamera);

	if (m_pPlayer)
		m_pPlayer->Render(m_pd3dCommandList.Get(), m_pCamera);

	for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
		if (m_pPlayer)
			if (m_pPlayer->GetPlayerId() == i)
				continue;

		if (m_vEnemyPlayers[i])
			m_vEnemyPlayers[i]->Render(m_pd3dCommandList.Get(), m_pCamera);
	}

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//명령 리스트를 닫힌 상태로 만든다.
	hResult = m_pd3dCommandList->Close();

	//명령 리스트를 명령 큐에 추가하여 실행한다.
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//2D 오브젝트 랜더링
	Render2D();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

	if(true == m_pScene->GetisEnd())
		if (false == isEnd_framework) {
			isEnd_framework = true;
			send_score_packet(m_pPlayer->GetPlayerScore());
		}
	frame_num++;
	SetFrame(frame_num);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (static_cast<unsigned __int64>(::gnRtvDescriptorIncrementSize) * m_nSwapChainBuffers);

	m_pScene = new CScene();
	// 이 부분 옮겨야 함(맵)
	//m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList.Get(), GetMapKey());
	m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList.Get(), 'c');
	m_pScene->BuildText(m_d2dDeviceContext, m_d2dFactory, m_dWriteFactory);
	for (int i = 0; i < 3; i++)
		m_pScene->m_sTitleTexts[i] = m_sTitleTexts[i];

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(m_pd3dDevice.Get(), m_pd3dCommandList.Get(), L"Textures/SpaceMan_Rank_01_Black.dds", RESOURCE_TEXTURE2D, 0);

	CPlayerShader* pPlayerShader = new CPlayerShader();
	pPlayerShader->CreateShader(m_pd3dDevice.Get(), m_pScene->GetGraphicsRootSignature().Get());
	pPlayerShader->CreateShaderVariables(m_pd3dDevice.Get(), m_pd3dCommandList.Get());
	//pPlayerShader->CreateCbvSrvDescriptorHeaps(m_pd3dDevice.Get(), 0, 1);
	//pPlayerShader->CreateShaderResourceViews(m_pd3dDevice.Get(), pTexture, 0, 4);
	CScene::CreateShaderResourceViews(m_pd3dDevice.Get(), pTexture, 0, 4);

	CSkinnedAnimationPlayerShader* pSkinnedPlayerShader = new CSkinnedAnimationPlayerShader();
	pSkinnedPlayerShader->CreateShader(m_pd3dDevice.Get(), m_pScene->GetGraphicsRootSignature().Get());
	pSkinnedPlayerShader->CreateShaderVariables(m_pd3dDevice.Get(), m_pd3dCommandList.Get());
	//pSkinnedPlayerShader->CreateCbvSrvDescriptorHeaps(m_pd3dDevice.Get(), 0, 1);
	//pSkinnedPlayerShader->CreateShaderResourceViews(m_pd3dDevice.Get(), pTexture, 0, 4);
	CScene::CreateShaderResourceViews(m_pd3dDevice.Get(), pTexture, 0, 4);

	CMaterial* pMat = new CMaterial();
	pMat->SetTexture(pTexture);

#ifdef USE_SERVER
	SetScene(m_pScene);
	Pos p = GetStartPos();
	int id = GetNetworkPlayerId();
	
	// 이 부분 옮길까?
	CMainPlayer* pCubePlayer0 = new CMainPlayer(m_pd3dDevice.Get(), m_pd3dCommandList.Get(),
		m_pScene->GetGraphicsRootSignature().Get(), -100.f, 250.f, -440.f, pPlayerShader, pSkinnedPlayerShader, pMat);

	CMainPlayer* pCubePlayer1 = new CMainPlayer(m_pd3dDevice.Get(), m_pd3dCommandList.Get(),
		m_pScene->GetGraphicsRootSignature().Get(), -280.f, 250.f, -440.f, pPlayerShader, pSkinnedPlayerShader, pMat);

	CMainPlayer* pCubePlayer2 = new CMainPlayer(m_pd3dDevice.Get(), m_pd3dCommandList.Get(),
		m_pScene->GetGraphicsRootSignature().Get(), -460.f, 250.f, -440.f, pPlayerShader, pSkinnedPlayerShader, pMat);

	m_vEnemyPlayers.push_back(pCubePlayer0);
	m_vEnemyPlayers.push_back(pCubePlayer1);
	m_vEnemyPlayers.push_back(pCubePlayer2);

	for (int i = 0; i < m_vEnemyPlayers.size(); ++i)
		m_vEnemyPlayers[i]->SetPlayerId(i);

	SetPlayers(m_vEnemyPlayers);

	m_pPlayer = m_vEnemyPlayers[id];
#else
	CMainPlayer* pCubePlayer = new CMainPlayer(m_pd3dDevice.Get(), m_pd3dCommandList.Get(),
		m_pScene->GetGraphicsRootSignature().Get(), 0.0f, 10.0f, 0.0f, pPlayerShader, pSkinnedPlayerShader, pMat);

	m_pPlayer = pCubePlayer;
	CMaterial* pMaterial = new CMaterial;
	m_pPlayer->SetMaterial(pMaterial);

	m_pPlayer->SearchRifle(&(m_pPlayer->m_pRifle));
	m_pPlayer->SearchShotgun(&(m_pPlayer->m_pShotgun));
	m_pPlayer->SearchPistol(&(m_pPlayer->m_pPistol));
#endif
	m_pPlayer->m_ppObjects = m_pScene->m_ppObjects;
	m_pPlayer->SetBlockNum(m_pScene->m_nBlock);
	m_pPlayer->m_pScene = m_pScene;

	m_pScene->m_pPlayer = m_pPlayer;
	m_pScene->m_vPlayers = m_vEnemyPlayers;

	m_pCamera = m_pPlayer->GetCamera();

	m_pPlayer->Update(m_GameTimer.GetTimeElapsed(), NULL);

	for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
		if (m_vEnemyPlayers[i])
			m_vEnemyPlayers[i]->Update(m_GameTimer.GetTimeElapsed(), NULL);
	}

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	//if (m_pScene)m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::BuildPlayer()
{
	int id = GetNetworkPlayerId();

	m_pPlayer = m_vEnemyPlayers[id];

	m_pPlayer->m_ppObjects = m_pScene->m_ppObjects;
	m_pPlayer->SetBlockNum(m_pScene->m_nBlock);
	m_pPlayer->m_pScene = m_pScene;

	m_pScene->m_pPlayer = m_pPlayer;

	m_pCamera = m_pPlayer->GetCamera();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene)m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	//if (true == m_pPlayer->GetIsActive()) {
		static UCHAR pKeyBuffer[256];
		DWORD dwDirection = 0;

		if (::GetKeyboardState(pKeyBuffer))
		{
			//cout << pKeyBuffer << endl;

			if (pKeyBuffer[0x57] & 0xF0) dwDirection |= DIR_FORWARD;			// W
			if (pKeyBuffer[0x53] & 0xF0) dwDirection |= DIR_BACKWARD;			// S
			if (pKeyBuffer[0x41] & 0xF0) dwDirection |= DIR_LEFT;				// A
			if (pKeyBuffer[0x44] & 0xF0) dwDirection |= DIR_RIGHT;				// D
			if (pKeyBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
			//if (pKeyBuffer[VK_RETURN] & 0xF0) m_SceneState = 1;
			//if (pKeyBuffer[VK_ESCAPE] & 0xF0) m_SceneState = 0;
			if (pKeyBuffer[0x43] & 0xF0){
				dwDirection |= KEY_SHOOT;
				m_pPlayer->SetIsShoot(true);
			}

			if (pKeyBuffer[0x31] & 0xF0) m_pPlayer->ActiveRifle();				// 1
			if (pKeyBuffer[0x32] & 0xF0) m_pPlayer->ActiveShotgun();			// 2
			if (pKeyBuffer[0x33] & 0xF0) m_pPlayer->ActivePistol();				// 3

			//if (pKeyBuffer[0x38] & 0xF0) m_pPlayer->UpgradePlayerBullet();		// 8
			//if (pKeyBuffer[0x39] & 0xF0) m_pPlayer->UpgradePlayerHp();			// 9
		}

		float cxDelta = 0.0f, cyDelta = 0.0f;
		float x, y, z;
		x = y = z = 0.f;
		POINT ptCursorPos;

		if (::GetCapture() == m_hWnd)
		{
			//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
			::SetCursor(NULL);

			//현재 마우스 커서의 위치를 가져온다.
			::GetCursorPos(&ptCursorPos);

			//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;

			//마우스 커서의 위치를 마우스가 눌려졌던 위치로 설정한다.
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		//마우스 또는 키 입력이 있으면 플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
#ifdef USE_SERVER
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
			{
				m_pPlayer->SetIsRotate(true);
			}
			//x = m_pPlayer->GetPosition().x;
			//y = m_pPlayer->GetPosition().y;
			//z = m_pPlayer->GetPosition().z;

			//send_move_packet(x, y, z, cxDelta, cyDelta);
		}

		if (cxDelta == 0.f && cyDelta == 0.f)
		{
			if (m_pPlayer->GetIsRotate()) {
				m_pPlayer->SetIsRotate(false);
				//x = m_pPlayer->GetPosition().x;
				//y = m_pPlayer->GetPosition().y;
				//z = m_pPlayer->GetPosition().z;

				//send_move_packet(x, y, z, cxDelta, cyDelta);
			}
		}
		x = m_pPlayer->GetPosition().x;
		y = m_pPlayer->GetPosition().y;
		z = m_pPlayer->GetPosition().z;
		//cout << x << ", " << y << ", " << z << endl;
		//cout << m_pPlayer->GetAniState() << endl;

		if (m_pScene->m_SceneState == 1)
			send_move_packet(x, y, z, m_pPlayer->GetRotationRadian(), cyDelta, m_pPlayer->GetAniState());

		if (y < -150.f && false == m_pPlayer->GetDeath()) {
			send_fall_packet();
			m_pPlayer->SetDeath(true);
		}
		//cout << dwDirection << endl;
#endif
		if (!m_pPlayer->GetDeath())			// Alive
		{
			if (cxDelta || cyDelta)
			{
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우
				cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다).
			이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다.*/

			float fPlayerSpeed = 300.0f + m_pPlayer->GetUpgradeSpeed();
			if (dwDirection) m_pPlayer->Move(dwDirection, fPlayerSpeed * m_GameTimer.GetTimeElapsed(), true);
		}
		
		//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다.
		m_pPlayer->Update(m_GameTimer.GetTimeElapsed(), dwDirection);

		m_pPlayer->SetIsShoot(false);
		///////////////////////////////////////////////////////////////////////////////////////////////////

		for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
			if (m_pPlayer)
				if (m_pPlayer->GetPlayerId() == i)
					continue;

			if (m_vEnemyPlayers[i]) {
				m_vEnemyPlayers[i]->OtherPlayerAnimationUpdate(GetOtherAni(i));
				//cout << "[" << i << "] " << GetOtherAni(i) << endl;
			}
		}
	//}
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	if (m_pPlayer)
		m_pPlayer->Animate(fTimeElapsed);

	for (int i = 0; i < m_vEnemyPlayers.size(); ++i) {
		if (m_pPlayer)
			if (m_pPlayer->GetPlayerId() == i)
				continue;

		if (m_vEnemyPlayers[i])
			m_vEnemyPlayers[i]->Animate(fTimeElapsed);
	}
}

void CGameFramework::UpdateTitleText(int index)
{


		
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다.
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
		if (m_pScene) {
			m_pScene->m_ptWinCursorMouse.x = LOWORD(lParam);
			m_pScene->m_ptWinCursorMouse.y = HIWORD(lParam);
			m_flag = m_pScene->CCTitleUI();
		}
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다.
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnPrecessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_CHAR:
		if (wParam == 32) {  //입력한 문자에 따른 메세지 처리
				
		}
		else if (wParam == VK_RETURN) {

		}
		else if (wParam == VK_TAB) {
			m_flag = m_pScene->m_flag = (m_flag + 1) % 3;
		}
		else if(m_pScene->m_SceneState == 0) {
			char c = static_cast<char>(wParam);
			*m_sTitleTexts[m_flag] += c;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			m_pScene->m_SceneState = 0;
			::ShowCursor(true);
			::PostQuitMessage(0);
			break;
		case VK_RETURN: {
#ifdef USE_SERVER			
			//try {
			//	int num = stoi(*m_sTitleTexts[RoomNumber]);
			//	cout << "여기 들어옴" << endl;
			//	send_login_packet(*m_sTitleTexts[ID], *m_sTitleTexts[PW], num);
			//}
			//catch (const std::invalid_argument& e) {
			//	// stoi를 했을 때 int가 아닌 다른 값이 들어오는 경우 -> 경고 메시지 출력
			//	cout << "룸에 이상한 값 넣지 마세요" << endl;
			//}
			//catch (const std::out_of_range& e) {
			//	// stoi를 했을 때 범위를 벗어난 값이 들어오는 경우 -> 경고 메시지 출력
			//	cout << "0이상 332 이하의 값을 넣어주세요" << endl;
			//}

			// 타이틀일 때
			if (m_pScene->m_SceneState == 0) { 
				send_login_packet(*m_sTitleTexts[ID], *m_sTitleTexts[PW]);
			}
			// 로비일 때
			else if (m_pScene->m_SceneState == 2) {
				//m_pScene->m_SceneState = 1;
				::ShowCursor(false);
				send_match_packet();
				// 매칭시작 패킷
			}
#else
			if (m_pScene->m_SceneState == 0) {
				m_pScene->m_SceneState = 2;
			}
			else if(m_pScene->m_SceneState == 2) {
				m_pScene->m_SceneState = 1;
				::ShowCursor(false);
			}
#endif
			break;
		}
		case VK_F8:
			//m_flag += 1;
			break;
		case VK_F9:
			//“F9” 키가 눌려지면 윈도우 모드와 전체화면 모드의 전환을 처리한다. 
			//ChangeSwapChainState();
			break;
		case VK_BACK:
			if (!m_sTitleTexts[m_flag]->empty()) {
				m_sTitleTexts[m_flag]->pop_back();
				if (!m_sTitleTexts[m_flag]->empty())
					m_sTitleTexts[m_flag]->pop_back();
			}
			break;
		default:
			break;
		}
		break;
	//case WM_KEYDOWN:
	//	switch(wParam)
	//	{

	//	}
	//	break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	/*case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}*/
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_CHAR:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}

	return 0;
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	::ZeroMemory(&dxgiTargetParameters, sizeof(DXGI_MODE_DESC));
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dRenderTargetBuffers[i].Get())
			m_ppd3dRenderTargetBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,
		m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}