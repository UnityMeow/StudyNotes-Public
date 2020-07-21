cbuffer cbPerObject : register(b0)
{
	float4x4 worldViewProj;
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
	o.PosH = mul(float4(vin.PosL, 1.0f), worldViewProj);
	//o.PosH = float4(vin.PosL, 1);
	o.Color = vin.Color;
	return o;
}

float4 PixelHusky(VertexOut pin) : SV_Target0
{
	return pin.Color;
}