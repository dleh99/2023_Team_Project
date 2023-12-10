#pragma once
#include "header.h"

constexpr int MAX_FRAME_COUNT = 60;

class Timer
{
private:
	std::chrono::system_clock::time_point		m_CurrentTime;					// ���� �ð�
	std::chrono::system_clock::time_point		m_LastTime;						// �ֱ� ����� �ð�
	float		m_frameScale;													// ������ ���� �ð��� © ��, ������ �� ��

	float		m_FPSTimeElapsed;												// 1�� ���� �ð��� �����ϴ� ����
	float		m_FrameTime[MAX_FRAME_COUNT];
	int			m_FrameCount;

	float		m_TimeElapsed;													// 1�ʰ� �̷����� ��� �ð�
public:
	Timer();
	~Timer();
	void Tick(float fLockFPS);
	float GetTimeElapsed();
};