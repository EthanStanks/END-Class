#include "mvp.hlsli"

struct PSIn
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};


float4 main(PSIn inputVert) : SV_Target
{
    return inputVert.color;
}