#pragma once
#include "header.h"

struct XMFLOAT3 {
	float x;
	float y;
	float z;
};

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
public:
	Block();
	~Block();
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