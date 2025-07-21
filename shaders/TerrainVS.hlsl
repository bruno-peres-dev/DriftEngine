cbuffer CBFrame : register(b0)
{
    matrix viewProj;
};

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput o;
    o.pos = mul(float4(input.pos, 1.0f), viewProj);
    o.normal = input.normal;
    o.uv = input.uv;
    return o;
}
