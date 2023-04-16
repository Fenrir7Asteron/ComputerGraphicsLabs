struct GS_IN
{
    int vertexID : TEXCOORD0;
};

struct GS_OUT
{
    float4 Position : SV_Position;
    float4 Color : COLOR0;
    float2 Tex : TEXCOORD0;
    float Radius : TEXCOORD1;
};

struct ConstParams
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4 DeltatimeMaxparticlesGroupdimY;
};

struct Particle
{
    float4 Position;
    float4 Velocity;
    float4 Color0;
    float4 Color1;
    float4 Color;
    float2 Size0Size1;
    float Size;
    float LifeTime;
};

cbuffer CONSTANT_BUFFER : register(b0)
{
    ConstParams Params;
}

StructuredBuffer<Particle>        renderBufSrc    : register(t0);
ConsumeStructuredBuffer<Particle> particlesBufSrc : register(u0);
AppendStructuredBuffer<Particle>  particlesBufDst : register(u1);


GS_IN VSMain(uint id : SV_VertexID)
{
    GS_IN output = (GS_IN) 0;
    output.vertexID = id;
	return output;
}

[maxvertexcount(4)]
void GSMain(point GS_IN inputPoint[1],
               inout TriangleStream<GS_OUT> outputStream
           )
{
    GS_OUT p0, p1, p2, p3;
    Particle prt = renderBufSrc[inputPoint[0].vertexID];
    
    float size = prt.Size;
    float4 color = prt.Color;
    
    //float4 pos = mul(Params.View, mul(Params.World, prt.Position));
    float4 pos = mul(Params.View, prt.Position);
    
    pos = float4(pos.xyz, 1.0f);
    
    p0.Position = mul(Params.Projection, pos + float4(size, size, 0, 0));
    p0.Tex = float2(1, 1);
    p0.Color = color;
    p0.Radius = size;
    
    p1.Position = mul(Params.Projection, pos + float4(-size, size, 0, 0));
    p1.Tex = float2(0, 1);
    p1.Color = color;
    p1.Radius = size;

    p2.Position = mul(Params.Projection, pos + float4(-size, -size, 0, 0));
    p2.Tex = float2(0, 0);
    p2.Color = color;
    p2.Radius = size;
    
    p3.Position = mul(Params.Projection, pos + float4(size, -size, 0, 0));
    p3.Tex = float2(1, 0);
    p3.Color = color;
    p3.Radius = size;
    
    outputStream.Append(p1);
    outputStream.Append(p0);
    outputStream.Append(p2);
    outputStream.Append(p3);
}

float4 PSMain(GS_OUT input) : SV_Target0
{
    float amount = length(input.Tex - float2(0.5f, 0.5f)) * 2.0f;
    amount = smoothstep(0.0f, 0.6f, 1.0f - amount);
    //return float4(input.Color.rgb, 1.0f);
    return float4(input.Color.rgb, amount);
}

#define BLOCK_SIZE 256
#define THREAD_IN_GROUP_TOTAL 256

[numthreads(BLOCK_SIZE, 1, 1)]
void CSMain(
    uint3 groupID             : SV_GroupID,
    uint3 groupThreadID       : SV_GroupThreadID,
    uint3 dispatchThreadID    : SV_DispatchThreadID,
    uint groupIndex           : SV_GroupIndex
)
{
    uint id = groupID.x * THREAD_IN_GROUP_TOTAL + groupID.y * Params.DeltatimeMaxparticlesGroupdimY.z * THREAD_IN_GROUP_TOTAL + groupIndex;
    
    [flatten]
    if (id >= (uint) Params.DeltatimeMaxparticlesGroupdimY.y)
        return;
    
#ifdef INJECTION
    Particle p = particlesBufSrc.Consume();

    if (p.LifeTime > 0)
        particlesBufDst.Append(p);
#endif
    
#ifdef SIMULATION
    Particle p = particlesBufSrc.Consume();
    
    float deltaTime = Params.DeltatimeMaxparticlesGroupdimY.x;

    p.LifeTime -= deltaTime;
    p.Color = lerp(p.Color1, p.Color0, p.LifeTime);
    p.Size = lerp(p.Size0Size1.y, p.Size0Size1.x, p.LifeTime);
    
    if (p.LifeTime > 0)
    {
    #ifdef ADD_GRAVITY
        p.Velocity += float4(0, -980.0f * deltaTime, 0, 0);
    #endif
        p.Position.xyz += (p.Velocity * deltaTime).xyz;
        particlesBufDst.Append(p);
    }
#endif
}