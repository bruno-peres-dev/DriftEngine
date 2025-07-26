// UIBatch.hlsl - shader para UI 2D com suporte a texturas
struct VSIn {
    float2 pos      : POSITION;
    float2 uv       : TEXCOORD0;
    float4 col      : COLOR0; // UNORM convertida para float automaticamente
    uint textureId  : TEXCOORD1;
};

struct PSIn {
    float4 pos      : SV_POSITION;
    float2 uv       : TEXCOORD0;
    float4 col      : COLOR0;
    uint textureId  : TEXCOORD1;
};

// Textura única para UI (será configurada pelo código)
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

PSIn VSMain(VSIn v) {
    PSIn o;
    // Usar Z = 0 para garantir que a UI fique na frente de tudo
    o.pos = float4(v.pos, 0.0, 1.0);
    o.uv = v.uv;
    o.col = v.col;
    o.textureId = v.textureId;
    return o;
}

float4 PSMain(PSIn i) : SV_TARGET { 
    // Se tem textura (textureId > 0), usar a textura com a cor
    if (i.textureId > 0) {
        float4 texColor = g_Texture.Sample(g_Sampler, i.uv);
        return texColor * i.col;
    }
    
    // Senão, retornar apenas a cor
    // Garantir que a cor seja aplicada corretamente
    return i.col;
} 