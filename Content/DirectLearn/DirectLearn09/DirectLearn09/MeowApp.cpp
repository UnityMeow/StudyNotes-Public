#include "MeowApp.h"

MeowApp::MeowApp(HINSTANCE hInstance)
	:
	D3D12App(hInstance)
{
}

MeowApp::~MeowApp()
{
}

bool MeowApp::Init(HINSTANCE hInstance, int nShowCmd)
{
	if (!D3D12App::Init(hInstance, nShowCmd))
		return false;
	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

	CreateDescriptorHeap();
	CreateConstantBufferView();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildTriangle();
	BuildPSO();

	ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdLists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCmdQueue();
	return true;
}

void MeowApp::Draw()
{
	Update();
	ThrowIfFailed(cmdAllocator->Reset());// 重复使用记录 命令 的相关内存
	// cmdList->SetPipelineState(PSO.Get());
	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), PSO.Get()));// 复用命令列表及其内存

	// 将后台缓冲资源从呈现状态转换到渲染目标状态 准备接收图像渲染
	UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),// 转换资源为后台缓冲区资源
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// 从呈现到渲染目标转换

	// 设置视口和裁剪矩形
	cmdList->RSSetViewports(1, &viewPort);
	cmdList->RSSetScissorRects(1, &scissorRect);


	// 清除后台缓冲区和深度缓冲区，并赋值
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, RTVDesSize);
	cmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::LightPink, 0, nullptr);// 清除RT背景色为XXX色，并且不设置裁剪矩形
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->ClearDepthStencilView(dsvHandle,	// DSV描述符句柄
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,	// FLAG
		1.0f,	// 默认深度值
		0,	// 默认模板值
		0,	// 裁剪矩形数量
		nullptr);	// 裁剪矩形指针
	
	// 指定将要渲染的缓冲区
	cmdList->OMSetRenderTargets(1,//待绑定的RTV数量
		&rtvHandle,	//指向RTV数组的指针
		true,	//RTV对象在堆内存中是连续存放的
		&dsvHandle);	//指向DSV的指针

	

	//设置CBV描述符堆
	ID3D12DescriptorHeap* descriHeaps[] = { cbvHeap.Get() };// 注意这里之所以是数组，是因为还可能包含SRV和UAV，而这里我们只用到了CBV
	cmdList->SetDescriptorHeaps(_countof(descriHeaps), descriHeaps);

	// 设置根签名
	cmdList->SetGraphicsRootSignature(rootSignature.Get());

	// 设置顶点缓冲区
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = vertexBufferGpu->GetGPUVirtualAddress();// 顶点缓冲区资源虚拟地址
	vbv.SizeInBytes = vbByteSize;	// 顶点缓冲区大小（所有顶点数据大小）
	vbv.StrideInBytes = sizeof(Vertex);	// 每个顶点元素所占用的字节数
	cmdList->IASetVertexBuffers(0, 1, &vbv);

	// 设置索引缓冲区
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = indexBufferGpu->GetGPUVirtualAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = ibByteSize;
	cmdList->IASetIndexBuffer(&ibv);

	// 将图元拓扑类型传入渲染管线
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 设置根描述符表
	//cmdList->SetGraphicsRootDescriptorTable(0, /* 根参数的起始索引 */cbvHeap->GetGPUDescriptorHandleForHeapStart());
	//设置根描述符表
	int objCbvIndex = 0;
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvHeap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(objCbvIndex, CbvSrvUavDesSize);
	cmdList->SetGraphicsRootDescriptorTable(0, //根参数的起始索引
		handle);

	int passCbvIndex = 1;
	handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvHeap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(passCbvIndex, CbvSrvUavDesSize);
	cmdList->SetGraphicsRootDescriptorTable(1, //根参数的起始索引
		handle);

	//绘制顶点（通过索引缓冲区绘制）
	cmdList->DrawIndexedInstanced(sizeof(indices), // 每个实例要绘制的索引数
		1,	// 实例化个数
		0,	// 起始索引位置
		0,	// 子物体起始索引在全局索引中的位置
		0);	// 实例化的高级技术，暂时设置为0

	// 再次转换RT资源
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));// 从渲染目标到呈现

	// 完成命令记得关闭命令列表
	ThrowIfFailed(cmdList->Close());

	// 等CPU将命令都准备好后，需要将待执行的命令列表加入GPU的命令队列
	ID3D12CommandList* commandLists[] = { cmdList.Get() };// 声明并定义命令列表数组
	cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);// 将命令从命令列表传至命令队列

	// 交换前后台缓冲区索引
	ThrowIfFailed(swapChain->Present(0, 0));
	// 1变0，0变1，为了让后台缓冲区索引永远为0
	ref_mCurrentBackBuffer = (ref_mCurrentBackBuffer + 1) % 2;

	// 设置fence值，刷新命令队列，使CPU和GPU同步
	FlushCmdQueue();
}

