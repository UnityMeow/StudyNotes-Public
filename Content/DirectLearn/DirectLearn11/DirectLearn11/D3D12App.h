#pragma once
#include "../../Common/d3dx12.h"
#include "../../Common/d3dUtil.h"
#include "../../Common/GameTimer.h"

using namespace Microsoft::WRL;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3D12App
{
protected:
    D3D12App(HINSTANCE hInstance);
    virtual ~D3D12App();
    D3D12App(const D3D12App& rhs) = delete;
    D3D12App& operator=(const D3D12App& rhs) = delete;
public:
	static D3D12App* GetApp();
	int Run();
	virtual bool Init(HINSTANCE hInstance, int nShowCmd);
	virtual void Draw();
	virtual void OnResize();
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	bool InitWindow(HINSTANCE hInstance, int nShowCmd);
	bool InitDirect3D();

	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();
	void CreateDescriptorHeap();
	void CreateRTV();
	void CreateDSV();
	void CreateViewPortAndScissorRect();

	void FlushCmdQueue();
	void CalculateFrameState();
	LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	static D3D12App* mApp;
	HINSTANCE mInstance;
	HWND mhMainWnd = 0;
	bool isAppPaused = false;
	bool isMinimized = false;
	bool isMaximized = false;
	bool isResizing = false;

	// 设备
	ComPtr<ID3D12Device> d3dDevice;
	ComPtr<IDXGIFactory4> DXGIFac;
	ComPtr<ID3D12Fence> fence;
	// 命令分配器
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	// 命令队列
	ComPtr<ID3D12CommandQueue> cmdQueue;
	// 命令列表
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	// 深度模板缓存资源
	ComPtr<ID3D12Resource> depthStencilBuffer;
	ComPtr<ID3D12Resource> swapChainBuffer[2];
	// 交换链
	ComPtr<IDXGISwapChain> swapChain;
	// RTV描述符堆
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	// DSV描述符堆
	ComPtr<ID3D12DescriptorHeap> dsvHeap;

	// 视口
	D3D12_VIEWPORT viewPort;
	// 裁剪矩形
	D3D12_RECT scissorRect;

	// 渲染目标缓冲区描述符
	UINT RTVDesSize = 0;
	// 深度模板缓冲区描述符
	UINT DSVDesSize = 0;
	// 常量缓冲区描述符、着色器资源缓冲描述符和随机访问缓冲描述符
	UINT CbvSrvUavDesSize = 0;
	UINT mCurrentBackBuffer = 0;

	// MSAA质量等级
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAAQL;
		
	//GameTime类实例声明
	GameTimer gt;

	float clientWidth = 1280;
	float clientHeight = 720;

	UINT64 mCurrentFence = 0;
};