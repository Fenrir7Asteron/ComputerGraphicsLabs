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
    float4 worldPos : POSITIONT;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
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

Texture2D DiffuseMap : register(t0);
SamplerState Sampler : register(s0);

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    output.pos = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, input.pos)));
    output.col = input.col;
    //output.norm = (input.norm + 1.0) * 0.5;
    output.norm = mul(worldMatrix, input.norm);
    output.tex = input.tex.xyz;
    output.worldPos = mul(worldMatrix, input.pos);
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    float4 norm = normalize(input.norm);
	//return input.norm;
    //return input.col * (1.0f - input.tex.z) + DiffuseMap.Sample(Sampler, input.tex.xy) * input.tex.z;
    
    // Texture color
    float3 objectColor = DiffuseMap.Sample(Sampler, input.tex.xy);
       
    // Diffuse
    float3 diffuseColor = lightColor * kD * dot(norm, -lightDir) * DSAIntensity.x;
    
    // Specular
    float3 reflected = normalize(reflect(lightDir, norm));
    float3 viewDir = normalize(cameraPos - input.worldPos);
    float3 specularColor = lightColor.xyz * kS_alpha.xyz * DSAIntensity.y * pow(max(dot(viewDir, reflected), 0.0f), kS_alpha.w);
    
    // Ambient
    float3 ambientColor = lightColor * kA * DSAIntensity.z;
    
    objectColor = objectColor * (diffuseColor + specularColor + ambientColor);
    return float4(objectColor, 1.0f);
}