void MeowApp::OnResize()
{
	D3D12App::OnResize();

	// 构建投影矩阵
	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * 3.1415926f, (float)(clientWidth / clientHeight), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj, p);
}

void MeowApp::Update()
{
	ObjectConstants objConstants;
	PassConstants passConstants;
	
	float x = radius * sinf(phi) * cosf(theta);
	float y = radius * cosf(phi);
	float z = radius * sinf(phi) * sinf(theta);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	// 相机的世界坐标
	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);
	//XMStoreFloat4x4(&view, v);

	// 构建世界矩阵
	XMMATRIX w = XMLoadFloat4x4(&world);
	w *= XMMatrixTranslation(2.0f, 1.0f, 0.0f);

	// 构建投影矩阵
	// 摄像机的屏幕坐标
	XMMATRIX p = XMLoadFloat4x4(&proj);
	// 矩阵计算
	XMMATRIX WVP_Matrix = v * p;

	// XMMATRIX赋值给XMFLOAT4X4
	XMStoreFloat4x4(&passConstants.viewProj, /*由于CPU与GPU是相反的所以要逆一下*/XMMatrixTranspose(WVP_Matrix));
	passCB->CopyData(0, passConstants);

	// XMMATRIX赋值给XMFLOAT4X4
	XMStoreFloat4x4(&objConstants.world, /*由于CPU与GPU是相反的所以要逆一下*/XMMatrixTranspose(w));
	// 将数据拷贝至GPU缓存
	objCB->CopyData(0, objConstants);
	
}

void MeowApp::CreateDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 2;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&cbvHeap)));
}

void MeowApp::CreateConstantBufferView()
{
	objCB = std::make_unique<UploadBuffer<ObjectConstants>>(d3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	// 获得常量缓冲区首地址
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objCB->Resource()->GetGPUVirtualAddress();
	// 常量缓冲区子物体个数（子缓冲区个数）下标
	int objCbElementIndex = 0;
	cbAddress += (int64_t)objCbElementIndex * objCBByteSize;
	// CBV堆中的CBV元素索引
	int heapIndex = 0;
	// 获得CBV堆首地址
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart());
	// CBV句柄（CBV堆中的CBV元素地址）
	handle.Offset(heapIndex, CbvSrvUavDesSize);	
	// 创建CBV描述符
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;
	d3dDevice->CreateConstantBufferView(
		&cbvDesc,
		handle);


	passCB = std::make_unique<UploadBuffer<PassConstants>>(d3dDevice.Get(), 1, true);

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	// 获得常量缓冲区首地址
	D3D12_GPU_VIRTUAL_ADDRESS cbAddressP = passCB->Resource()->GetGPUVirtualAddress();
	// 常量缓冲区子物体个数（子缓冲区个数）下标
	int passCbElementIndex = 0;
	cbAddressP += (int64_t)passCbElementIndex * passCBByteSize;
	// CBV堆中的CBV元素索引
	int heapIndex1 = 1;
	// 获得CBV堆首地址
	auto handle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart());
	// CBV句柄（CBV堆中的CBV元素地址）
	handle1.Offset(heapIndex1, CbvSrvUavDesSize);
	// 创建CBV描述符
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc1;
	cbvDesc1.BufferLocation = cbAddressP;
	cbvDesc1.SizeInBytes = passCBByteSize;
	// 一个bug找一天  = = 
	d3dDevice->CreateConstantBufferView(
		&cbvDesc1,
		 handle1);
}

