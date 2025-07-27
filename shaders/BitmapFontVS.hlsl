// Vertex Shader para renderização de fontes bitmap simples
// Compatível com stbtt_BakeFontBitmap e estrutura UIVertex

struct VertexInput {
    float2 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    uint textureId : TEXCOORD1;
    float offsetX : TEXCOORD2;
    float offsetY : TEXCOORD3;
    float scale : TEXCOORD4;
    float rotation : TEXCOORD5;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Constantes do shader
cbuffer UIConstants : register(b0) {
    float2 screenSize;
    float2 atlasSize;
    float2 padding;
    float time;
    float4 debugColor;
}

VertexOutput main(VertexInput input) {
    VertexOutput output;
    
    // CRÍTICO: Coordenadas já estão em clip space, usar diretamente
    // A conversão já foi feita no UIBatcher usando ToClipX() e ToClipY()
    output.position = float4(input.position, 0.0f, 1.0f);
    
    // Coordenadas de textura
    output.texCoord = input.texCoord;
    
    // Cor (já convertida para RGBA no UIBatcher)
    output.color = input.color;
    
    return output;
} 