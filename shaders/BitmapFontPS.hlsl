// Pixel Shader para renderização de fontes bitmap (8-bit)
// Compatível com stbtt_BakeFontBitmap

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
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

// Textura do atlas de fontes (8-bit, R8_UNORM)
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

// Função principal do pixel shader
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura de fonte (valor 0-1)
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    float alpha = texColor.r; // Para R8_UNORM, o valor está no canal R
    
    // Aplica contraste se configurado
    if (contrast != 1.0f) {
        alpha = pow(alpha, contrast);
    }
    
    // Aplica correção gamma se configurado
    if (gamma != 1.0f) {
        alpha = pow(alpha, 1.0f / gamma);
    }
    
    // Aplica suavização se configurado
    if (smoothing > 0.0f) {
        // Amostras adicionais para suavização
        float2 texelSize = 1.0f / atlasSize;
        
        float4 t1 = fontAtlas.Sample(fontSampler, input.texCoord + float2(texelSize.x, 0.0f));
        float4 t2 = fontAtlas.Sample(fontSampler, input.texCoord + float2(-texelSize.x, 0.0f));
        float4 t3 = fontAtlas.Sample(fontSampler, input.texCoord + float2(0.0f, texelSize.y));
        float4 t4 = fontAtlas.Sample(fontSampler, input.texCoord + float2(0.0f, -texelSize.y));
        
        float a1 = t1.r;
        float a2 = t2.r;
        float a3 = t3.r;
        float a4 = t4.r;
        
        // Média ponderada
        alpha = lerp(alpha, (alpha + a1 + a2 + a3 + a4) * 0.2f, smoothing);
    }
    
    // Prepara a cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    // Clampa valores finais
    return saturate(finalColor);
}

// Versão simplificada para performance
float4 mainFast(VertexOutput input) : SV_Target {
    // Amostra a textura de fonte
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    float alpha = texColor.r; // Para R8_UNORM, o valor está no canal R
    
    // Cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return saturate(finalColor);
} 