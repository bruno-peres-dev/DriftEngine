cbuffer CBFrame : register(b0)
{
    matrix viewProj;
};

struct VS_IN {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_OUT VSMain(VS_IN input) {
    VS_OUT output;
    output.pos = input.pos;
    output.normal = input.normal; // NÃO transformar!
    output.uv = input.uv;
    return output;
}
