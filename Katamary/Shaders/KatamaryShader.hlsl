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

struct CascadeData
{
    float4x4 lightViewProjection[4];
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
Texture2DArray ShadowMap : register(t1);

SamplerState Sampler : register(s0);
SamplerComparisonState ShadowCompSampler : register(s1);

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
    float4 viewPos = input.viewPos;
    float depthVal = abs(viewPos.z);
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
    
    float4 lightSpacePos = mul(cascadeData.lightViewProjection[cascadeIdx], input.worldPos);
    lightSpacePos = lightSpacePos / lightSpacePos.w;
    float2 texCoords = (lightSpacePos.xy + float2(1.0f, 1.0f)) * 0.5f;
    texCoords.y = 1.0f - texCoords.y;
    
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(norm, lightDir.xyz)), 0.005);
    bias *= 1 / (cascadeData.distances[layer] * 0.5f);
    
    float shadowCoeff = PCF(float3(texCoords.x, texCoords.y, cascadeIdx), lightSpacePos.z - bias);
    
    float3 cascadeVector = float3(cascadeDepth, 0.0f, 0.0f);
    objectColor = objectColor * ((diffuseColor + specularColor) * shadowCoeff + ambientColor + cascadeVector);
    return float4(objectColor, 1.0f);
}