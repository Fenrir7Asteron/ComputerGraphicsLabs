struct VS_IN
{
	float4 pos : POSITION0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
};

struct CascadeData
{
    float4x4 lightViewProjection[4];
    float4 distances;
};

// Phong directional light parameters
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 cameraPos;
    float4 lightParam;
    float4 lightColor;
    float4 DSAIntensity;
};

cbuffer CascadeData : register(b1)
{
    CascadeData cascadeData;
};

cbuffer VS_CONSTANT_BUFFER : register(b2)
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
Texture2D AmbientCoeffMap  : register(t5);

Texture2DArray ShadowMap   : register(t6);

SamplerComparisonState ShadowCompSampler : register(s0);

struct GBufferData
{
    float4 WorldPos_Depth;
    float3 Normal;
    float3 AlbedoCol;
    float3 DiffuseCoeff;
    float4 SpecularCoeff;
    float3 AmbientCoeff;
};

GBufferData ReadGBuffer(float2 screenPos)
{
    GBufferData buf = (GBufferData) 0;
    
    buf.WorldPos_Depth = WorldPosDepthMap.Load(float3(screenPos, 0.0f));
    buf.Normal = NormalMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.AlbedoCol = AlbedoMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.DiffuseCoeff = DiffuseCoeffMap.Load(float3(screenPos, 0.0f)).xyz;
    buf.SpecularCoeff = SpecularCoeffMap.Load(float3(screenPos, 0.0f));
    buf.AmbientCoeff = AmbientCoeffMap.Load(float3(screenPos, 0.0f)).xyz;
    
    return buf;
}

PS_IN VSMain( 
#ifdef SCREEN_QUAD
    uint id : SV_VertexID
#else
    VS_IN input
#endif
)
{
	PS_IN output = (PS_IN)0;
#ifdef SCREEN_QUAD
    float2 inds = float2(id & 1, (id & 2) >> 1);
    output.pos = float4(inds * float2(2, -2) + float2(-1, 1), 0, 1);	
#else
    output.pos = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, input.pos)));
#endif
	return output;
}

float PCF(float3 loc, float cmpDepth)
{
    float shadow = 0.0f;
    float shWidth, shHeight, cascadesCount;
    ShadowMap.GetDimensions(shWidth, shHeight, cascadesCount);
    
    float2 texelSize = 1.0f / float2(shWidth, shHeight);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = ShadowMap.SampleCmpLevelZero(
                    ShadowCompSampler,
                    float3(loc.xy + float2(float(x), float(y)) * texelSize, loc.z), \
                    cmpDepth).r;
            
            shadow += (cmpDepth) > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (cmpDepth > 1.0f)
    {
        shadow = 0.0f;
    }
    
    float depth = ShadowMap.SampleCmpLevelZero(ShadowCompSampler, loc, cmpDepth).x;
    return depth;
}

float4 PSMain( PS_IN input ) : SV_Target
{
#ifdef DIRECTIONAL
    GBufferData gBuffer = ReadGBuffer(input.pos.xy);
    float3 worldPos = gBuffer.WorldPos_Depth.xyz;
    float3 norm = normalize(gBuffer.Normal);
    float3 lDir = normalize(lightParam.xyz);
    
    // Texture color
    float3 objectColor = gBuffer.AlbedoCol;
       
    // Diffuse
    float3 diffuseColor = lightColor.xyz * gBuffer.DiffuseCoeff * saturate(dot(norm, -lDir)) * DSAIntensity.x;
    
    // Specular
    float3 reflected = normalize(reflect(lDir, norm.xyz));
    float3 viewDir = normalize(cameraPos.xyz - worldPos);
    float3 specularColor = lightColor.xyz * gBuffer.SpecularCoeff.xyz * DSAIntensity.y * pow(saturate(dot(reflected, viewDir)), gBuffer.SpecularCoeff.w);
    
    // Ambient
    float3 ambientColor = lightColor.xyz * gBuffer.AmbientCoeff * DSAIntensity.z;
    
    
    // Cascade shadows
    float depthVal = abs(gBuffer.WorldPos_Depth.w);
    float cascadeIdx = 3.0f;
    int layer = 3;
    float cascadeDepth = 1.0f;

    for (int i = 0; i < 4; ++i)
    {
        if (depthVal < cascadeData.distances[i])
        {
            cascadeIdx = float(i);
            layer = i;
            cascadeDepth = 0.25f * i;
            break;
        }
    }      
    
    float4 lightSpacePos = mul(cascadeData.lightViewProjection[cascadeIdx], float4(worldPos, 1.0f));
    lightSpacePos = lightSpacePos / lightSpacePos.w;
    float2 texCoords = (lightSpacePos.xy + float2(1.0f, 1.0f)) * 0.5f;
    texCoords.y = 1.0f - texCoords.y;
    
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(norm, lightParam.xyz)), 0.005);
    bias *= 1 / (cascadeData.distances[layer] * 0.5f);
    
    float shadowCoeff = PCF(float3(texCoords.x, texCoords.y, cascadeIdx), lightSpacePos.z - bias);
    
    float3 cascadeVector = float3(cascadeDepth, 0.0f, 0.0f);
    objectColor = objectColor * ((diffuseColor + specularColor) * shadowCoeff + ambientColor);
    return float4(objectColor, 1.0f);
#endif
    
#ifdef POINT
    GBufferData gBuffer = ReadGBuffer(input.pos.xy);
    float3 worldPos = gBuffer.WorldPos_Depth.xyz;
    float3 norm = normalize(gBuffer.Normal);
    float3 toLight = lightParam.xyz - worldPos;
    float distToLight = length(toLight);
    float distToLightSquared = max(dot(toLight, toLight), 0.00001f);
    float radius = lightParam.w;
    
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
#endif
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}