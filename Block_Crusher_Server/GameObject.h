#pragma once
#include "header.h"

constexpr int TYPE_NORMAL = 1;

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

	XMFLOAT3 GetPosition();
	void SetPosition(float x, float y, float z);
};

class Block : public GameObject
{
private:
	int type;
public:
	Block();
	Block(int input_id, XMFLOAT3 input_pos);
	~Block();

	int GetBlockType() { return type; };
};

class BulletObject : public GameObject
{
private:
	XMFLOAT3 bullet_vec;

	float bullet_speed;

	int bullet_id;
public:
	BulletObject();
	~BulletObject();

	XMFLOAT3 GetBulletVec();
	void SetBulletVec(float x, float y, float z);

	void Move(float fTimeElapsed);

	void SetBulletId(int x) { bullet_id = x; };
	int GetbulletId() { return bullet_id; };
};