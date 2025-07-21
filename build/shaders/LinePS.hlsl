// LinePS.hlsl
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PS_IN input) : SV_TARGET {
    return input.color;
} 