struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
	float4 norm : NORMAL0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    output.pos = mul(mul(mul(input.pos, worldMatrix), viewMatrix), projectionMatrix);
    output.col = (input.norm + 1.0) * 0.5;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.col;
	return col;
}