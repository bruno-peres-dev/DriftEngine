// ============================================================================
// VERTEX SHADER MSDF - SISTEMA DE FONTES AVANÇADO
// ============================================================================
// Este shader processa vértices para renderização de texto MSDF

#include "Common.hlsl"

// ============================================================================
// ESTRUTURAS DE ENTRADA E SAÍDA
// ============================================================================

// Estrutura de entrada para vértices de texto
struct TextVertexInput
{
    float2 position : POSITION;     // Posição em coordenadas de tela
    float2 uv : TEXCOORD0;          // Coordenadas UV para atlas
    float4 color : COLOR;           // Cor do vértice
    uint textureId : TEXCOORD1;     // ID da textura (para múltiplos atlas)
};

// Estrutura de saída para o pixel shader
struct TextVertexOutput
{
    float4 position : SV_Position;  // Posição no clip space
    float2 uv : TEXCOORD0;          // Coordenadas UV
    float4 color : COLOR;           // Cor interpolada
    uint textureId : TEXCOORD1;     // ID da textura
    float2 screenPos : TEXCOORD2;   // Posição em coordenadas de tela
};

// ============================================================================
// CONSTANTES E BUFFERS
// ============================================================================

// Buffer de constantes para transformações
cbuffer TextConstants : register(b0)
{
    float4x4 projectionMatrix;      // Matriz de projeção
    float4x4 viewMatrix;            // Matriz de view
    float4x4 modelMatrix;           // Matriz de modelo
    float2 screenSize;              // Tamanho da tela
    float2 viewportOffset;          // Offset do viewport
    float2 padding;                 // Padding para alinhamento
};

// Buffer de constantes específicas do texto
cbuffer TextRenderConstants : register(b1)
{
    float2 textPosition;            // Posição base do texto
    float2 textScale;               // Escala do texto
    float textRotation;             // Rotação do texto (radianos)
    float2 textPivot;               // Ponto de pivô para rotação
    float2 textAlignment;           // Alinhamento do texto
    float2 textBounds;              // Dimensões do texto
    float2 padding2;                // Padding adicional
};

// ============================================================================
// FUNÇÕES UTILITÁRIAS
// ============================================================================

// Converte coordenadas de tela para clip space
float4 screenToClip(float2 screenPos, float2 screenSize)
{
    // Converter de coordenadas de tela (0 a screenSize) para clip space (-1 a 1)
    float2 clipPos;
    clipPos.x = (screenPos.x / screenSize.x) * 2.0f - 1.0f;
    clipPos.y = 1.0f - (screenPos.y / screenSize.y) * 2.0f; // Inverter Y
    
    return float4(clipPos, 0.0f, 1.0f);
}

// Aplica transformação de texto
float2 transformTextPosition(float2 position, float2 scale, float rotation, float2 pivot)
{
    // Aplicar escala
    float2 scaledPos = position * scale;
    
    // Aplicar rotação se necessário
    if (rotation != 0.0f)
    {
        float cosRot = cos(rotation);
        float sinRot = sin(rotation);
        
        // Transladar para o pivô
        float2 centeredPos = scaledPos - pivot;
        
        // Aplicar rotação
        float2 rotatedPos;
        rotatedPos.x = centeredPos.x * cosRot - centeredPos.y * sinRot;
        rotatedPos.y = centeredPos.x * sinRot + centeredPos.y * cosRot;
        
        // Transladar de volta
        scaledPos = rotatedPos + pivot;
    }
    
    return scaledPos;
}

// Calcula alinhamento do texto
float2 calculateTextAlignment(float2 position, float2 bounds, float2 alignment)
{
    // alignment: (-1 = left/top, 0 = center, 1 = right/bottom)
    float2 offset;
    offset.x = bounds.x * (alignment.x * 0.5f);
    offset.y = bounds.y * (alignment.y * 0.5f);
    
    return position - offset;
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO VERTEX SHADER
// ============================================================================

TextVertexOutput MSDFFontVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar transformações de texto
    float2 transformedPos = transformTextPosition(
        input.position, 
        textScale, 
        textRotation, 
        textPivot
    );
    
    // Aplicar posição base do texto
    transformedPos += textPosition;
    
    // Aplicar alinhamento se necessário
    if (any(textAlignment != 0.0f))
    {
        transformedPos = calculateTextAlignment(transformedPos, textBounds, textAlignment);
    }
    
    // Aplicar viewport offset
    transformedPos += viewportOffset;
    
    // Converter para clip space
    output.position = screenToClip(transformedPos, screenSize);
    
    // Passar coordenadas UV
    output.uv = input.uv;
    
    // Passar cor
    output.color = input.color;
    
    // Passar ID da textura
    output.textureId = input.textureId;
    
    // Passar posição em coordenadas de tela para efeitos
    output.screenPos = transformedPos;
    
    return output;
}

