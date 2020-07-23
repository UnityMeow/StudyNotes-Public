cbuffer cbPerObject : register(b0)
{
	float4x4 world;
};

cbuffer cbPass : register(b1)
{
	float4x4 viewProj;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VertMeow(VertexIn vin)
{	
	VertexOut o;
	float3 PosW = mul(float4(vin.PosL, 1.0f), world).xyz;
	o.PosH = mul(float4(PosW, 1.0f), viewProj);
	o.Color = vin.Color;
	return o;
}

float4 PixelHusky(VertexOut pin) : SV_Target0
{
	return pin.Color;
}