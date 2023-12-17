#pragma once
#include "Mesh.h"
#include "Camera.h"

class CGameObject;
class CMeshLoadInfo;
class CMesh;
class CSkinnedMesh;

#define TYPE_PLAYER 0
#define TYPE_BLOCK 1
#define TYPE_BULLET 2

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CShader;

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType;

	int								m_nTextures = 0;
	ID3D12Resource** m_ppd3dTextures = NULL;
	ID3D12Resource** m_ppd3dTextureUploadBuffers;

	UINT* m_pnResourceTypes = NULL;

	_TCHAR(*m_ppstrTextureNames)[64] = NULL;

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
	int* m_pnBufferElements = NULL;

	int								m_nRootParameters = 0;
	int* m_pnRootParameterIndices = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName, UINT nResourceType, UINT nIndex);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	_TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
	ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
	int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	std::string						m_strTextureName;
	CTexture*						m_pTexture = NULL;
	CShader*						m_pShader = NULL;

	void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetTexture(CTexture* pTexture);
	void SetShader(CShader* pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};

struct CALLBACKKEY
{
	float				m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() {  }
	~CAnimationCallbackHandler() {  }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) {  }
};

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	~CAnimationSet();

public:
	char				m_pstrAnimationSetName[64];

	float				m_fLength = 0.0f;
	int					m_nFramePerSecond = 0;
	int					m_nKeyFrames = 0;
	float* m_pKeyFrameTimes = NULL;
	XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;

public:
	XMFLOAT4X4 GetSRT(int nBone, float fPosition);
};

class CAnimationSets
{
public:
	CAnimationSets(int nAnimationSets);
	~CAnimationSets();

private:
	int					m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	int					m_nAnimationSets = 0;
	CAnimationSet** m_pAnimationSets = NULL;

	int					m_nBoneFrames = 0;
	CGameObject** m_ppBoneFrameCaches = NULL;
};

class CAnimationTrack
{
public:
	CAnimationTrack() {  }
	~CAnimationTrack();

public:
	BOOL				m_bEnable = true;
	float				m_fSpeed = 1.0f;
	float				m_fPosition = -ANIMATION_CALLBACK_EPSILON;
	float				m_fWeight = 1.0f;

	int					m_nAnimationSet = 0;

	int					m_nType = ANIMATION_TYPE_LOOP;

	int					m_nCallbackKeys = 0;
	CALLBACKKEY* m_pCallbackKeys = NULL;

	CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;

public:
	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }

	float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

	void HandleCallback();
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL;

	CAnimationSets* m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};

class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAnimationController();

public:
	float				m_fTime = 0.0f;

	int					m_nAnimationTracks = 0;
	CAnimationTrack* m_pAnimationTracks = NULL;

	CAnimationSets* m_pAnimationSets = NULL;

	int					m_nSkinnedMeshes = 0;
	CSkinnedMesh**		m_ppSkinnedMeshes = NULL;

	ID3D12Resource**	m_ppd3dcbSkinningBoneTransforms = NULL;
	XMFLOAT4X4**		m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL;

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTracksEnable(int nAnimationTrack);
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler);

	int GetTrueEnableAnimationTrack();

	void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);

public:
	bool				m_bRootMotion = false;
	CGameObject*		m_pModelRootObject = NULL;

	CGameObject*		m_pRootMotionObject = NULL;
	XMFLOAT3			m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) {  }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) {  }
};

//===============================================================//===============================================================

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	char m_pstrFrameName[64];
	int m_nFrames = 0;

	CMesh* m_pMesh = NULL;

	CMaterial* m_pMaterial = NULL;

	CShader* m_pShader = NULL;

	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4X4 m_xmf4x4World;

	float m_fBlockBoundingRadius;
	//| 0 - 주인공 | 1 - 블럭 | 2 - 총알 |
	int m_ObjType = TYPE_BLOCK;
	bool m_bActive = false;

	CGameObject* m_pParent = NULL;
	CGameObject* m_pChild = NULL;
	CGameObject* m_pSibling = NULL;

	CAnimationController* m_pSkinnedAnimationController = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

public:
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void ReleaseUploadBuffers();

	virtual void SetMesh(CMesh* pMesh);
	virtual void SetMaterial(CMaterial* pMaterial);
	virtual void SetShader(CShader* pShader);

	virtual void OnPrepareAnimation() {  }
	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// 상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// 상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// 게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	int GetObjectType();
	bool GetIsActive();

	float GetBoundingRadius() { return m_fBlockBoundingRadius; };

	// 게임 객체의 워치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float scale);
	void SetScale(float x, float y, float z);
	void SetObjectType(int type);
	void SetIsActive(bool Active);
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }

	// 게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void moveForward(float fDistance = 1.0f);

	// BS-> Bounding Sphere 바운딩 스피어끼리 충돌체크
	void BSCollisionCheck(XMFLOAT3 Position, float Radius) {};

	// 게임 객체를 회전(x-축, y-축, z-축)한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void SetChild(CGameObject* pChild);
	CGameObject* GetParent() { return m_pParent; }
	CGameObject* FindFrame(char* pstrFrameName);
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

public:
	void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	static CLoadedModelInfo* LoadModelAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrModelFileName, const char* pstrAnimationFileName,
		CShader* pPlayerMeshShader, CShader* pPlayerSkinnedMeshShader);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, std::ifstream& fileStream,
		CShader* pPlayerMeshShader, CShader* pPlayerSkinnedMeshShader, int* pnSkinnedMeshes);
	static void LoadAnimationFromFile(std::ifstream& fileStream, CLoadedModelInfo* pLoadedModel);

	static CMeshLoadInfo* LoadMeshInfoFromFile(std::ifstream& fileStream, float modelScaleFactor);
	static void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::ifstream& fileStream, CGameObject* pObj, CShader* pShader);
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed);
};

class CBlockObject : public CGameObject
{
public:
	CBlockObject() 
	{
		m_iBlockType = 0; 
		m_iBlockHP = 1;
	};
	virtual ~CBlockObject() {};

private:
	int m_iBlockType;
	int m_iBlockHP;

public:
	int GetBlockType();
	int GetBlockHP() { return m_iBlockHP; };
	void Animate(float fTimeElapsed) {};
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject()
	{
		m_iBulletType = 0;
		m_iBulletDamage = 1;
		m_xmf3Vector = { 0,0,0 };
		m_fSpeed = 200.0f;
		m_fBoundingSph = 2.0f;
		m_fBlockBoundingRadius = 2.0f;
	};
	virtual ~CBulletObject() {};

	void SetPlayerId(int x) { player_id = x; };
	void SetBulletId(int x) { bullet_id = x; };

	int GetPlayerId() { return player_id; };
	int GetBulletId() { return bullet_id; };

private:
	int m_iBulletType;
	int m_iBulletDamage;
	float m_fSpeed;
	float m_fBoundingSph;
	XMFLOAT3 m_xmf3Vector;

	int player_id;
	int bullet_id;


public:
	int GetBulletType() { return m_iBulletType; };
	int GetBulletDamage() { return m_iBulletDamage; };

	void SetBulletVector(XMFLOAT3 vector) { m_xmf3Vector = vector; };
	XMFLOAT3 GetBulletVector() { return m_xmf3Vector; };
	virtual void Animate(float fTimeElapsed);
};

class CSkyBox : public CGameObject
{
public:
	CSkyBox() = default;
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};