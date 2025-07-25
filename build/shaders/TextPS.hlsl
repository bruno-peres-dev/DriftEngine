// Pixel Shader para renderização de texto com MSDF
// Anti-aliasing de qualidade AAA com suporte a subpixel rendering

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixelPos : TEXCOORD1;
    float2 screenPos : TEXCOORD2;
};

// Constantes do shader
cbuffer TextConstants : register(b0) {
    float4x4 viewProjection;
    float2 screenSize;
    float2 atlasSize;
    float msdfRange;
    float smoothing;
    float contrast;
    float gamma;
    float padding[2];
}

// Textura do atlas de fontes
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

// Função para calcular distância MSDF
float CalculateMSDFDistance(float4 msdf, float2 pixelPos) {
    // Extrai os canais MSDF (R, G, B)
    float3 distance = msdf.rgb;
    
    // Calcula a distância mínima entre os canais
    float minDistance = min(min(distance.r, distance.g), distance.b);
    
    // Aplica o range do MSDF
    return minDistance * msdfRange;
}

// Função para aplicar anti-aliasing
float ApplyAntiAliasing(float distance, float smoothing) {
    // Aplica suavização baseada na distância
    float alpha = smoothstep(-smoothing, smoothing, distance);
    
    // Aplica contraste
    alpha = pow(alpha, contrast);
    
    return alpha;
}

// Função para aplicar correção gamma
float ApplyGammaCorrection(float alpha, float gamma) {
    return pow(alpha, 1.0 / gamma);
}

// Função para subpixel rendering
float4 ApplySubpixelRendering(float4 color, float2 pixelPos, float alpha) {
    // Calcula offset subpixel baseado na posição
    float2 subpixelOffset = frac(pixelPos);
    
    // Aplica filtro de subpixel para melhorar a nitidez
    float subpixelAlpha = alpha;
    
    // Filtro de subpixel RGB (ClearType-like)
    float3 subpixelWeights = float3(1.0, 1.0, 1.0);
    
    // Ajusta pesos baseado na posição subpixel
    if (subpixelOffset.x < 0.33) {
        subpixelWeights.r *= 1.2;
        subpixelWeights.g *= 0.8;
        subpixelWeights.b *= 0.8;
    } else if (subpixelOffset.x < 0.66) {
        subpixelWeights.r *= 0.8;
        subpixelWeights.g *= 1.2;
        subpixelWeights.b *= 0.8;
    } else {
        subpixelWeights.r *= 0.8;
        subpixelWeights.g *= 0.8;
        subpixelWeights.b *= 1.2;
    }
    
    // Aplica os pesos subpixel
    color.rgb *= subpixelWeights;
    color.a = subpixelAlpha;
    
    return color;
}

// Função para aplicar efeitos de borda
float4 ApplyEdgeEffects(float4 color, float distance, float2 pixelPos) {
    // Efeito de borda suave
    float edgeSoftness = 0.5;
    float edgeAlpha = smoothstep(-edgeSoftness, edgeSoftness, distance);
    
    // Efeito de sombra (opcional)
    float shadowDistance = distance - 1.0;
    float shadowAlpha = smoothstep(-edgeSoftness, edgeSoftness, shadowDistance) * 0.3;
    
    // Combina os efeitos
    color.a = max(color.a, shadowAlpha);
    
    return color;
}

// Função principal do pixel shader
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura MSDF
    float4 msdf = fontAtlas.Sample(fontSampler, input.texCoord);
    
    // Calcula a distância MSDF
    float distance = CalculateMSDFDistance(msdf, input.pixelPos);
    
    // Aplica anti-aliasing
    float alpha = ApplyAntiAliasing(distance, smoothing);
    
    // Aplica correção gamma
    alpha = ApplyGammaCorrection(alpha, gamma);
    
    // Prepara a cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    // Aplica subpixel rendering se habilitado
    if (gamma > 0.0) {
        finalColor = ApplySubpixelRendering(finalColor, input.pixelPos, alpha);
    }
    
    // Aplica efeitos de borda
    finalColor = ApplyEdgeEffects(finalColor, distance, input.pixelPos);
    
    // Clampa valores finais
    finalColor = saturate(finalColor);
    
    return finalColor;
}

// Versão simplificada para performance
float4 mainFast(VertexOutput input) : SV_Target {
    // Amostra a textura MSDF
    float4 msdf = fontAtlas.Sample(fontSampler, input.texCoord);
    
    // Calcula distância simples
    float distance = min(min(msdf.r, msdf.g), msdf.b) * msdfRange;
    
    // Anti-aliasing básico
    float alpha = smoothstep(-smoothing, smoothing, distance);
    
    // Cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return saturate(finalColor);
} 