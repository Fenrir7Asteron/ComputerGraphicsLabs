struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
};

cbuffer VS_CONSTANT_BUFFER0 : register(b0)
{
	float4 positionOffset;
};

cbuffer VS_CONSTANT_BUFFER1 : register(b1)
{
    float4 color;
};

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = input.pos;
	output.pos += positionOffset;
	
    output.col = color;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.col;
	return col;
}