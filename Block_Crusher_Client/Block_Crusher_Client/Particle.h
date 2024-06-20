#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CParticle : public CGameObject
{
private:
	float m_fLifeTime;
	float m_fTime;
	float m_fspeed;
	float m_fGravity;
	float m_fScale;
	bool m_bStart;
	XMFLOAT3 m_f3Direction;

public:

	CParticle();
	~CParticle(){};

	bool isDead() { return (m_fLifeTime < 0); };
	void Animate(float time);
	void SetDiretion(XMFLOAT3 dir);
};

