#pragma once
#include "header.h"

constexpr int TYPE_NORMAL = 1;
//constexpr int TYPE_AIR = 2;

class GameObject
{
private:
	XMFLOAT3	pos;
	bool		isActive;
	int id;
	float ObjectBoundingRadius;
public:
	GameObject();
	~GameObject();

	bool GetisActive() { return isActive; };
	void SetisActive(bool ac) { isActive = ac; };

	void SetId(int input_id) { id = input_id; };
	int GetId() { return id; };

	void SetRadius(float rad) { ObjectBoundingRadius = rad; };
	float GetRadius() { return ObjectBoundingRadius; };

	Range_Pos Convert_index(float x, float y, float z);

	XMFLOAT3 GetPosition();
	void SetPosition(float x, float y, float z);
};

class Block : public GameObject
{
private:
	int		type;
	float	falling_speed = 5.f;

	int block_x_range;
	int block_y_range;
	int block_z_range;
public:
	Block();
	Block(int input_id, XMFLOAT3 input_pos);
	~Block();

	int GetBlockType() { return type; };
	void Init_Block(int input_id, XMFLOAT3 input_pos, int input_type);

	void Move(float fTimeElapsed);

	Range_Pos Block_Convert_index(float x, float y, float z);
	void SetBlockRange(float x, float y, float z);
};

class BulletObject : public GameObject
{
private:
	XMFLOAT3 bullet_vec;

	float bullet_speed;

	int bullet_id;

	int bullet_x_range;
	int bullet_y_range;
	int bullet_z_range;
public:
	BulletObject();
	~BulletObject();

	XMFLOAT3 GetBulletVec() { return bullet_vec; };
	void SetBulletVec(float x, float y, float z);

	void Move(float fTimeElapsed);

	void SetBulletId(int x) { bullet_id = x; };
	int GetbulletId() { return bullet_id; };
	void SetBulletRange(float x, float y, float z);
	Range_Pos GetBulletRange() { return Range_Pos{ bullet_x_range, bullet_y_range, bullet_z_range }; };
};