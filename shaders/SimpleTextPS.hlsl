// Shader simples para teste de textura
struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Textura do atlas de fontes
Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

float4 main(VertexOutput input) : SV_Target {
    // Amostra a textura
    float4 texColor = fontAtlas.Sample(fontSampler, input.texCoord);
    
    // Retorna apenas o valor da textura (sem multiplicar pela cor)
    return texColor;
} 