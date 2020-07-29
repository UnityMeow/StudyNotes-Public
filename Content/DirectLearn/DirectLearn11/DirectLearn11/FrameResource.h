#pragma once
#include "../../Common/d3dx12.h"
#include "../../Common/d3dUtil.h"
#include "../../common//UploadBuffer.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace Microsoft::WRL;

// 定义顶点结构体
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

// 单个物体的物体常量数据（不变的）
struct ObjectConstants
{
	DirectX::XMFLOAT4X4 world = MathHelper::Identity4x4();
};

// 单个物体的过程常量数据（每帧变化）
struct PassConstants
{
	XMFLOAT4X4 viewProj = MathHelper::Identity4x4();
};

struct FrameResource
{
	//每帧资源都需要独立的命令分配器
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	//每帧都需要单独的资源缓冲区（此案例仅为2个常量缓冲区）
	std::unique_ptr<UploadBuffer<ObjectConstants>> objCB = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
	//CPU端的围栏值
	UINT64 fenceCPU = 0;

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator = (const FrameResource& rhs) = delete;
	FrameResource(ID3D12Device* device, UINT passCount, UINT objCount);
	~FrameResource();

};