cbuffer CBFrame : register(b0)
{
    matrix viewProj;
};

struct VS_IN {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewPos : TEXCOORD1;
};

PSInput VSMain(VS_IN input) {
    PSInput output;
    
    // Transform to clip space
    output.pos = mul(float4(input.pos, 1.0), viewProj);
    
    // Pass world position for distance-based effects
    output.worldPos = input.pos;
    
    // Pass normal (could be enhanced with normal mapping later)
    output.normal = input.normal;
    
    // Pass UV coordinates
    output.uv = input.uv;
    
    // Calculate view space position for fog/distance effects
    output.viewPos = mul(float4(input.pos, 1.0), viewProj).xyz;
    
    return output;
}
