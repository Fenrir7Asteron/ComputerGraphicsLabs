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
    float4 CameraPosX;
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

struct GBufferData
{
    float4 WorldPos_Depth;
    float3 Normal;
};

cbuffer CONSTANT_BUFFER : register(b0)
{
    ConstParams Params;
}

StructuredBuffer<Particle> renderBufSrc : register(t0);
RWStructuredBuffer<Particle> particlesPool: register(u0);

//#define INJECTION
//#define SIMULATION

#ifdef INJECTION
AppendStructuredBuffer<float2>  sortedParticles : register(u1);
ConsumeStructuredBuffer<uint>  deadParticles : register(u2);
ConsumeStructuredBuffer<Particle>  injectionBuf : register(u3);
#endif

#ifdef SIMULATION
RWStructuredBuffer<float2>  sortedParticles : register(u1);
AppendStructuredBuffer<uint>  deadParticles : register(u2);
#endif


Texture2D WorldPosDepthMap : register(t1);
Texture2D NormalMap : register(t2);

GBufferData ReadGBuffer(float2 screenPos)
{
    GBufferData buf = (GBufferData) 0;
    
    buf.WorldPos_Depth = WorldPosDepthMap.Load(float3(screenPos, 0.0f));
    buf.Normal = NormalMap.Load(float3(screenPos, 0.0f)).xyz;
    
    return buf;
}


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
    uint poolId = deadParticles.Consume();
    Particle p = injectionBuf.Consume();
    
    float distance = Params.CameraPosX.xyz - mul(Params.World, float4(p.Position.xyz, 1.0f)).xyz;
    distance *= distance;
    
    sortedParticles.Append(float2(poolId, -distance));
    
    particlesPool[poolId] = p;
#endif
    
#ifdef SIMULATION
    uint pId = (uint)sortedParticles[id].x;
    
    Particle p = particlesPool[pId];
    
    float deltaTime = Params.DeltatimeMaxparticlesGroupdimY.x;
    
    p.LifeTime -= deltaTime;
    
    [branch]
    if (p.LifeTime <= 0)
    {
        deadParticles.Append(pId);
        sortedParticles[id] = float2(pId, 1000);
        
        p.Color0 = float4(1.0f, 0.0f, 0.0f, 1.0f);
        p.Color1 = float4(1.0f, 0.0f, 0.0f, 1.0f);
        p.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        p.Size0Size1 = float2(20, 20);
        particlesPool[pId] = p;
        return;
    }
    
#ifdef ADD_GRAVITY
        p.Velocity += float4(0, -980.0f * deltaTime, 0, 0);
#endif
  
    p.Color = lerp(p.Color1, p.Color0, p.LifeTime);
    p.Size = lerp(p.Size0Size1.y, p.Size0Size1.x, p.LifeTime);
    
    if (p.LifeTime > 0)
    {
        float4 newPosition = p.Position + (p.Velocity * deltaTime);
        
        float4 viewPos = mul(Params.View, newPosition);
        float4 cameraPos = mul(Params.Projection, viewPos);
        cameraPos /= cameraPos.w;
        
        cameraPos.xy = (cameraPos.xy + 1.0f) * 0.5f;
        cameraPos.y = 1.0f - cameraPos.y;
        float gBufferWidth, gBufferHeight;
        NormalMap.GetDimensions(gBufferWidth, gBufferHeight);
        
        cameraPos.xy *= float2(gBufferWidth, gBufferHeight);
        
        GBufferData gBuffer = ReadGBuffer(cameraPos.xy);
        
        float depthVal = gBuffer.WorldPos_Depth.w;
        
        if (viewPos.z + 0.005f > depthVal 
            && cameraPos.x >= 0.0f && cameraPos.x <= gBufferWidth
            && cameraPos.y >= 0.0f && cameraPos.y <= gBufferHeight
            )
        {
            float3 norm = normalize(gBuffer.Normal);
            
            p.Velocity.xyz = reflect(p.Velocity.xyz, norm).xyz;
            newPosition = p.Position;
        }
        
        p.Position.xyz = newPosition.xyz;
        particlesPool[pId] = p;
    }
#endif
}