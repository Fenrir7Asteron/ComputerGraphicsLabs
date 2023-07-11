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
    float4 tex : TEXCOORD;
    float4 worldPos : POSITION1;
    float4 viewPos : POSITION2;
};

struct PS_OUT
{
    float4 worldPos_Depth : SV_Target0; // xyz - world position, w - world view depth
    float4 norm : SV_Target1;
    float4 albedoCol : SV_Target2;
    float4 diffuseCoeff: SV_Target3;
    float4 specularCoeff : SV_Target4;
    float4 ambientCoeff : SV_Target5;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 transposeInverseWorldMatrix;
};

// Phong material coefficients
cbuffer PS_CONSTANT_BUFFER : register(b1)
{        
    float4 kD;    
    float4 kS_alpha; // specular coefficient + shininess    
    float4 kA;     
};

Texture2D DiffuseMap : register(t0);

SamplerState Sampler : register(s0);

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    float4 modelPos = mul(worldMatrix, input.pos);
    float4 viewPos = mul(viewMatrix, modelPos);
    output.pos = mul(projectionMatrix, viewPos);
    output.col = input.col;
    output.norm = mul(transposeInverseWorldMatrix, input.norm);
    output.tex = input.tex;
    output.worldPos = modelPos;
    output.viewPos = viewPos;
	
	return output;
}

PS_OUT PSMain(PS_IN input)
{
    PS_OUT psOut;
    float3 norm = normalize(input.norm.xyz);
    
    psOut.worldPos_Depth.xyz = input.worldPos.xyz;
    psOut.worldPos_Depth.w = input.viewPos.z;
    psOut.norm = float4(norm, 1.0f);
    
    psOut.albedoCol = DiffuseMap.Sample(Sampler, input.tex.xy);
    psOut.diffuseCoeff = kD;
    psOut.specularCoeff = kS_alpha;
    psOut.ambientCoeff = kA;
    return psOut;
}