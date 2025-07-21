Texture2D gDiffuse : register(t0);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

float3 gLightDir = normalize(float3(0.5, -1, 0.5)); // Direção da luz

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef WIREFRAME
    return float4(0, 1, 0, 1); // verde
#else
    return gDiffuse.Sample(gSampler, input.uv);
#endif
}
