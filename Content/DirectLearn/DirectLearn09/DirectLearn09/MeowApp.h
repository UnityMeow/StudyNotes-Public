#pragma once
#include "D3D12App.h"
#include <DirectXPackedVector.h>
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4X4 world = MathHelper::Identity4x4();
};

struct PassConstants
{
	XMFLOAT4X4 viewProj = MathHelper::Identity4x4();
};

class MeowApp : public D3D12App
{
	//绘制子物体的三个属性
	struct SubmeshGeometry
	{
		UINT indexCount;
		UINT startIndexLocation;
		UINT baseVertexLocation;
	};

	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	// 获取顶点、索引 数据大小
	UINT vbByteSize;
	UINT ibByteSize;

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
	std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;

	// PSO
	ComPtr<ID3D12PipelineState> PSO = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	XMFLOAT4X4 world = MathHelper::Identity4x4();
	XMFLOAT4X4 view = MathHelper::Identity4x4();
	XMFLOAT4X4 proj = MathHelper::Identity4x4();

	POINT lastMousePos;

	float theta = 1.5f * XM_PI;
	float phi = XM_PIDIV4;
	float radius = 10.0f;

public:
	MeowApp(HINSTANCE hInstance);
	~MeowApp();
	bool Init(HINSTANCE hInstance, int nShowCmd) override;
private:
	virtual void Draw()override;
	virtual void OnResize()override;
	void Update();
	// 创建描述符堆
	void CreateDescriptorHeap();
	// 创建常量缓冲区描述符CBV
	void CreateConstantBufferView();
	// 构建根签名
	void BuildRootSignature();
	// 输入布局描述和编译着色器字节码
	void BuildShadersAndInputLayout();
	// 构建几何体
	void BuildGeometry();
	// 构建PSO
	void BuildPSO();

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
};
