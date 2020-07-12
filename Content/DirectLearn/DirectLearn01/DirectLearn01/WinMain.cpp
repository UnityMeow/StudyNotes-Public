#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <windowsx.h>
#include <comdef.h>
#include "../../Common/d3dx12.h"

using namespace Microsoft::WRL;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

//某个窗口的句柄，ShowWindow和UpdateWindow函数均要调用此句柄
HWND mhMainWnd = 0;	
//窗口过程函数的声明,HWND是主窗口句柄
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	//当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = MainWndProc;	//指定窗口过程
	wc.cbClsExtra = 0;	//借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	//借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInstance;	//应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	//没有菜单栏
	wc.lpszClassName = L"MainWnd";	//窗口名
	//窗口类注册失败
	if (!RegisterClass(&wc))
	{
		//消息框函数，参数1：消息框所属窗口句柄，可为NULL。参数2：消息框显示的文本信息。参数3：标题文本。参数4：消息框样式
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	//窗口类注册成功
	RECT R;	//裁剪矩形
	R.left = 0;
	R.top = 0;
	R.right = 1280;
	R.bottom = 720;
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//根据窗口的客户区大小计算窗口的大小
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	//创建窗口,返回布尔值
	mhMainWnd = CreateWindow(L"MainWnd", L"DirectXMeow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, hight, 0, 0, hInstance, 0);
	//窗口创建失败
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	//窗口创建成功,则显示并更新窗口
	ShowWindow(mhMainWnd, nShowCmd);
	UpdateWindow(mhMainWnd);

	//消息循环
	//定义消息结构体
	MSG msg = { 0 };
	BOOL bRet = 0;
	//如果GetMessage函数不等于0，说明没有接受到WM_QUIT
	while (bRet = GetMessage(&msg, 0, 0, 0) != 0)
	{
		//如果等于-1，说明GetMessage函数出错了，弹出错误框
		if (bRet == -1)
		{
			MessageBox(0, L"GetMessage Failed", L"Errow", MB_OK);
		}
		//如果等于其他值，说明接收到了消息
		else
		{
			TranslateMessage(&msg);	//键盘按键转换，将虚拟键消息转换为字符消息
			DispatchMessage(&msg);	//把消息分派给相应的窗口过程
		}
	}
	return (int)msg.wParam;
}

/// <summary>
/// 窗口过程函数
/// </summary>
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//消息处理
	switch (msg)
	{
		//当窗口被销毁时，终止消息循环
	case WM_DESTROY:
		PostQuitMessage(0);	//终止消息循环，并发出WM_QUIT消息
		return 0;
	default:
		break;
	}
	//将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/// <summary>
/// 创建设备
/// </summary>
void CreateDevice()
{
	ComPtr<IDXGIFactory4> DXGIFac;
	CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFac));

	ComPtr<ID3D12Device> d3dDevice;
	D3D12CreateDevice(nullptr,		//此参数如果设置为nullptr，则使用主适配器
		D3D_FEATURE_LEVEL_12_0,		//应用程序需要硬件所支持的最低功能级别
		IID_PPV_ARGS(&d3dDevice));	//返回所建设备
}

/// <summary>
/// 创建Fence 为了CPU与GPU进行同步
/// 类似与多线程的互斥锁mutex
/// </summary>
void CreateFence(ComPtr<ID3D12Device>& d3dDevice)
{
	ComPtr<ID3D12Fence> fence;
	d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
}

/// <summary>
/// 获取描述符大小
/// 方便之后在地址中做偏移来找到堆中的描述符元素
/// </summary>
void GetDescriptorSize(ComPtr<ID3D12Device>& d3dDevice)
{
	// 渲染目标缓冲区描述符
	UINT RTVDesSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 深度模板缓冲区描述符
	UINT DSVDesSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	// 常量缓冲区描述符、着色器资源缓冲描述符和随机访问缓冲描述符
	UINT CbvSrvUavDesSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

/// <summary>
/// 设置MSAA抗锯齿属性
/// </summary>
void SetMSAA(ComPtr<ID3D12Device>& d3dDevice)
{
	// MSAA质量等级
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAAQL;

	// 初始化多重采样属性
	// 采样格式
	MSAAQL.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//UNORM是归一化处理的无符号整数
	// 多重采样数量
	MSAAQL.SampleCount = 1;
	// 多重采样模式
	MSAAQL.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	// 多重采样质量
	MSAAQL.NumQualityLevels = 0;

	d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MSAAQL, sizeof(MSAAQL));
}

/// <summary>
/// 创建命令队列、命令列表和命令分配器
/// </summary>
void CreateCommandObject(ComPtr<ID3D12Device>& d3dDevice)
{
	// 初始化命令描述符
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// 创建命令队列
	ComPtr<ID3D12CommandQueue> cmdQueue;
	d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&cmdQueue));
	// 创建命令分配器
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	// 创建命令列表
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	d3dDevice->CreateCommandList(0, //掩码值为0，单GPU
		D3D12_COMMAND_LIST_TYPE_DIRECT, //命令列表类型
		cmdAllocator.Get(),	//命令分配器接口指针
		nullptr,	//流水线状态对象PSO，这里不绘制，所以空指针
		IID_PPV_ARGS(&cmdList));	//返回创建的命令列表
	// 重置命令列表前必须将其关闭
	cmdList->Close();
}

