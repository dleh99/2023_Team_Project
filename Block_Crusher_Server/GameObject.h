#pragma once

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
public:
	GameObject();
	~GameObject();

	bool GetisActive() { return isActive; };
	void SetisActive(bool ac) { isActive = ac; };

	XMFLOAT3 GetPosition();
	void SetPosition(float x, float y, float z);
};

class Block : public GameObject
{

};

class BulletObject : public GameObject
{
private:
	XMFLOAT3 bullet_vec;

	float bullet_speed;
public:
	BulletObject();
	~BulletObject();

	XMFLOAT3 GetBulletVec();
	void SetBulletVec(float x, float y, float z);

	void Move(float fTimeElapsed);
};