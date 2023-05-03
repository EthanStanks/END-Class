#include "mvp.hlsli"

struct VSIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};


VSOut main(VSIn inputVert)
{
    VSOut outputVert;
    outputVert.pos = float4(inputVert.pos, 1);
    outputVert.color = inputVert.color;
    
    outputVert.pos = mul(outputVert.pos, modeling);
    outputVert.pos = mul(outputVert.pos, view);
    outputVert.pos = mul(outputVert.pos, projection);
    
    return outputVert;
}