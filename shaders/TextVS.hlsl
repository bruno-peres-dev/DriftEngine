// Vertex Shader para renderização de texto bitmap
// Suporte a transformações básicas

struct VertexInput {
    float2 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Constantes do shader
cbuffer TextConstants : register(b0) {
    float4x4 viewProjection;
}

VertexOutput main(VertexInput input) {
    VertexOutput output;
    
    // Aplica matriz de view-projection
    output.position = mul(viewProjection, float4(input.position, 0.0, 1.0));
    
    // Passa coordenadas de textura
    output.texCoord = input.texCoord;
    
    // Passa cor
    output.color = input.color;
    
    return output;
} 