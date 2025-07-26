// Pixel Shader para renderização de fontes bitmap simples
// Funciona com texturas R8_UNORM (um canal)

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Textura do atlas de fontes (R8_UNORM)
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

// Função principal do pixel shader
float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura bitmap (um canal)
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    
    // Para texturas R8_UNORM, o valor está no canal R
    float alpha = texColor.r;
    
    // Aplicar suavização básica para melhor qualidade
    alpha = smoothstep(0.0f, 1.0f, alpha);
    
    // Aplica a cor do texto
    float4 finalColor = input.color;
    finalColor.a *= alpha;
    
    return finalColor;
} 