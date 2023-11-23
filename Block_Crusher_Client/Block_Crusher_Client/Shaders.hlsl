
// ���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�.
cbuffer cbGameObjectInfo : register(b0)
{
	matrix gmtxWorld : packoffset(c0);
}

// ī�޶��� ������ ���� ��� ���۸� �����Ѵ�.
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
}


// ���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�.
struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// ���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// ���� ���̴��� �����Ѵ�.
VS_OUTPUT VSDiffused(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return output;
}

// �ȼ� ���̴��� �����Ѵ�.
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	return input.color;
}

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
	return float4(0.0f, 128.0f, 0.0f, 1.0f);
}