// UIBatch.hlsl - shader simples para UI 2D
struct VSIn {
    float2 pos  : POSITION;
    float4 col  : COLOR0; // UNORM convertida para float automaticamente
};

struct PSIn {
    float4 pos  : SV_POSITION;
    float4 col  : COLOR0;
};

PSIn VSMain(VSIn v) {
    PSIn o;
    o.pos = float4(v.pos, 0.0, 1.0);
    o.col = v.col;
    return o;
}

float4 PSMain(PSIn i) : SV_TARGET { return i.col; } 