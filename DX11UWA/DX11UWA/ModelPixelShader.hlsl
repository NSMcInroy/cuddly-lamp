


texture2D baseTexture : register(t0); // first texture

texture2D detailTexture : register(t1); // second texture

SamplerState filters[2] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer DirectionConstantBuffer : register(b0)
{
	float4 dir_direction;
	float4 dir_color;
};

cbuffer PointConstantBuffer : register(b1)
{
	float4 point_pos;
	float4 point_color;
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
float4 finalColor = baseTexture.Sample(filters[0], input.uv);// *modulate; // get base color

	//directional light
	float lightRatio = dot(-dir_direction.xyz, input.normals);
float3 dirColor = dir_color.xyz * lightRatio;

//point light
float attenuation = 1.0f - saturate(length(point_pos.xyz - input.posw) / 3.0f);
float3 pointDir = point_pos.xyz - input.posw;
float pointRatio = saturate(dot(normalize(pointDir), input.normals));
float3 pointColor = point_color.xyz * finalColor.xyz * pointRatio *attenuation;

float3 lightsColor = saturate(dirColor + pointColor);


finalColor.xyz = saturate(finalColor.xyz * lightsColor);

return finalColor; // return a transition based on the detail alpha
//return float4(input.uv, 1.0f);
}