/// <summary>
/// 创建交换链
/// 交换链存着渲染目标资源（后台缓冲区资源）
/// </summary>
void CreateSwapChain(ComPtr<IDXGIFactory4>& DXGIFac, ComPtr<ID3D12CommandQueue>& cmdQueue)
{
	ComPtr<IDXGISwapChain> swapChain;
	swapChain.Reset();
	
	// 初始化交换链描述符
	DXGI_SWAP_CHAIN_DESC swapChainDesc;	
	swapChainDesc.BufferDesc.Width = 1280;	// 缓冲区分辨率的宽度
	swapChainDesc.BufferDesc.Height = 720;	// 缓冲区分辨率的高度
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 缓冲区的显示格式
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	// 刷新率的分子
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	// 刷新率的分母
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// 逐行扫描VS隔行扫描(未指定的)
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// 图像相对屏幕的拉伸（未指定的）
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 将数据渲染至后台缓冲区（即作为渲染目标）
	swapChainDesc.OutputWindow = mhMainWnd;	// 渲染窗口句柄
	swapChainDesc.SampleDesc.Count = 1;	// 多重采样数量
	swapChainDesc.SampleDesc.Quality = 0;	// 多重采样质量
	swapChainDesc.Windowed = true;	//是否窗口化
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 固定写法
	swapChainDesc.BufferCount = 2;	// 后台缓冲区数量（双缓冲）
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//自适应窗口模式（自动选择最适于当前窗口尺寸的显示模式）
	// 创建交换链
	DXGIFac->CreateSwapChain(cmdQueue.Get(),	// 命令队列接口指针
		&swapChainDesc,		// 交换链描述符
		swapChain.GetAddressOf());		// 交换链地址
}

/// <summary>
/// 创建描述符堆
/// 描述符堆是存放描述符的一段连续内存空间
/// </summary>
void CreateDescriptorHeap(ComPtr<ID3D12Device>& d3dDevice)
{
	// 初始化RTV描述符堆描述符
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc;
	// 双缓冲 所以创建存放2个RTV的RTV堆
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	// 通过设备创建RTV描述符堆
	d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap));

	// 初始化DSV描述符堆描述符
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc;
	// 深度模板缓存 只有一个
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NodeMask = 0;
	ComPtr<ID3D12DescriptorHeap> dsvHeap;

	// 通过设备创建DSV描述符堆
	d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap));
}

/// <summary>
/// 创建RTV描述符
/// </summary>
void CreateRTV(ComPtr<ID3D12DescriptorHeap>& rtvHeap, ComPtr<IDXGISwapChain>& swapChain, ComPtr<ID3D12Device>& d3dDevice, UINT RTVDesSize)
{
	// 这个变体类在d3dx12.h头文件中定义，DX库并没有集成
	// 变体类，它的构造函数初始化了D3D12_CPU_DESCRIPTOR_HANDLE结构体中的元素
	// 从RTV堆中 拿到首个RTV句柄
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	
	ComPtr<ID3D12Resource> swapChainBuffer[2];
	for (int i = 0; i < 2; i++)
	{
		// 获得存于交换链中的后台缓冲区资源
		swapChain->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer[i].GetAddressOf()));
		// 创建RTV
		d3dDevice->CreateRenderTargetView(swapChainBuffer[i].Get(),
			nullptr,	// 在交换链创建中已经定义了该资源的数据格式，所以这里指定为空指针
			rtvHeapHandle);	// 描述符句柄结构体（这里是变体，继承自CD3DX12_CPU_DESCRIPTOR_HANDLE）
		// 偏移到描述符堆中的下一个缓冲区
		rtvHeapHandle.Offset(1, RTVDesSize);
	}
}

/// <summary>
/// 创建DSV描述符
/// </summary>
void CreateDSV(ComPtr<ID3D12DescriptorHeap>& dsvHeap, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAAQL, ComPtr<ID3D12Device>& d3dDevice)
{
	// 在CPU中创建好深度模板数据资源
	// 初始化DSV资源描述符
	D3D12_RESOURCE_DESC dsvResourceDesc;
	dsvResourceDesc.Alignment = 0;	// 指定对齐
	dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 指定资源维度（类型）为TEXTURE2D
	dsvResourceDesc.DepthOrArraySize = 1;	// 纹理深度为1
	dsvResourceDesc.Width = 1280;	// 资源宽
	dsvResourceDesc.Height = 720;	// 资源高
	dsvResourceDesc.MipLevels = 1;	// MIPMAP层级数量
	dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// 指定纹理布局（这里不指定）
	dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// 深度模板资源的Flag
	dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	dsvResourceDesc.SampleDesc.Count = 4;	// 多重采样数量
	dsvResourceDesc.SampleDesc.Quality = MSAAQL.NumQualityLevels - 1;

	CD3DX12_CLEAR_VALUE optClear;	// 清除资源的优化值，提高清除操作的执行速度（CreateCommittedResource函数中传入）
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// 24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	optClear.DepthStencil.Depth = 1;	// 初始深度值为1
	optClear.DepthStencil.Stencil = 0;	// 初始模板值为0

	// 创建深度模板缓存 资源
	ComPtr<ID3D12Resource> depthStencilBuffer;
	// 将深度模板数据提交至GPU显存中
	d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),	//堆类型为默认堆（不能写入）
		D3D12_HEAP_FLAG_NONE,	//Flag
		&dsvResourceDesc,	//上面定义的DSV资源描述符指针
		D3D12_RESOURCE_STATE_COMMON,	//资源的状态为初始状态
		&optClear,	//上面定义的优化值指针
		IID_PPV_ARGS(&depthStencilBuffer));

	d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(),
		nullptr,	//D3D12_DEPTH_STENCIL_VIEW_DESC类型指针，可填&dsvDesc（见上注释代码),由于在创建深度模板资源时已经定义深度模板数据属性，所以这里可以指定为空指针
		dsvHeap->GetCPUDescriptorHandleForHeapStart());	//DSV句柄
}
