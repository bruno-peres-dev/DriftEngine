cbuffer CBFrame : register(b0)
{
    matrix viewProj;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct PSInput
{
    float4 pos : SV_POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), viewProj);
    return output;
}
