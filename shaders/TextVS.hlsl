// Vertex Shader para renderização de texto com MSDF
// Suporte a transformações, anti-aliasing e qualidade AAA

struct VertexInput {
    float2 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    float2 offset : TEXCOORD1;
    float scale : TEXCOORD2;
    float rotation : TEXCOORD3;
};

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

// Função para aplicar transformações
float2 TransformPosition(float2 position, float2 offset, float scale, float rotation) {
    // Aplica escala
    position *= scale;
    
    // Aplica rotação
    float s = sin(rotation);
    float c = cos(rotation);
    float2 rotated = float2(
        position.x * c - position.y * s,
        position.x * s + position.y * c
    );
    
    // Aplica offset
    return rotated + offset;
}

// Função para calcular coordenadas de tela
float2 CalculateScreenPosition(float2 worldPos) {
    return (worldPos / screenSize) * 2.0 - 1.0;
}

VertexOutput main(VertexInput input) {
    VertexOutput output;
    
    // Aplica transformações
    float2 transformedPos = TransformPosition(input.position, input.offset, input.scale, input.rotation);
    
    // Converte para coordenadas de tela
    float2 screenPos = CalculateScreenPosition(transformedPos);
    
    // Aplica matriz de view-projection
    output.position = mul(viewProjection, float4(transformedPos, 0.0, 1.0));
    
    // Passa coordenadas de textura
    output.texCoord = input.texCoord;
    
    // Passa cor
    output.color = input.color;
    
    // Coordenadas de pixel para MSDF
    output.pixelPos = input.texCoord * atlasSize;
    
    // Posição na tela para efeitos
    output.screenPos = screenPos;
    
    return output;
} 