#include "GameObject.h"

GameObject::GameObject()
{
	pos.x = 0.f;
	pos.y = 0.f;
	pos.z = 0.f;

	isActive = false;

	ObjectBoundingRadius = 0.f;
}

GameObject::~GameObject()
{
}

XMFLOAT3 GameObject::GetPosition()
{
	return pos;
}

void GameObject::SetPosition(float x, float y, float z)
{
	pos = { x, y, z };
}

BulletObject::BulletObject()
{
	bullet_speed = 200.f;

	bullet_vec.x = 0.f;
	bullet_vec.y = 0.f;
	bullet_vec.z = 0.f;

	float rad = 2.f;
	SetRadius(rad);
}

BulletObject::~BulletObject()
{
}

XMFLOAT3 BulletObject::GetBulletVec()
{
	return bullet_vec;
}

void BulletObject::SetBulletVec(float x, float y, float z)
{
	bullet_vec = { x, y, z };
}

void BulletObject::Move(float fTimeElapsed)
{
	XMFLOAT3 position = GetPosition();
	XMFLOAT3 velocity;

	float speed = bullet_speed * fTimeElapsed;
	velocity = bullet_vec;
	velocity.x *= speed;
	velocity.y *= speed;
	velocity.z *= speed;

	position = { position.x + velocity.x, position.y + velocity.y, position.z + velocity.z };

	if (position.x > 200.0f || position.x < -200.0f) {
		SetisActive(false);
		return;
	}

	if (position.z > 200.0f || position.z < -200.0f) {
		SetisActive(false);
		return;
	}

	if (position.y > 200.0f || position.y < -200.0f) {
		SetisActive(false);
		return;
	}

	SetPosition(position.x, position.y, position.z);
}

Block::Block()
{
	float rad = sqrt(144.f * 3.f) / 2.f;
	SetRadius(rad);

	SetisActive(true);

	type = TYPE_NORMAL;
}

Block::Block(int input_id, XMFLOAT3 input_pos)
{
	SetId(input_id);
	SetPosition(input_pos.x, input_pos.y, input_pos.z);
	float rad = sqrt(144.f * 3.f) / 2.f;
	SetRadius(rad);
	SetisActive(true);
}

Block::~Block()
{
	
}