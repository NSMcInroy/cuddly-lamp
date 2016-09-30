


texture2D baseTexture : register(t0); // first texture

texture2D detailTexture : register(t1); // second texture

SamplerState filters[2] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer DirectionConstantBuffer : register(b0)
{
	float4 dir_direction;
	float4 dir_color;
};



// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normals : NORMAL;
	float3 posw : WORLD;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{

	float lightRatio = dot(-dir_direction.xyz, input.normals);
//return float4(input.uv, 1.0f);
float4 finalColor = baseTexture.Sample(filters[0], input.uv);// *modulate; // get base color
finalColor.xyz = saturate(finalColor.xyz * dir_color.xyz * lightRatio);
//float4 detailColor = detailTexture.Sample(filters[1], detailUV); // get detail effect
//float4 finalColor = float4(lerp(baseColor.rgb, detailColor.rgb, detailColor.a), baseColor.a);
return finalColor; // return a transition based on the detail alpha
}
