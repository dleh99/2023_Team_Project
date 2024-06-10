#pragma once
#include "header.h"

constexpr int MAX_FRAME_COUNT = 60;

enum EVENT_TYPE { EV_RESPAWN, EV_RESTART };

class Timer
{
private:
	std::chrono::system_clock::time_point		m_CurrentTime;					// 현재 시간
	std::chrono::system_clock::time_point		m_LastTime;						// 최근 저장된 시간
	float		m_frameScale;													// 프레임 으로 시간을 짤 때, 스케일 할 양

	float		m_FPSTimeElapsed;												// 1초 동안 시간을 저장하는 변수
	float		m_FrameTime[MAX_FRAME_COUNT];
	int			m_FrameCount;

	float		m_TimeElapsed;													// 1초간 이뤄지는 평균 시간
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