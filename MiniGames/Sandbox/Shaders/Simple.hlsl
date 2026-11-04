
struct ConstantData
{
	float4x4 WorldViewProj;
};

struct MeshConstData {
	float4x4 Transform;
	float4 Color;
};

cbuffer ConstBuf : register(b0) {
	ConstantData ConstData;
}

cbuffer ConstMeshBuf : register(b1) {
	MeshConstData MeshData;
}

struct VS_IN
{
	float4 pos : POSITION;
	float4 col : COLOR;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

PS_IN VSMain( VS_IN input, uint vId : SV_VertexID )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = mul(float4(input.pos.xyz, 1.0f), ConstData.WorldViewProj);
	output.col = input.col;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.col;
	return col;
}
