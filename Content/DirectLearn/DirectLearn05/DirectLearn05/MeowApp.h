#pragma once
#include "D3D12App.h"
#include <DirectXPackedVector.h>
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMCOLOR Color;
};

//单个物体的常量数据
struct ObjectConstants
{
	//初始化物体空间变换到裁剪空间矩阵，Identity4x4是单位矩阵，需要包含MathHelper头文件
	XMFLOAT4X4 worldViewProj = MathHelper::Identity4x4();
	float gTime = 0.0f;
	XMFLOAT4 gPulseColor;
};

class MeowApp : public D3D12App
{
	// 顶点数据
	std::array<Vertex, 3> vertices =
	{
		Vertex({ XMFLOAT3(-1,-1,1), XMCOLOR(Colors::Red) }),
		Vertex({ XMFLOAT3(0.5,1,1), XMCOLOR(Colors::Green) }),
		Vertex({ XMFLOAT3(1,-1,1), XMCOLOR(Colors::Blue) }),
	};
	// 索引数据
	std::array<std::uint16_t, 3> indices =
	{
		0,1,2
	};
	
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	ComPtr<ID3DBlob> vertexBufferCpu = nullptr;	// CPU系统内存上的顶点数据
	ComPtr<ID3DBlob> indexBufferCpu = nullptr;	// CPU系统内存上的索引数据

	ComPtr<ID3D12Resource> vertexBufferGpu = nullptr;	// GPU默认堆中的顶点缓冲区（最终的GPU顶点缓冲区）
	ComPtr<ID3D12Resource> indexBufferGpu = nullptr;	// GPU默认堆中的索引缓冲区（最终的GPU索引缓冲区）

	ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;	// GPU上传堆中的顶点缓冲区
	ComPtr<ID3D12Resource> indexBufferUploader = nullptr;	// GPU上传堆中的索引缓冲区
	
	// 根签名
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

public:
	MeowApp();
	~MeowApp();
	bool Init(HINSTANCE hInstance, int nShowCmd) override;
private:
	virtual void Draw()override;
	// 创建常量缓冲区描述符CBV
	// void CreateConstantBufferView();
	void BuildRootSignature();
	// 构建几何体
	void BuildTriangle();
	// 构建PSO
	void BuildPSO();
};