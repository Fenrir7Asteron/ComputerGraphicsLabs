struct VS_IN
{
	float4 pos : POSITION0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
};

// Phong directional light parameters
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 cameraPos;    
    float4 lightPos; // xyz = position, w = radius
    float4 lightColor;
    float4 DSAIntensity;
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

Texture2D WorldPosDepthMap : register(t0);
Texture2D NormalMap        : register(t1);
Texture2D AlbedoMap        : register(t2);
Texture2D DiffuseCoeffMap  : register(t3);
Texture2D SpecularCoeffMap : register(t4);

struct GBufferData
{
    float4 WorldPos_Depth;
    float3 Normal;
    float3 AlbedoCol;
    float3 DiffuseCoeff;
    float4 SpecularCoeff;
};

GBufferData ReadGBuffer(float2 screenPos)
{
    GBufferData buf = (GBufferData) 0;
    
    buf.WorldPos_Depth = WorldPosDepthMap.Load(float3(screenPos, 0.0f));
    buf.Normal = NormalMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.AlbedoCol = AlbedoMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.DiffuseCoeff = DiffuseCoeffMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.SpecularCoeff = SpecularCoeffMap.Load(float3(screenPos, 0.0f));
    
    return buf;
}

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
    output.pos = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, input.pos)));
	return output;
}

[earlydepthstencil]
float4 PSMain( PS_IN input ) : SV_Target
{
    GBufferData gBuffer = ReadGBuffer(input.pos.xy);
    float3 worldPos = gBuffer.WorldPos_Depth.xyz;
    float3 norm = normalize(gBuffer.Normal);
    float3 toLight = lightPos.xyz - worldPos;
    float distToLight = length(toLight);
    float distToLightSquared = max(dot(toLight, toLight), 0.00001f);
    float radius = lightPos.w;
    //float radius = 100.0f;
    
    toLight = normalize(toLight);
        
    // Texture color
    float3 objectColor = gBuffer.AlbedoCol;
       
    // Diffuse
    float3 diffuseColor = lightColor.xyz * gBuffer.DiffuseCoeff * saturate(dot(norm, toLight)) * DSAIntensity.x;
    
    // Specular
    float3 viewDir = normalize(cameraPos.xyz - worldPos);
    float3 halfWay = normalize(viewDir + toLight);
    float3 specularColor = lightColor.xyz * gBuffer.SpecularCoeff.xyz * DSAIntensity.y * pow(saturate(dot(halfWay, norm)), gBuffer.SpecularCoeff.w);
    
    float tmp = distToLightSquared / (radius * radius);
    float rangeAttenuation = pow(saturate(1.0f - tmp * tmp), 2.0f);
    float attenuation = saturate(1.0f - distToLight / radius);
    attenuation *= attenuation;
  
    objectColor = attenuation * objectColor * (diffuseColor + specularColor);
    return float4(objectColor, 1.0f);
}