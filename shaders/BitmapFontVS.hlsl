// Vertex Shader para renderização de fontes bitmap
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

VertexOutput main(VertexInput input) {
    VertexOutput output;
    
    // Aplica escala
    float2 scaledPos = input.position * input.scale;
    
    // Aplica rotação se necessário
    float2 rotatedPos = scaledPos;
    if (input.rotation != 0.0f) {
        float s = sin(input.rotation);
        float c = cos(input.rotation);
        rotatedPos = float2(
            scaledPos.x * c - scaledPos.y * s,
            scaledPos.x * s + scaledPos.y * c
        );
    }
    
    // Aplica offset
    float2 finalPos = rotatedPos + float2(input.offsetX, input.offsetY);
    
    // Converte para coordenadas de tela
    float2 screenPos = (finalPos / screenSize) * 2.0f - 1.0f;
    screenPos.y = -screenPos.y; // Inverte Y para coordenadas de tela
    
    // Posição final
    output.position = float4(screenPos, 0.0f, 1.0f);
    
    // Coordenadas de textura
    output.texCoord = input.texCoord;
    
    // Cor (já convertida para RGBA no UIBatcher)
    output.color = input.color;
    
    return output;
} 