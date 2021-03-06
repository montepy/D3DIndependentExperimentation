cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};
//note: keep structure of structs in fx files same as those in c++ code.
struct Light {
	float3 dir;
	float3 att;
	float3 pos;
	float range;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame {
	Light light;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal: NORMAL;
	float4 worldPos: POSITION;
};
Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_OUTPUT VS(float4 inPos: POSITION, float2 texCoord : TEXCOORD, float3 normal : NORMAL) {
	VS_OUTPUT output;
	output.texCoord = texCoord;
	output.Pos = mul(inPos, WVP);
	output.worldPos = mul(inPos, World);
	output.normal = mul(normal, World);
	return output;
}


float4 PS(VS_OUTPUT input) : SV_TARGET{
	input.normal = normalize(input.normal);

float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.texCoord);
float3 finalColor = float3(0.0, 0.0, 0.0);
float3 lightToPixelVec = light.pos - input.worldPos;
float d = length(lightToPixelVec);


float3 finalAmbient = diffuse*light.ambient;
if (d > light.range) {
	return float4(finalAmbient, diffuse.a);
}
lightToPixelVec /= d;
float Intensity = dot(lightToPixelVec, input.normal);
if (Intensity > 0.0f) {
	finalColor += Intensity*diffuse*light.diffuse;
	finalColor /= light.att[0] + (light.att[1] * d) + (light.att[2] * (d*d));
}
finalColor = saturate(finalColor + finalAmbient);

return float4(finalColor,diffuse.a);

}

float4 D2D_PS(VS_OUTPUT input) : SV_TARGET
{ input.normal = normalize(input.normal);
float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.texCoord);

return diffuse;
}