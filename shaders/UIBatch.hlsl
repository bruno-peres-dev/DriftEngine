// UIBatch.hlsl - shader para UI 2D com suporte a texturas
struct VSIn {
    float2 pos      : POSITION;
    float2 uv       : TEXCOORD0;
    float4 col      : COLOR0; // RGBA format (convertido de ARGB)
    uint textureId  : TEXCOORD1;
};

struct PSIn {
    float4 pos      : SV_POSITION;
    float2 uv       : TEXCOORD0;
    float4 col      : COLOR0; // RGBA format
    uint textureId  : TEXCOORD1;
};

// Array de texturas para UI (suporte a múltiplas texturas)
Texture2D g_Textures[8] : register(t0);
SamplerState g_Samplers[8] : register(s0);

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
    // Cor já está em formato RGBA, usar diretamente
    float4 rgbaColor = i.col;
    
    // Se tem textura (textureId < 8), usar a textura com a cor (0-7 = slots de textura, 8+ = sem textura)
    if (i.textureId < 8) {
        float4 texColor;
        
        // Usar switch para evitar erro de sampler array index
        switch (i.textureId) {
            case 0: texColor = g_Textures[0].Sample(g_Samplers[0], i.uv); break;
            case 1: texColor = g_Textures[1].Sample(g_Samplers[1], i.uv); break;
            case 2: texColor = g_Textures[2].Sample(g_Samplers[2], i.uv); break;
            case 3: texColor = g_Textures[3].Sample(g_Samplers[3], i.uv); break;
            case 4: texColor = g_Textures[4].Sample(g_Samplers[4], i.uv); break;
            case 5: texColor = g_Textures[5].Sample(g_Samplers[5], i.uv); break;
            case 6: texColor = g_Textures[6].Sample(g_Samplers[6], i.uv); break;
            case 7: texColor = g_Textures[7].Sample(g_Samplers[7], i.uv); break;
            default: texColor = float4(1, 1, 1, 1); break;
        }
        
        return texColor * rgbaColor;
    }
    
    // Senão, retornar apenas a cor (já convertida para RGBA)
    return rgbaColor;
} 