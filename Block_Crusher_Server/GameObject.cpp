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

Range_Pos GameObject::Convert_index(float x, float y, float z)
{
	Range_Pos temp;

	temp.x = ((x - 26) / 12) * (-1);
	temp.y = ((y + 114) / 12);
	temp.z = ((z - 46) / 12) * (-1);

	return temp;
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

	float rad = 1.f;
	SetRadius(rad);
}

BulletObject::~BulletObject()
{
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

	if (position.x > 800.0f || position.x < -800.0f) {
		SetisActive(false);
		return;
	}

	if (position.z > 800.0f || position.z < -800.0f) {
		SetisActive(false);
		return;
	}

	if (position.y > 500.0f || position.y < -200.0f) {
		SetisActive(false);
		return;
	}

	SetBulletRange(position.x, position.y, position.z);
	SetPosition(position.x, position.y, position.z);
}

void BulletObject::SetBulletRange(float x, float y, float z)
{
	Range_Pos temp = Convert_index(x, y, z);
	bullet_x_range = temp.x;
	bullet_y_range = temp.y;
	bullet_z_range = temp.z;
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
	type = TYPE_NORMAL;
}

Block::~Block()
{
	
}

void Block::Init_Block(int input_id, XMFLOAT3 input_pos, int input_type)
{
	SetId(input_id);
	SetPosition(input_pos.x, input_pos.y, input_pos.z);
	float rad = sqrt(144.f * 3.f) / 2.f;
	SetRadius(rad);
	SetisActive(true);
	type = input_type;
}


void Block::Move(float fTimeElapsed)
{
	XMFLOAT3 position = GetPosition();

	float speed = falling_speed * fTimeElapsed;

	position = { position.x, position.y - speed, position.z };

	/*if (position.y > 500.0f || position.y < -200.0f) {
		SetisActive(false);
		return;
	}*/

	SetBlockRange(position.x, position.y, position.z);
	SetPosition(position.x, position.y, position.z);
}

Range_Pos Block::Block_Convert_index(float x, float y, float z)
{
	Range_Pos temp;

	temp.x = ((x - 26) / 12) * (-1);
	temp.y = ((y + 108) / 12);
	temp.z = ((z - 46) / 12) * (-1);

	return temp;
}

void Block::SetBlockRange(float x, float y, float z)
{
	Range_Pos temp = Block_Convert_index(x, y, z);
	
	if (temp.y != block_y_range) {
		// 여기서 더 해야함
		// Map_B에 이전에 있던 건 지우고 새로운 곳을 활성화 시켜야 함
		
	}

	block_x_range = temp.x;
	block_y_range = temp.y;
	block_z_range = temp.z;
}