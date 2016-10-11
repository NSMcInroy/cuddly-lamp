


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
	float3 cameraposw : CAMERA;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
float4 finalColor = baseTexture.Sample(filters[0], input.uv); // get base color
if (finalColor.a < 0.6)
	discard;

float3 bumpNormal = input.normals;
if (input.normalmap == 1.0f)
{

float4 bumpMap = normalmapTexture.Sample(filters[0], input.uv);
bumpMap = (bumpMap * 2.0f) - 1.0f;
bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normals);
bumpNormal = normalize(bumpNormal);
}

float3 ViewVector = normalize(input.posw - input.cameraposw);
//directional light
float dirlightRatio = dot(-dir_direction.xyz, bumpNormal);
float3 dirColor = dir_color.xyz * dirlightRatio;

//Specular directional
float3 dirRefelection = normalize(reflect(-dir_direction.xyz, bumpNormal));
float dirRdotV = max(0, dot(dirRefelection, ViewVector));
float3 dirSpecColor = saturate(dirColor * pow(dirRdotV, 3));

//point light
float pointAttenuation = 1.0f - saturate(length(point_pos.xyz - input.posw) / 5.0f);
float3 pointDir = point_pos.xyz - input.posw;
float pointRatio = saturate(dot(normalize(pointDir), bumpNormal));
float3 pointColor = point_color.xyz  * pointRatio * pointAttenuation;


//Specular point
float3 pointRefelection = normalize(reflect(-pointDir, bumpNormal));
float pointRdotV = max(0, dot(pointRefelection, ViewVector));
float3 pointSpecColor = saturate(pointColor * pow(pointRdotV, 3));


//Spotlight
float3 spotlightDir = normalize(spot_pos.xyz - input.posw);
float surfaceRatio = saturate(dot(-spotlightDir, spot_conedir.xyz));
float spotFactor = (surfaceRatio > spot_coneratio.y) ? 1 : 0;
float spotRange = (spotlightDir < spot_coneratio.z) ? 1 : 0;
float spotlightRatio = saturate(dot(spotlightDir, bumpNormal));
float spotAttenuation = 1.0f - saturate((spot_coneratio.x - surfaceRatio) / (spot_coneratio.x - spot_coneratio.y));
float3 spotlightColor = spotFactor * spotlightRatio * spot_color.xyz * spotAttenuation * spotRange;

//Specular spotlight
float3 spotRefelection = normalize(reflect(-spotlightDir, bumpNormal));
float spotRdotV = max(0, dot(spotRefelection, ViewVector));
float3 spotSpecColor = saturate(spotlightColor * pow(spotRdotV, 3));

float3 lightsColor = saturate( pointColor + pointSpecColor + spotlightColor + spotSpecColor + dirColor + dirSpecColor);
float3 ambient = { 0.1,0.1,0.1 };
lightsColor = saturate(lightsColor + ambient);

finalColor.xyz = saturate(finalColor.xyz * lightsColor );

return finalColor; // return a transition based on the detail alpha
//return float4(input.uv, 1.0f);
}
