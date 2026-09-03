
struct VertexOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
}

static const float2 kPositions[3] = float2[3](
    float2(-0.66, 0.66),
    float2(0.66, 0.66),
    float2(0.0, -0.66)
);

static const float3 kColors[3] = float3[3](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

[shader("vertex")]
VertexOutput VertexMain(uint vertexID : SV_VertexID)
{
    return VertexOutput(float4(kPositions[vertexID], 0.5, 1.0), kColors[vertexID]);
}

[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
