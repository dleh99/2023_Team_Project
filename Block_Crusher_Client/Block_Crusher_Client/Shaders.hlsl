
// 게임 객체의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbGameObjectInfo : register(b0)
{
	matrix gmtxWorld : packoffset(c0);
}

// 카메라의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
}


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
};

VS_PLAYER_OUTPUT VSPlayerDiffused(VS_PLAYER_INPUT input)
{
	VS_PLAYER_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.normal = input.normal;
	output.tangent = input.tangent;
	output.uv = input.uv;

	return output;
}

float4 PSPlayerDiffused(VS_PLAYER_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtAlbedoTexture.Sample(gSamplerState, input.uv);
	return cColor;

	//return float4(1,0,0,1);
}

/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
	//float4 aColor = { 1,1,1,1 };

	return(cColor);
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
};

VS_SKINNED_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_SKINNED_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; ++i)
	{
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}

	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = input.normal;
	output.tangentW = input.tangent;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return output;
}

float4 PSSkinnedAnimationStandard(VS_SKINNED_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtAlbedoTexture.Sample(gSamplerState, input.uv);

	return cColor;
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
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_OUTPUT_INSTANCE
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

struct VS_INPUT_INSTANCE
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	matrix worldMatrix : INSTANCE;
};

VS_OUTPUT_INSTANCE VSInstancing(VS_INPUT_INSTANCE input)
{
	VS_OUTPUT_INSTANCE output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), input.worldMatrix), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return output;
}

float4 PSInstancing(VS_OUTPUT_INSTANCE input) : SV_TARGET
{
		float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
		//float4 cColor = { 1,1,1,1 };

		return(cColor);
}