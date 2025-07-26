// Pixel Shader para renderização de texto bitmap
// Funciona com texturas RGBA8_UNORM

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Textura do atlas de fontes (RGBA8_UNORM)
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

// Função para calcular alpha a partir da textura bitmap
float CalculateBitmapAlpha(float4 texColor) {
    // Para texturas bitmap, usamos o canal alpha ou a média dos canais RGB
    return texColor.a;
}

// Função para aplicar suavização básica
float ApplySmoothing(float alpha) {
    // Suavização simples para melhorar a qualidade
    return smoothstep(0.0, 1.0, alpha);
}

// Função principal do pixel shader
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura bitmap
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    
    // Calcula o alpha a partir da textura
    float alpha = CalculateBitmapAlpha(texColor);
    
    // Aplica suavização básica
    alpha = ApplySmoothing(alpha);
    
    // Prepara a cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return finalColor;
} 