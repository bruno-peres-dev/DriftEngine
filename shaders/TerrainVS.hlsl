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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

PSInput VSMain(VS_IN input) {
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0), viewProj);
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}
