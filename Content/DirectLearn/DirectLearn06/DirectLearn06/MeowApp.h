#pragma once
#include "D3D12App.h"
#include <DirectXPackedVector.h>
#include "../../Common/UploadBuffer.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4 testColor;
};

class MeowApp : public D3D12App
{
	// 顶点数据
	std::array<Vertex, 3> vertices =
	{
		Vertex({ XMFLOAT3(-0.5f, -0.5f,0),XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(0.0f, 0.5f, 0),XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(0.5f, -0.5f, 0),XMFLOAT4(Colors::Blue) }),
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
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	// 根签名
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// CBV
	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objCB = nullptr;

	// PSO
	ComPtr<ID3D12PipelineState> PSO = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

public:
	MeowApp();
	~MeowApp();
	bool Init(HINSTANCE hInstance, int nShowCmd) override;
private:
	virtual void Draw()override;
	// 创建描述符堆
	void CreateDescriptorHeap();
	// 创建常量缓冲区描述符CBV
	void CreateConstantBufferView();
	// 构建根签名
	void BuildRootSignature();
	// 输入布局描述和编译着色器字节码
	void BuildShadersAndInputLayout();
	// 构建几何体
	void BuildTriangle();
	// 构建PSO
	void BuildPSO();
};