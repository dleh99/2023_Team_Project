#include "Particle.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(-200, 200);

CParticle::CParticle() {

	m_fLifeTime = 3.f;
	m_fspeed = abs(dist(gen)) % 50 + 100;
	m_bStart = false;
	m_f3Direction = {};
	m_fGravity = 4.f;
	
	float rdscale = abs(dist(gen)) % 10 + 10;
	m_fScale = rdscale / 100.0f;

	m_f3Direction.x = dist(gen);
	m_f3Direction.y = std::abs(dist(gen)) + 200;
	m_f3Direction.z = dist(gen);

	m_f3Direction = Vector3::Normalize(m_f3Direction);

	SetScale(m_fScale, m_fScale, m_fScale);
}

void CParticle::Animate(float time) {
	//std::cout << "이거맞나" << std::endl;
	if (m_fLifeTime < 0) {
		return;
	}

	XMFLOAT3 pos = GetPosition();

	pos.x += m_fspeed * m_f3Direction.x * time;
	pos.y += m_fspeed * m_f3Direction.y * time - 0.5 * m_fGravity * time * time;
	pos.z += m_fspeed * m_f3Direction.z * time;

	m_f3Direction.y -= m_fGravity * time;

	SetPosition(pos);

	m_fLifeTime -= time;
}

void CParticle::SetDiretion(XMFLOAT3 dir) {
	m_f3Direction = dir;
}