// ============================================================================
// PIXEL SHADER MSDF - SISTEMA DE FONTES AVANÇADO
// ============================================================================
// Este shader implementa Multi-channel Signed Distance Field (MSDF)
// para renderização de fontes de alta qualidade em qualquer resolução

#include "Common.hlsl"

// ============================================================================
// ESTRUTURAS E CONSTANTES
// ============================================================================

// Constantes MSDF
static const float MSDF_PIXEL_RANGE = 2.0f;
static const float MSDF_EDGE_THRESHOLD = 0.5f;
static const float MSDF_SMOOTHING = 0.0625f;

// Configurações de renderização
cbuffer MSDFConstants : register(b1)
{
    float4 textColor;           // Cor do texto (RGBA)
    float4 outlineColor;        // Cor do outline (RGBA)
    float4 shadowColor;         // Cor da sombra (RGBA)
    float outlineWidth;         // Largura do outline
    float2 shadowOffset;        // Deslocamento da sombra
    float shadowBlur;           // Blur da sombra
    float msdfPixelRange;       // Range de pixels para MSDF
    float msdfEdgeThreshold;    // Threshold para detecção de bordas
    float enableSmoothing;      // Habilitar suavização
    float enableSubpixel;       // Renderização subpixel
    float2 padding;             // Padding para alinhamento
};

// Texturas
Texture2D<float4> msdfAtlas : register(t0);
SamplerState msdfSampler : register(s0);

// ============================================================================
// FUNÇÕES UTILITÁRIAS MSDF
// ============================================================================

// Calcula a distância MSDF a partir dos canais RGB
float msdfMedian(float r, float g, float b, float a)
{
    return median(r, g, b);
}

// Amostra o MSDF e retorna a distância
float sampleMSDF(float2 uv, float pixelRange)
{
    float4 msdf = msdfAtlas.Sample(msdfSampler, uv);
    
    // Extrair canais MSDF (R=distância horizontal, G=distância vertical, B=distância diagonal)
    float distance = msdfMedian(msdf.r, msdf.g, msdf.b);
    
    // Converter para distância em pixels
    distance = distance * pixelRange;
    
    return distance;
}

// Calcula a distância MSDF com suavização
float msdfDistance(float2 uv, float pixelRange, float smoothing)
{
    float distance = sampleMSDF(uv, pixelRange);
    
    // Aplicar suavização se habilitada
    if (smoothing > 0.0f)
    {
        // Amostras adicionais para suavização
        float2 texelSize = 1.0f / float2(msdfAtlas.GetDimensions());
        
        float d1 = sampleMSDF(uv + float2(texelSize.x, 0.0f), pixelRange);
        float d2 = sampleMSDF(uv + float2(-texelSize.x, 0.0f), pixelRange);
        float d3 = sampleMSDF(uv + float2(0.0f, texelSize.y), pixelRange);
        float d4 = sampleMSDF(uv + float2(0.0f, -texelSize.y), pixelRange);
        
        // Média ponderada
        distance = lerp(distance, (distance + d1 + d2 + d3 + d4) * 0.2f, smoothing);
    }
    
    return distance;
}

// Calcula a opacidade baseada na distância MSDF
float msdfOpacity(float distance, float edgeThreshold)
{
    // Função de suavização baseada na distância
    float opacity = 1.0f - smoothstep(-edgeThreshold, edgeThreshold, distance);
    
    // Clamp para evitar artefatos
    return saturate(opacity);
}

// Renderização subpixel para melhor qualidade
float3 subpixelRendering(float2 uv, float pixelRange, float edgeThreshold)
{
    if (enableSubpixel <= 0.0f)
    {
        float opacity = msdfOpacity(msdfDistance(uv, pixelRange, MSDF_SMOOTHING), edgeThreshold);
        return float3(opacity, opacity, opacity);
    }
    
    // Renderização subpixel RGB
    float2 texelSize = 1.0f / float2(msdfAtlas.GetDimensions());
    float2 subpixelOffsets[3] = {
        float2(-texelSize.x * 0.25f, 0.0f),  // Vermelho
        float2(0.0f, 0.0f),                   // Verde
        float2(texelSize.x * 0.25f, 0.0f)    // Azul
    };
    
    float3 subpixel;
    for (int i = 0; i < 3; ++i)
    {
        float2 offsetUV = uv + subpixelOffsets[i];
        float distance = msdfDistance(offsetUV, pixelRange, MSDF_SMOOTHING);
        subpixel[i] = msdfOpacity(distance, edgeThreshold);
    }
    
    return subpixel;
}

// ============================================================================
// FUNÇÕES DE EFEITOS VISUAIS
// ============================================================================

