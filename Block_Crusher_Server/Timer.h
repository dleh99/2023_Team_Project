#pragma once
#include "header.h"

constexpr int MAX_FRAME_COUNT = 60;

enum EVENT_TYPE { EV_RESPAWN, EV_RESTART };

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

class TIMER_EVENT {
public:
	int obj_id;
	std::chrono::system_clock::time_point		wakeup_time;
	EVENT_TYPE event_id;
	int target_id;
public:
	TIMER_EVENT();
	TIMER_EVENT(int ob_id, std::chrono::system_clock::time_point time, EVENT_TYPE et, int t_id);
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};