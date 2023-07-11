struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
	float4 norm : NORMAL0;
    float4 tex : TEXCOORD0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 lightView;
    float4x4 lightProjection;
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 transposeInverseWorldMatrix;
};

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    output.pos = mul(lightProjection, mul(lightView, mul(worldMatrix, input.pos)));
    
	return output;
}