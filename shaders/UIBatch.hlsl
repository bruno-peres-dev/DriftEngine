// UIBatch.hlsl - shader otimizado para UI 2D de nível AAA
// Suporte a 16 texturas e otimizações de performance

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

// Array de texturas para UI (suporte a 16 texturas)
Texture2D g_Textures[16] : register(t0);
SamplerState g_Samplers[16] : register(s0);

// Constantes para otimizações
cbuffer UIConstants : register(b0) {
    float2 screenSize;
    float2 atlasSize;
    float2 padding;
    float time;
    float4 debugColor;
};

// Função otimizada para amostragem de textura
float4 SampleTexture(uint textureId, float2 uv) {
    // Usar switch otimizado para evitar sampler array index errors
    // Compilador pode otimizar melhor este switch
    switch (textureId) {
        case 0:  return g_Textures[0].Sample(g_Samplers[0], uv);
        case 1:  return g_Textures[1].Sample(g_Samplers[1], uv);
        case 2:  return g_Textures[2].Sample(g_Samplers[2], uv);
        case 3:  return g_Textures[3].Sample(g_Samplers[3], uv);
        case 4:  return g_Textures[4].Sample(g_Samplers[4], uv);
        case 5:  return g_Textures[5].Sample(g_Samplers[5], uv);
        case 6:  return g_Textures[6].Sample(g_Samplers[6], uv);
        case 7:  return g_Textures[7].Sample(g_Samplers[7], uv);
        case 8:  return g_Textures[8].Sample(g_Samplers[8], uv);
        case 9:  return g_Textures[9].Sample(g_Samplers[9], uv);
        case 10: return g_Textures[10].Sample(g_Samplers[10], uv);
        case 11: return g_Textures[11].Sample(g_Samplers[11], uv);
        case 12: return g_Textures[12].Sample(g_Samplers[12], uv);
        case 13: return g_Textures[13].Sample(g_Samplers[13], uv);
        case 14: return g_Textures[14].Sample(g_Samplers[14], uv);
        case 15: return g_Textures[15].Sample(g_Samplers[15], uv);
        default: return float4(1, 1, 1, 1);
    }
}

// Vertex shader otimizado
PSIn VSMain(VSIn v) {
    PSIn o;
    
    // CRÍTICO: Coordenadas já estão em clip space, usar diretamente
    // Não fazer conversão dupla de coordenadas
    o.pos = float4(v.pos, 0.0, 1.0);
    o.uv = v.uv;
    o.col = v.col;
    o.textureId = v.textureId;
    
    return o;
}

// Pixel shader otimizado com early-out e melhor qualidade
float4 PSMain(PSIn i) : SV_TARGET { 
    // Early-out para transparência total
    if (i.col.a <= 0.0) {
        discard;
    }
    
    // Cor já está em formato RGBA, usar diretamente
    float4 rgbaColor = i.col;
    
    // Se tem textura (0-15) e não é o identificador especial 8 (sem textura)
    if (i.textureId < 16 && i.textureId != 8) {
        float4 texColor = SampleTexture(i.textureId, i.uv);
        
        // Otimização: early-out para texturas completamente transparentes
        if (texColor.a <= 0.0) {
            discard;
        }
        
        // Para texto, usar apenas o alpha da textura para blending
        // Manter a cor do texto, mas aplicar o alpha da textura
        float4 finalColor = rgbaColor;
        
        // Usar o canal alpha da textura para determinar a opacidade do texto
        // Se a textura tem apenas um canal (R8_UNORM), usar o canal R como alpha
        float textureAlpha = texColor.a;
        if (textureAlpha == 0.0 && texColor.r > 0.0) {
            // Se alpha é 0 mas R tem valor, provavelmente é uma textura R8_UNORM
            textureAlpha = texColor.r;
        }
        
        // Aplicar o alpha da textura à cor final
        // Para texto, multiplicar o alpha da textura pelo alpha da cor
        finalColor.a *= textureAlpha;
        
        // Early-out para resultado final transparente
        if (finalColor.a <= 0.0) {
            discard;
        }
        
        return finalColor;
    }
    
    // Senão, retornar apenas a cor (já convertida para RGBA)
    return rgbaColor;
}

// Pixel shader alternativo para debug/desenvolvimento
float4 PSMainDebug(PSIn i) : SV_TARGET {
    // Modo debug: mostrar informações de textura
    if (i.textureId < 16) {
        return debugColor;
    }
    
    return i.col;
} 