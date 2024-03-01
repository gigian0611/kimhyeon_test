static const float4 QuadNDCoords[4] = {
        float4(-1, -1, 0, 1),
        float4(-1,  1, 0, 1),
        float4(1, -1, 0, 1),
        float4(1,  1, 0, 1)
};

static const float2 QuadUVCoords[4] = {
    float2(0, 1),
    float2(0, 0),
    float2(1, 1),
    float2(1, 0)
};

struct VSInput
{
    uint vId     : SV_VertexID;
};


struct PSInput
{
    float4 ndc  : SV_POSITION;
    float2 uv   : TEXCOORD;
};

cbuffer cb0 : register(b0)
{
    uint2 texture_size;
    uint2 tile_size;
    uint2 virtual_texture_size;
    uint2 pad;
};

Texture2D indirectTexture : register(t0);
Texture2D randomTexture : register(t1);
SamplerState sampler0 : register(s0);

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.ndc = QuadNDCoords[input.vId];
    result.uv = QuadUVCoords[input.vId];
    return result;
}

void PSMain(
    PSInput input,
    out float4 outTarget0 : SV_TARGET0
)
{
    uint2 indirectTextureSize = texture_size;
    uint2 tileSize = tile_size;
    uint2 virtualTextureSize = uint2(virtual_texture_size.x / indirectTextureSize.x, virtual_texture_size.y / indirectTextureSize.y);

    uint2 first = uint2(input.ndc.x / indirectTextureSize.x, input.ndc.y / indirectTextureSize.y);
    uint2 second =  uint2(input.ndc.x % indirectTextureSize.x, input.ndc.y % indirectTextureSize.y);

    float random_pixel = randomTexture.Load(uint3(first.xy, 0)).xyz * float3(tileSize.xy, 0.0).x;
    float3 result = indirectTexture.Sample(sampler0,(input.uv + float2(random_pixel, 0.0)) * virtualTextureSize / tileSize).xyz;

    outTarget0 = float4(result, 1.0);
}
