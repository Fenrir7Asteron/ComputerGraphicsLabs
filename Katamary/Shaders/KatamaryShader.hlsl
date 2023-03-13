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
 	float4 col : COLOR;
 	float4 norm : NORMAL;
    float3 tex : TEXCOORD;
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
    output.col = input.col;
    output.norm = (input.norm + 1.0) * 0.5;
    output.tex = input.tex.xyz;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    input.norm = normalize(input.norm);
	//return input.norm;
    //return input.col * (1.0f - input.tex.z) + DiffuseMap.Sample(Sampler, input.tex.xy) * input.tex.z;
    return DiffuseMap.Sample(Sampler, input.tex.xy);
}