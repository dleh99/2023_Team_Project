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
		cout << "�����̹� ���� �����մϴ�." << endl;
	else if (gameMode == 1)
		cout << "RPG ���� �����մϴ�." << endl;*/

	return true;
}

void CGameFramework::OnDestroy()
{
	NetCleanup();
}

void CGameFramework::EnumOutputs()
{
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)m_pdxgiFactory.GetAddressOf()))) {
		//td::cout << "DXGI Factory COM ��ü ���� ���� . . ." << '\n';
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
		//std::cout << "VRAM �뷮 : " << currentAdapterDesc.DedicatedVideoMemory << '\n';
		//std::cout << "�ý��� �뷮 : " << currentAdapterDesc.DedicatedSystemMemory << '\n';
	}
	//std::cout << '\n';
	m_pdxgiMainAdapter = m_adapters[0];		// �� ���� ���α׷��� default ����͸� ���

	ComPtr<IDXGIOutput> currentDxgiOutput = nullptr;
	DXGI_OUTPUT_DESC currentOutputDesc;
	::ZeroMemory(&currentOutputDesc, sizeof(DXGI_OUTPUT_DESC));

	for (UINT i = 0;
		(m_pdxgiMainAdapter->EnumOutputs(i, currentDxgiOutput.GetAddressOf())) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		m_outputs.push_back(currentDxgiOutput);
		currentDxgiOutput->GetDesc(&currentOutputDesc);
		//std::cout << "����� �̸� : " << currentOutputDesc.DeviceName << '\n';
		//std::cout << "����� ������ ��ǥ : " << currentOutputDesc.DesktopCoordinates.right
		//	<< ", ����� �ٴ� ��ǥ : " << currentOutputDesc.DesktopCoordinates.bottom << '\n';
		//std::cout << "����Ͱ� ����ũ�鿡 �پ��ִ°�? : " << currentOutputDesc.AttachedToDesktop << '\n';
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
		//std::cout << "�� ��ǻ���� ����ʹ� Feature Level 12_0�� �������� �ʽ��ϴ�." << '\n';
		//std::cout << "Warp ����͸� ����Ͽ� ����̽� ������ �õ��ϰڽ��ϴ�." << '\n';

		ComPtr<IDXGIAdapter> pWarpAdapter = nullptr;
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)pWarpAdapter.GetAddressOf());
		if (FAILED(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
			__uuidof(ID3D12Device), (void**)pDevice.GetAddressOf()))) {
			//std::cout << "�� ��ǻ�ʹ� Warp ����͸� �̿��� Feature Level 12_0�� �������� �ʽ��ϴ�." << '\n';
			//std::cout << "��� Direct3D ����̽��� ������ �����߽��ϴ�." << '\n';
		}
		else {
			//std::cout << "Warp ����͸� �̿��� Direct3D ����̽��� ���� ����!" << '\n' << '\n';
			m_pd3dDevice = pDevice;
		}
	}
	else {
		//std::cout << "Default ����͸� �̿��� Direct3D ����̽��� ���� ����!" << '\n' << '\n';
		m_pd3dDevice = pDevice;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	::ZeroMemory(&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x ���� ���ø�
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;

	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;

	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	//�潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�.
	if (FAILED(m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		__uuidof(ID3D12Fence), (void**)m_pd3dFence.GetAddressOf())))
	{
		//std::cout << "Fence ��ü ������ �����߽��ϴ� . . ." << '\n' << '\n';
	}
	else
		//std::cout << "Fence ��ü ���� ����!!!" << '\n' << '\n';
	m_nFenceValue = 0;

	/*�潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�(�̺�Ʈ ��ü�� �ʱⰪ�� FALSE�̴�).
	�̺�Ʈ�� ����Ǹ�(Signal) �̺�Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.*/
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
		//std::cout << "Command Queue ���� ����!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command Queue ���� ���� . . ." << '\n' << '\n';
	}

	if (SUCCEEDED(m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)m_pd3dCommandAllocator.GetAddressOf()))) {
		//std::cout << "Command Allocator ���� ����!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command Allocator ���� ���� . . ." << '\n' << '\n';
	}

	if (SUCCEEDED(m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(),
		NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_pd3dCommandList.GetAddressOf()))) {
		//std::cout << "Command List ���� ����!" << '\n' << '\n';
	}
	else {
		//std::cout << "Command List ���� ���� . . ." << '\n' << '\n';
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
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü��(�ĸ����)�� ũ�⿡ �°� �����Ѵ�.
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	//����ü���� �����Ѵ�.
	if (SUCCEEDED(m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(),
		&dxgiSwapChainDesc, (IDXGISwapChain**)m_pdxgiSwapChain.GetAddressOf())))
	{
		//std::cout << "Swap Chain ���� ����!" << '\n' << '\n';
	}
	else
	{
		//std::cout << "Swap Chain ���� ���� . . ." << '\n' << '\n';
	}
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	//��Alt+Enter�� Ű�� ������ ��Ȱ��ȭ�Ѵ�.
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

	// 2D ��ü ������
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
	//Ÿ�̸��� �ð��� ���ŵǵ��� �ϰ� ������ ����Ʈ�� ����Ѵ�.
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

	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	// �׸��� �� ����
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

	/*���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���.
	������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)����
	���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� �ٲ� ���̴�.*/
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (static_cast<unsigned __int64>(m_nSwapChainBufferIndex) * m_nRtvDescriptorIncrementSize);

	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//���ϴ� �������� ���� Ÿ��(��)�� �����.
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	//���ϴ� ������ ����-���ٽ�(��)�� �����.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�.
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//������ �ڵ�� ���⿡ �߰��� ���̴�.
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

	//��� ����Ʈ�� ���� ���·� �����.
	hResult = m_pd3dCommandList->Close();

	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//2D ������Ʈ ������
	Render2D();
	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.
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
	// �� �κ� �Űܾ� ��(��)
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
	
	// �� �κ� �ű��?
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
			//���콺 Ŀ���� ȭ�鿡�� ���ش�(������ �ʰ� �Ѵ�).
			::SetCursor(NULL);

			//���� ���콺 Ŀ���� ��ġ�� �����´�.
			::GetCursorPos(&ptCursorPos);

			//���콺 ��ư�� ���� ���¿��� ���콺�� ������ ���� ���Ѵ�.
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;

			//���콺 Ŀ���� ��ġ�� ���콺�� �������� ��ġ�� �����Ѵ�.
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		//���콺 �Ǵ� Ű �Է��� ������ �÷��̾ �̵��ϰų�(dwDirection) ȸ���Ѵ�(cxDelta �Ǵ� cyDelta).
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
				/*cxDelta�� y-���� ȸ���� ��Ÿ���� cyDelta�� x-���� ȸ���� ��Ÿ����. ������ ���콺 ��ư�� ������ ���
				cxDelta�� z-���� ȸ���� ��Ÿ����.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�).
			�̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�. �÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�.*/

			float fPlayerSpeed = 300.0f + m_pPlayer->GetUpgradeSpeed();
			if (dwDirection) m_pPlayer->Move(dwDirection, fPlayerSpeed * m_GameTimer.GetTimeElapsed(), true);
		}
		
		//�÷��̾ ������ �̵��ϰ� ī�޶� �����Ѵ�. �߷°� �������� ������ �ӵ� ���Ϳ� �����Ѵ�.
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
		//���콺 ĸ�ĸ� �ϰ� ���� ���콺 ��ġ�� �����´�.
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
		//���콺 ĸ�ĸ� �����Ѵ�.
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
		if (wParam == 32) {  //�Է��� ���ڿ� ���� �޼��� ó��
				
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
			//	cout << "���� ����" << endl;
			//	send_login_packet(*m_sTitleTexts[ID], *m_sTitleTexts[PW], num);
			//}
			//catch (const std::invalid_argument& e) {
			//	// stoi�� ���� �� int�� �ƴ� �ٸ� ���� ������ ��� -> ��� �޽��� ���
			//	cout << "�뿡 �̻��� �� ���� ������" << endl;
			//}
			//catch (const std::out_of_range& e) {
			//	// stoi�� ���� �� ������ ��� ���� ������ ��� -> ��� �޽��� ���
			//	cout << "0�̻� 332 ������ ���� �־��ּ���" << endl;
			//}

			// Ÿ��Ʋ�� ��
			if (m_pScene->m_SceneState == 0) { 
				send_login_packet(*m_sTitleTexts[ID], *m_sTitleTexts[PW]);
			}
			// �κ��� ��
			else if (m_pScene->m_SceneState == 2) {
				//m_pScene->m_SceneState = 1;
				::ShowCursor(false);
				send_match_packet();
				// ��Ī���� ��Ŷ
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
			//��F9�� Ű�� �������� ������ ���� ��üȭ�� ����� ��ȯ�� ó���Ѵ�. 
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