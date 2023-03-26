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
    float4 worldPos : POSITIONT;
};

struct CascadeData
{
    float4x4 lightView[4];
    float4x4 lightProjection[4];
    float4 distances;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 transposeInverseWorldMatrix;
};

// Phong directional light parameters
cbuffer PS_CONSTANT_BUFFER : register(b1)
{
    float4 cameraPos;
    
    float4 lightDir;
    float4 lightColor;
    
    float4 kD;    
    float4 kS_alpha; // specular coefficient + shininess    
    float4 kA; 
    
    float4 DSAIntensity;
};

cbuffer CascadeData : register(b2)
{
    CascadeData cascadeData;
};

Texture2D DiffuseMap : register(t0);
Texture2D ShadowMap : register(t1);

SamplerState Sampler : register(s0);
SamplerComparisonState ShadowCompSampler : register(s1);

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    float4 modelPos = mul(worldMatrix, input.pos);
    output.pos = mul(projectionMatrix, mul(viewMatrix, modelPos));
    output.col = input.col;
    output.norm = mul(transposeInverseWorldMatrix, input.norm);
    output.tex = input.tex;
    output.worldPos = modelPos;
	
	return output;
}

float offset_lookup(float2 loc, float cmpDepth, float2 offset)
{
    float depth = ShadowMap.SampleCmpLevelZero(ShadowCompSampler, loc + offset, cmpDepth).x;
    return depth;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    float3 norm = normalize(input.norm.xyz);
    float3 lDir = normalize(lightDir.xyz);
    
    // Texture color
    float3 objectColor = DiffuseMap.Sample(Sampler, input.tex.xy);
       
    // Diffuse
    float3 diffuseColor = lightColor * kD * saturate(dot(norm, -lDir)) * DSAIntensity.x;
    
    // Specular
    float3 reflected = normalize(reflect(lDir, norm.xyz));
    float3 viewDir = normalize(cameraPos.xyz - input.worldPos.xyz);
    float3 specularColor = lightColor.xyz * kS_alpha.xyz * DSAIntensity.y * pow(saturate(dot(reflected, viewDir)), kS_alpha.w);
    
    // Ambient
    float3 ambientColor = lightColor * kA * DSAIntensity.z;
    
    
    // Cascade shadows
    //float4 lightSpacePos;    
    //for (int i = 0; i < 4; ++i)
    //{
    //    lightSpacePos = mul(cascadeData.lightProjection[i], mul(cascadeData.lightView[i], input.worldPos));
    //    lightSpacePos = lightSpacePos / lightSpacePos.w;
        
    //    float depthVal = abs(lightSpacePos.z);
    //    if (depthVal < cascadeData.distances[i])
    //    {
    //        break;
    //    }
    //}
        
    
    //float2 texCoords = (lightSpacePos.xy + float2(1.0f, 1.0f)) * 0.5f;
    //texCoords.y = 1.0f - texCoords.y;
    
    //float shWidth, shHeight;
    //ShadowMap.GetDimensions(shWidth, shHeight);
    
    //float bias = 1.0f;
    //float shadowCoeff = offset_lookup(texCoords, lightSpacePos.z + cameraPos.w - bias, float2(0.0f, 0.0f));
    
    //objectColor = objectColor * ((diffuseColor + specularColor) * shadowCoeff + ambientColor);
    
    objectColor = objectColor * ((diffuseColor + specularColor) + ambientColor);
    //return float4(texCoords.x, texCoords.y, 0.0f, 1.0f);
    //return float4(shadowCoeff, shadowCoeff, shadowCoeff, 1.0f);
    return float4(objectColor, 1.0f);
}