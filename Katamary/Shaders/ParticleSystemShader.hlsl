struct GS_IN
{
    int vertexID : TEXCOORD0;
};

struct GS_OUT
{
    float4 Position : SV_Position;
    float4 Color : COLOR0;
    float2 Tex : TEXCOORD0;
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
    float2 Size0Size1;
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
    
    float size = prt.Size0Size1.x;
    float4 color = prt.Color0;
    
    float4 pos = mul(Params.View, mul(Params.World, prt.Position));
    
    pos = float4(pos.xyz, 1.0f);
    
    p0.Position = mul(Params.Projection, pos + float4(size, size, 0, 0));
    p0.Tex = float2(1, 1);
    p0.Color = color;
    
    p1.Position = mul(Params.Projection, pos + float4(-size, size, 0, 0));
    p1.Tex = float2(0, 1);
    p1.Color = color;

    p2.Position = mul(Params.Projection, pos + float4(-size, -size, 0, 0));
    p2.Tex = float2(0, 0);
    p2.Color = color;
    
    p3.Position = mul(Params.Projection, pos + float4(size, -size, 0, 0));
    p3.Tex = float2(1, 0);
    p3.Color = color;
    
    outputStream.Append(p1);
    outputStream.Append(p0);
    outputStream.Append(p2);
    outputStream.Append(p3);
}

float4 PSMain(GS_OUT input) : SV_Target0
{
    float amount = length(input.Tex - float2(0.5f, 0.5f)) * 2.0f;
    amount = smoothstep(0.0f, 1.0f, 1.0f - amount);
    //return float4(input.Color.rgb, 1.0f);
    return float4(input.Color.rgb, amount);
}

#define THREAD_GROUP_X 16
#define THREAD_GROUP_Y 16
#define THREAD_IN_GROUP_TOTAL 256

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, 1)]
void CSMain(
    uint3 groupID             : SV_GroupID,
    uint3 groupThreadID       : SV_GroupID,
    uint3 dispatchThreadID    : SV_GroupID,
    uint groupIndex          : SV_GroupID
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
    
    if (p.LifeTime > 0)
    {
    #ifdef ADD_GRAVITY
        p.Velocity += float4(0, -9.8f * deltaTime, 0, 0);
    #endif
        p.Position.xyz += (p.Velocity * deltaTime).xyz;
        particlesBufDst.Append(p);
    }
#endif
}