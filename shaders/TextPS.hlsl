// Pixel Shader para renderização de texto bitmap
// Funciona com texturas R8_UNORM (um canal)

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Textura do atlas de fontes (R8_UNORM)
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

// Função para calcular alpha a partir da textura bitmap
float CalculateBitmapAlpha(float texValue) {
    // Para texturas R8_UNORM, o valor é diretamente o alpha
    return texValue;
}

// Função para aplicar suavização básica
float ApplySmoothing(float alpha) {
    // Suavização simples para melhorar a qualidade
    return smoothstep(0.0, 1.0, alpha);
}

// Função principal do pixel shader
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura bitmap (um canal)
    float alpha = fontAtlas.Sample(fontSampler, input.texCoord).r;
    
    // Calcula o alpha a partir da textura
    alpha = CalculateBitmapAlpha(alpha);
    
    // Aplica suavização básica
    alpha = ApplySmoothing(alpha);
    
    // Prepara a cor final
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return finalColor;
} 