// ============================================================================
// VERSÕES OTIMIZADAS PARA DIFERENTES CASOS DE USO
// ============================================================================

// Versão simples sem transformações complexas
TextVertexOutput MSDFFontSimpleVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar apenas posição base e escala
    float2 transformedPos = input.position * textScale + textPosition;
    
    // Converter para clip space
    output.position = screenToClip(transformedPos, screenSize);
    
    // Passar dados básicos
    output.uv = input.uv;
    output.color = input.color;
    output.textureId = input.textureId;
    output.screenPos = transformedPos;
    
    return output;
}

// Versão para texto estático (sem animações)
TextVertexOutput MSDFFontStaticVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar transformações básicas
    float2 transformedPos = input.position + textPosition;
    
    // Converter para clip space
    output.position = screenToClip(transformedPos, screenSize);
    
    // Passar dados
    output.uv = input.uv;
    output.color = input.color;
    output.textureId = input.textureId;
    output.screenPos = transformedPos;
    
    return output;
}

// Versão para texto com matriz de transformação completa
TextVertexOutput MSDFFontMatrixVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar matriz de transformação completa
    float4 worldPos = mul(float4(input.position, 0.0f, 1.0f), modelMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    float4 clipPos = mul(viewPos, projectionMatrix);
    
    output.position = clipPos;
    
    // Passar dados
    output.uv = input.uv;
    output.color = input.color;
    output.textureId = input.textureId;
    output.screenPos = input.position;
    
    return output;
}

// ============================================================================
// VERSÕES PARA EFEITOS ESPECIAIS
// ============================================================================

// Versão para texto com efeito de onda
TextVertexOutput MSDFFontWaveVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar efeito de onda
    float time = 0.0f; // TODO: Passar tempo via constantes
    float waveOffset = sin(input.position.x * 0.1f + time) * 5.0f;
    
    float2 wavePos = input.position + float2(0.0f, waveOffset);
    float2 transformedPos = transformTextPosition(wavePos, textScale, textRotation, textPivot);
    transformedPos += textPosition;
    
    // Converter para clip space
    output.position = screenToClip(transformedPos, screenSize);
    
    // Passar dados
    output.uv = input.uv;
    output.color = input.color;
    output.textureId = input.textureId;
    output.screenPos = transformedPos;
    
    return output;
}

// Versão para texto com efeito de escala
TextVertexOutput MSDFFontScaleVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Aplicar efeito de escala pulsante
    float time = 0.0f; // TODO: Passar tempo via constantes
    float scale = 1.0f + sin(time * 2.0f) * 0.1f;
    
    float2 scaledPos = input.position * textScale * scale;
    float2 transformedPos = scaledPos + textPosition;
    
    // Converter para clip space
    output.position = screenToClip(transformedPos, screenSize);
    
    // Passar dados
    output.uv = input.uv;
    output.color = input.color;
    output.textureId = input.textureId;
    output.screenPos = transformedPos;
    
    return output;
}

// ============================================================================
// VERSÕES PARA DEBUG E DESENVOLVIMENTO
// ============================================================================

// Debug: Mostra vértices como pontos
TextVertexOutput MSDFFontDebugVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Renderizar como pontos para debug
    float2 transformedPos = input.position + textPosition;
    output.position = screenToClip(transformedPos, screenSize);
    
    // Cor de debug
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.uv = input.uv;
    output.textureId = input.textureId;
    output.screenPos = transformedPos;
    
    return output;
}

// Debug: Mostra bounds do texto
TextVertexOutput MSDFFontBoundsDebugVS(TextVertexInput input)
{
    TextVertexOutput output;
    
    // Renderizar bounds do texto
    float2 boundsPos = input.position * textBounds + textPosition;
    output.position = screenToClip(boundsPos, screenSize);
    
    // Cor de debug para bounds
    output.color = float4(0.0f, 1.0f, 0.0f, 0.5f);
    output.uv = input.uv;
    output.textureId = input.textureId;
    output.screenPos = boundsPos;
    
    return output;
} 