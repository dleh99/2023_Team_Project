
struct MATERIAL
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular;
	float4 m_cEmissive;
};

// 게임 객체의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbGameObjectInfo : register(b0)
{
	matrix gmtxWorld : packoffset(c0);
	MATERIAL gMaterial : packoffset(c4);
}

// 카메라의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
	float3 gvCameraPosition : packoffset(c8);
}

#include "Light.hlsl"

struct CB_TO_LIGHT_SPACE
{
	matrix				mtxToTextureSpace;
	float4				f4Position;
};

cbuffer cbToLightSpace : register(b6)
{
	CB_TO_LIGHT_SPACE	gcbToLightSpaces[MAX_LIGHTS];
};

// 정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// 정점 세이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Texture2D gtxtTexture : register(t0);
TextureCube gtxtSkyCubeTexture : register(t1);
Texture2D gtxtAlbedoTexture : register(t2);

SamplerState gSamplerState : register(s0);
SamplerState gssClamp : register(s1);
// 정점 셰이더를 정의한다.
VS_OUTPUT VSDiffused(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return output;
}

// 픽셀 셰이더를 정의한다.
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	float4 color = {1,0,0,1};

	return color;
}

/// ////////////////////////////////////////////////////////////////////////////////////////////

struct VS_PLAYER_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : UV;
};

struct VS_PLAYER_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
	float4 shadowMapUVs[MAX_LIGHTS] : TEXCOORD1;
};

////////// Player Mesh
VS_PLAYER_OUTPUT VSPlayerDiffused(VS_PLAYER_INPUT input)
{
	VS_PLAYER_OUTPUT output = (VS_PLAYER_OUTPUT)0;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.normal = mul(input.normal, (float3x3)gmtxWorld);
	output.tangent = input.tangent;
	output.uv = input.uv;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.shadowMapUVs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTextureSpace);
	}

	return output;
}

float4 PSPlayerDiffused(VS_PLAYER_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtAlbedoTexture.Sample(gSamplerState, input.uv);
	float3 normalW = normalize(input.normal);
	float3 positionW = float3(input.position.x, input.position.y, input.position.z);
	float4 shadowMapUVs[MAX_LIGHTS];

	float4 cIllumination = Lighting(positionW, normalW, false, shadowMapUVs);

	//return cColor;
	return lerp(cColor, cIllumination, 0.4f);
}

/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Block
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;	
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 positionW : POSITION;
	float4 shadowMapUVs[MAX_LIGHTS] : TEXCOORD1;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.normal = mul(input.normal, (float3x3)gmtxWorld);

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.shadowMapUVs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTextureSpace);
	}

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
	float3 normalW = normalize(input.normal);

	float4 cIllumination = Lighting(input.position.xyz, normalW, true, input.shadowMapUVs);

	return lerp(cColor, cIllumination, 0.5f);

	//return(cColor);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b2)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b3)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : UV;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

struct VS_SKINNED_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	float4 shadowMapUVs[MAX_LIGHTS] : TEXCOORD1;
};

VS_SKINNED_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_SKINNED_STANDARD_OUTPUT output = (VS_SKINNED_STANDARD_OUTPUT)0;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; ++i)
	{
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}

	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.tangentW = input.tangent;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	[unroll]
	for (int j = 0; j < MAX_LIGHTS; j++)
	{
		if (gcbToLightSpaces[j].f4Position.w != 0.0f) output.shadowMapUVs[j] = mul(positionW, gcbToLightSpaces[j].mtxToTextureSpace);
	}

	return output;
}

float4 PSSkinnedAnimationStandard(VS_SKINNED_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtAlbedoTexture.Sample(gSamplerState, input.uv);
	float3 normalW = normalize(input.normalW);
	float4 shadowMapUVs[MAX_LIGHTS];

	float4 cIllumination = Lighting(input.positionW, normalW, false, shadowMapUVs);

	//return cColor;
	return lerp(cColor, cIllumination, 0.4f);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return output;
}

struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;

	float4 shadowMapUVs[MAX_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxWorld).xyz;

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.shadowMapUVs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTextureSpace);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.shadowMapUVs);

	return(cIllumination);
}

struct VS_SHADOWTEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SHADOWTEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_SHADOWTEXTURED_OUTPUT output = (VS_SHADOWTEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return output;
}

SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_SHADOWTEXTURED_OUTPUT input) : SV_Target
{
	float fDepthFromLight0 = gtxtDepthTextures[0].SampleLevel(gssBorder, input.uv, 0).r;

	return (float4)(fDepthFromLight0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_OUTPUT_INSTANCE
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;

	float3 normal : NORMAL;
	float3 positionW : POSITION;
	float4 shadowMapUVs[MAX_LIGHTS] : TEXCOORD1;
};

struct VS_INPUT_INSTANCE
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	matrix worldMatrix : INSTANCE;
	float3 normal : NORMAL;
};

VS_OUTPUT_INSTANCE VSInstancing(VS_INPUT_INSTANCE input)
{
	VS_OUTPUT_INSTANCE output = (VS_OUTPUT_INSTANCE)0;

	//float4 positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	float4 positionW = mul(float4(input.position, 1.0f), input.worldMatrix);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);

	//output.position = mul(mul(mul(float4(input.position, 1.0f), input.worldMatrix), gmtxView), gmtxProjection);
	output.uv = input.uv;

	output.normal = mul(input.normal, (float3x3)input.worldMatrix);

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.shadowMapUVs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTextureSpace);
	}

	return output;


}

float4 PSInstancing(VS_OUTPUT_INSTANCE input) : SV_TARGET
{
		float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
		float3 normalW = normalize(input.normal);
		//float4 cColor = { 1,1,1,1 };

		float4 cIllumination = Lighting(input.position.xyz, normalW, true, input.shadowMapUVs);

		//return(cColor);
		return lerp(cColor, cIllumination, 0.5f);
}