Texture2D gDiffuse : register(t0);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewPos : TEXCOORD1;
};

// AAA Industry Standard: Enhanced lighting
float3 gLightDir = normalize(float3(0.5, -1, 0.5));
float3 gLightColor = float3(1.0, 0.95, 0.8);
float3 gAmbientColor = float3(0.2, 0.2, 0.25);

// AAA Industry Standard: LOD color coding
float3 GetLODColor(float3 worldPos) {
    // Simple distance-based LOD color visualization
    float distanceFromOrigin = length(worldPos.xz);
    
    if (distanceFromOrigin < 150.0)
        return float3(1.0, 0.0, 0.0); // Red - LOD0 (highest detail)
    else if (distanceFromOrigin < 300.0)
        return float3(0.0, 1.0, 0.0); // Green - LOD1
    else if (distanceFromOrigin < 600.0)
        return float3(0.0, 0.0, 1.0); // Blue - LOD2
    else
        return float3(1.0, 1.0, 0.0); // Yellow - LOD3 (lowest detail)
}

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef WIREFRAME
    return float4(0, 1, 0, 1); // Green wireframe
#else
    // Sample base texture
    float4 baseColor = gDiffuse.Sample(gSampler, input.uv);
    
    // Calculate basic lighting
    float3 normal = normalize(input.normal);
    float NdotL = max(0.0, dot(normal, -gLightDir));
    
    // Combine ambient and diffuse lighting
    float3 lighting = gAmbientColor + gLightColor * NdotL;
    
    // Apply lighting to base color
    float3 finalColor = baseColor.rgb * lighting;
    
#ifdef LOD_COLORS
    // Blend with LOD color for debugging
    float3 lodColor = GetLODColor(input.worldPos);
    finalColor = lerp(finalColor, lodColor, 0.3);
#endif
    
    // Add subtle distance fog for depth perception
    float fogDistance = length(input.viewPos);
    float fogFactor = saturate((fogDistance - 500.0) / 1000.0);
    float3 fogColor = float3(0.7, 0.8, 0.9);
    finalColor = lerp(finalColor, fogColor, fogFactor);
    
    return float4(finalColor, baseColor.a);
#endif
}
