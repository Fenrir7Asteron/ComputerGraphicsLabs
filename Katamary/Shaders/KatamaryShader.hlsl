struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
	float4 norm : NORMAL0;
    float2 tex : TEXCOORD0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
    float2 tex : TEXCOORD;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

Texture2D DiffuseMap : register(t0);
SamplerState Sampler : register(s0);

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    output.pos = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, input.pos)));
    output.col = (input.norm + 1.0) * 0.5;
    output.tex = input.tex;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	//return input.col;
    return DiffuseMap.Sample(Sampler, input.tex);
}