void MeowApp::BuildRootSignature()
{
	// 根参数可以是描述符表、根描述符、根常量
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	// 创建由单个CBV所组成的描述符表
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, // 描述符类型
		1, // 描述符数量
		0);// 描述符所绑定的寄存器槽号
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);


	// 创建由单个CBV所组成的描述符表
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, // 描述符类型
		1, // 描述符数量
		1);// 描述符所绑定的寄存器槽号
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);


	// 根签名由一组根参数构成
	CD3DX12_ROOT_SIGNATURE_DESC rootSig(2, // 根参数的数量 TODO: currently zero before constant buffer
		slotRootParameter, // 根参数指针
		0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 用单个寄存器槽来创建一个根签名，该槽位指向一个仅含有单个常量缓冲区的描述符区域
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSig, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);

	if (errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(d3dDevice->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)));
}

void MeowApp::BuildShadersAndInputLayout()
{
	mvsByteCode = d3dUtil::CompileShader(L"Meow.hlsl", nullptr, "VertMeow", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Meow.hlsl", nullptr, "PixelHusky", "ps_5_0");
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void MeowApp::BuildTriangle()
{
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &vertexBufferCpu));	// 创建顶点数据内存空间
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &indexBufferCpu));	// 创建索引数据内存空间

	CopyMemory(vertexBufferCpu->GetBufferPointer(), vertices.data(), vbByteSize);	// 将顶点数据拷贝至顶点系统内存中
	CopyMemory(indexBufferCpu->GetBufferPointer(), indices.data(), ibByteSize);	// 将索引数据拷贝至索引系统内存中

	// 数据从CPU内存拷贝至上传堆，再从上传堆拷贝至默认堆
	// 默认堆是GPU只读的，非常省资源，所以静态几何体的顶点缓冲区放默认堆来做优化，动态几何体直接放上传堆
	// CPU的数据不能直接传入默认堆，CPU写入数据必须依靠上传堆
	vertexBufferGpu = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(), cmdList.Get(), vertices.data(), vbByteSize, vertexBufferUploader);
	indexBufferGpu = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(), cmdList.Get(), indices.data(), ibByteSize, indexBufferUploader);
}

void MeowApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// 光栅器状态修改 
	/*	FillMode
		D3D12_FILL_MODE_WIREFRAME以线框模式渲染立方体 
		D3D12_FILL_MODE_SOLID以实体模式渲染立方体 
	*/
	// psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	/*	CullMode
		D3D12_CULL_MODE_FRONT剔除正面朝向的三角形
		D3D12_CULL_MODE_BACK剔除背面朝向的三角形
	*/
	// psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;	//0xffffffff,全部采样，没有遮罩
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;	//归一化的无符号整型
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;	//不使用4XMSAA
	psoDesc.SampleDesc.Quality = 0;	//不使用4XMSAA

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
}

void MeowApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	// 按下是鼠标坐标
	lastMousePos.x = x;
	lastMousePos.y = y;
	SetCapture(mhMainWnd);
}

void MeowApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MeowApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0) // 左键
	{
		// 将鼠标的移动距离换算成弧度，0.25为调节阈值
		float dx = XMConvertToRadians((float)(lastMousePos.x - x) * 0.25f);
		float dy = XMConvertToRadians((float)(lastMousePos.y - y) * 0.25f);
		// 计算鼠标没有松开前的累计弧度
		theta += dx;
		phi += dy;
		// 限制角度phi的范围在（0.1， Pi-0.1）
		phi = MathHelper::Clamp(phi, 0.2f, 3.1416f - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0) // 右键
	{
		// 将鼠标的移动距离换算成缩放大小，0.005为调节阈值
		float dx = 0.005f * (float)(x - lastMousePos.x);
		float dy = 0.005f * (float)(y - lastMousePos.y);
		// 根据鼠标输入更新摄像机可视范围半径
		radius += dx - dy;
		// 限制可视范围半径
		radius = MathHelper::Clamp(radius, 3.0f, 15.0f);
	}
	lastMousePos.x = x;
	lastMousePos.y = y;
}


