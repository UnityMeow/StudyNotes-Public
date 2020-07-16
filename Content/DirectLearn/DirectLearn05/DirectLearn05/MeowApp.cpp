#include "MeowApp.h"

MeowApp::MeowApp()
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

	BuildRootSignature();
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
	ThrowIfFailed(cmdAllocator->Reset());// 重复使用记录 命令 的相关内存
	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));// 复用命令列表及其内存

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

	cmdList->OMSetRenderTargets(1,//待绑定的RTV数量
		&rtvHandle,	//指向RTV数组的指针
		true,	//RTV对象在堆内存中是连续存放的
		&dsvHandle);	//指向DSV的指针

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
	//cmdList->SetGraphicsRootDescriptorTable(0, // 根参数的起始索引
	//	cbvHeap->GetGPUDescriptorHandleForHeapStart());

	//绘制顶点（通过索引缓冲区绘制）
	cmdList->DrawIndexedInstanced(sizeof(indices), // 每个实例要绘制的索引数
		1,	// 实例化个数
		0,	// 起始索引位置
		0,	// 子物体起始索引在全局索引中的位置
		0);	// 实例化的高级技术，暂时设置为0

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));// 从渲染目标到呈现
	// 完成命令记的关闭命令列表
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

void MeowApp::BuildRootSignature()
{
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
}