// Renderiza outline
float4 renderOutline(float2 uv, float4 baseColor, float outlineWidth, float4 outlineColor)
{
    if (outlineWidth <= 0.0f)
        return baseColor;
    
    float2 texelSize = 1.0f / float2(msdfAtlas.GetDimensions());
    float maxDistance = outlineWidth * msdfPixelRange;
    
    // Amostras para outline
    float2 offsets[8] = {
        float2(-texelSize.x, -texelSize.y),
        float2(-texelSize.x, 0.0f),
        float2(-texelSize.x, texelSize.y),
        float2(0.0f, -texelSize.y),
        float2(0.0f, texelSize.y),
        float2(texelSize.x, -texelSize.y),
        float2(texelSize.x, 0.0f),
        float2(texelSize.x, texelSize.y)
    };
    
    float outlineDistance = 1000.0f;
    for (int i = 0; i < 8; ++i)
    {
        float2 offsetUV = uv + offsets[i] * outlineWidth;
        float distance = msdfDistance(offsetUV, msdfPixelRange, MSDF_SMOOTHING);
        outlineDistance = min(outlineDistance, distance);
    }
    
    float outlineOpacity = msdfOpacity(outlineDistance, msdfEdgeThreshold);
    
    // Blend outline com base
    return lerp(baseColor, outlineColor, outlineOpacity * outlineColor.a);
}

// Renderiza sombra
float4 renderShadow(float2 uv, float4 baseColor, float2 shadowOffset, float shadowBlur, float4 shadowColor)
{
    if (shadowBlur <= 0.0f)
        return baseColor;
    
    float2 texelSize = 1.0f / float2(msdfAtlas.GetDimensions());
    float2 shadowUV = uv + shadowOffset * texelSize;
    
    // Amostras para blur da sombra
    float shadowDistance = 1000.0f;
    int samples = int(shadowBlur * 4.0f);
    
    for (int x = -samples; x <= samples; ++x)
    {
        for (int y = -samples; y <= samples; ++y)
        {
            float2 offset = float2(x, y) * texelSize * shadowBlur;
            float2 sampleUV = shadowUV + offset;
            float distance = msdfDistance(sampleUV, msdfPixelRange, MSDF_SMOOTHING);
            shadowDistance = min(shadowDistance, distance);
        }
    }
    
    float shadowOpacity = msdfOpacity(shadowDistance, msdfEdgeThreshold);
    float4 shadow = shadowColor * shadowOpacity;
    
    // Blend sombra com base
    return lerp(baseColor, shadow, shadow.a);
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO PIXEL SHADER
// ============================================================================

float4 MSDFFontPS(PSInput input) : SV_Target
{
    // Obter coordenadas UV
    float2 uv = input.uv;
    
    // Renderização subpixel
    float3 subpixel = subpixelRendering(uv, msdfPixelRange, msdfEdgeThreshold);
    float opacity = (subpixel.r + subpixel.g + subpixel.b) / 3.0f;
    
    // Cor base do texto
    float4 baseColor = textColor * opacity;
    
    // Aplicar sombra se configurada
    if (shadowBlur > 0.0f)
    {
        baseColor = renderShadow(uv, baseColor, shadowOffset, shadowBlur, shadowColor);
    }
    
    // Aplicar outline se configurado
    if (outlineWidth > 0.0f)
    {
        baseColor = renderOutline(uv, baseColor, outlineWidth, outlineColor);
    }
    
    // Aplicar cor final
    baseColor.rgb *= subpixel;
    
    return baseColor;
}

// ============================================================================
// VERSÕES OTIMIZADAS PARA DIFERENTES CASOS DE USO
// ============================================================================

// Versão simples sem efeitos
float4 MSDFFontSimplePS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float opacity = msdfOpacity(msdfDistance(uv, msdfPixelRange, 0.0f), msdfEdgeThreshold);
    return textColor * opacity;
}

// Versão com outline apenas
float4 MSDFFontOutlinePS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float opacity = msdfOpacity(msdfDistance(uv, msdfPixelRange, MSDF_SMOOTHING), msdfEdgeThreshold);
    float4 baseColor = textColor * opacity;
    
    if (outlineWidth > 0.0f)
    {
        baseColor = renderOutline(uv, baseColor, outlineWidth, outlineColor);
    }
    
    return baseColor;
}

// Versão com sombra apenas
float4 MSDFFontShadowPS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float opacity = msdfOpacity(msdfDistance(uv, msdfPixelRange, MSDF_SMOOTHING), msdfEdgeThreshold);
    float4 baseColor = textColor * opacity;
    
    if (shadowBlur > 0.0f)
    {
        baseColor = renderShadow(uv, baseColor, shadowOffset, shadowBlur, shadowColor);
    }
    
    return baseColor;
}

// ============================================================================
// FUNÇÕES DE DEBUG E DESENVOLVIMENTO
// ============================================================================

// Debug: Mostra apenas o MSDF
float4 MSDFDebugPS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float4 msdf = msdfAtlas.Sample(msdfSampler, uv);
    
    // Mostrar canais MSDF
    return float4(msdf.rgb, 1.0f);
}

// Debug: Mostra distância MSDF
float4 MSDFDistanceDebugPS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float distance = msdfDistance(uv, msdfPixelRange, MSDF_SMOOTHING);
    
    // Normalizar distância para visualização
    float normalized = saturate((distance + msdfPixelRange) / (msdfPixelRange * 2.0f));
    return float4(normalized, normalized, normalized, 1.0f);
}

// Debug: Mostra opacidade MSDF
float4 MSDFOpacityDebugPS(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float opacity = msdfOpacity(msdfDistance(uv, msdfPixelRange, MSDF_SMOOTHING), msdfEdgeThreshold);
    return float4(opacity, opacity, opacity, 1.0f);
} 