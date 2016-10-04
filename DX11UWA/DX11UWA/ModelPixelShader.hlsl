


texture2D baseTexture : register(t0); // first texture

texture2D normalmapTexture : register(t1); // second texture

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
cbuffer SpotlightConstantBuffer : register(b2)
{
	float4 spot_pos;
	float4 spot_color;
	float4 spot_conedir;
	float4 spot_coneratio;
};


// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float normalmap : NORMALMAP;
	float3 normals : NORMAL;
	float3 posw : WORLD;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
float4 finalColor = baseTexture.Sample(filters[0], input.uv); // get base color


float3 bumpNormal = input.normals;
if (input.normalmap == 1.0f)
{

float4 bumpMap = normalmapTexture.Sample(filters[0], input.uv);
bumpMap = (bumpMap * 2.0f) - 1.0f;
bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normals);
bumpNormal = normalize(bumpNormal);
}

//directional light
float dirlightRatio = dot(-dir_direction.xyz, bumpNormal);
float3 dirColor = dir_color.xyz * dirlightRatio;

//point light
float pointAttenuation = 1.0f - saturate(length(point_pos.xyz - input.posw) / 3.0f);
float3 pointDir = point_pos.xyz - input.posw;
float pointRatio = saturate(dot(normalize(pointDir), bumpNormal));
float3 pointColor = point_color.xyz  * pointRatio * pointAttenuation;

//Spotlight
float3 spotlightDir = normalize(spot_pos.xyz - input.posw);
float surfaceRatio = saturate(dot(-spotlightDir, spot_conedir.xyz));
float spotFactor = (surfaceRatio > spot_coneratio.y) ? 1 : 0;
float spotlightRatio = saturate(dot(spotlightDir, bumpNormal));
float spotAttenuation = 1.0f - saturate((spot_coneratio.x - surfaceRatio) / (spot_coneratio.x - spot_coneratio.y));
float3 spotlightColor = spotFactor * spotlightRatio * spot_color.xyz * spotAttenuation;


float3 lightsColor = saturate( pointColor + spotlightColor + dirColor);


finalColor.xyz = saturate(finalColor.xyz * lightsColor);

return finalColor; // return a transition based on the detail alpha
//return float4(input.uv, 1.0f);
}
