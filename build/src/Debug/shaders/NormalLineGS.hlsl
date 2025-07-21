// NormalLineGS.hlsl
// Geometry shader para desenhar linhas de normais
struct VS_OUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct GS_OUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CBFrame : register(b0) {
    float4x4 viewProj;
};

[maxvertexcount(2)]
void GS(point VS_OUT input[1], inout LineStream<GS_OUT> outStream) {
    GS_OUT v0, v1;
    float3 p = input[0].pos;
    float3 n = input[0].normal;
    v0.pos = mul(float4(p, 1.0), viewProj);
    v0.color = float4(1,1,0,1); // amarelo
    v1.pos = mul(float4(p + n * 5.0, 1.0), viewProj);
    v1.color = float4(1,0,0,1); // vermelho
    outStream.Append(v0);
    outStream.Append(v1);
} 