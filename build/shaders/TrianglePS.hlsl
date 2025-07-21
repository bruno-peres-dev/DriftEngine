// shaders/TrianglePS.hlsl
struct PSInput
{
    float4 pos : SV_POSITION;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1,1,0,1); // amarelo
}
