struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
    float4 norm : NORMAL0;
    float4 tex : TEXCOORD0;
};

struct GS_IN
{
	float4 pos : POSITION0;
};

struct GS_OUT
{
	float4 pos : SV_POSITION;
	uint arrInd : SV_RenderTargetArrayIndex;
};

struct CascadeData
{
    float4x4 lightViewProjection[4];
    float4 distances;
};

cbuffer GS_CONSTANT_BUFFER : register(b0)
{
    CascadeData cascadeData;
}

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 transposeInverseWorldMatrix;
};

GS_IN VSMain(VS_IN input)
{
    GS_IN output = (GS_IN) 0;
    float4 pos = float4(input.pos.xyz, 1.0f);
    output.pos = mul(worldMatrix, pos);
    return output;
}

[instance(4)]
[maxvertexcount(3)]
void GSMain(   triangle GS_IN p[3],
               in uint id : SV_GSInstanceID,
               inout TriangleStream<GS_OUT> stream
           )
{
	[unroll]
    for (int i = 0; i < 3; ++i)
    {
        GS_OUT gs = (GS_OUT) 0;
        float4 worldPos = float4(p[i].pos.xyz, 1.0f);
        gs.pos = mul(cascadeData.lightViewProjection[id], worldPos);
        gs.arrInd = id;
        stream.Append(gs);
    }
}