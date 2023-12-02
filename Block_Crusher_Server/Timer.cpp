#include "Timer.h"

Timer::Timer()
{
    m_TimeElapsed = 0.f;
    m_LastTime = std::chrono::system_clock::now();
    m_frameScale = 0.001f;

    m_FrameCount = 0;
    m_FPSTimeElapsed = 0.f;

    for (int i{}; i < MAX_FRAME_COUNT; ++i)
        m_FrameTime[i] = 0.f;
}

Timer::~Timer()
{
}

void Timer::Tick(float fLockFPS)
{
    // 함수가 시작된 시간 체크
    m_CurrentTime = std::chrono::system_clock::now();

    // 가장 마지막에 저장한 시간과 비교해 프레임 시간 구하기

    auto TimeElapsed = m_CurrentTime - m_LastTime;
    float TimeElapsed_f = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(TimeElapsed).count();

    float nowTimeElapsed = TimeElapsed_f * m_frameScale;

    // 현재 시간을 가장 마지막에 저장한 시간에 저장
    m_LastTime = m_CurrentTime;

    // 만약 현재 프레임 시간과 평균 프레임 처리 시간이 1초보다 작다면
    // 현재 프레임 처리 시간을 저장
    if (fabsf(nowTimeElapsed - m_TimeElapsed) < 1.f) {
        ::memmove(&m_FrameTime[1], m_FrameTime, (MAX_FRAME_COUNT - 1) * sizeof(float));
        m_FrameTime[0] = nowTimeElapsed;
        if (m_FrameCount < MAX_FRAME_COUNT) m_FrameCount++;
    }

    // 현재 프레임 타임을 1초 동안 담는 용기에 넣음
    m_FPSTimeElapsed += nowTimeElapsed;

    // 티끌 모아서 1초 달성
    if (m_FPSTimeElapsed > 1.f) 
        m_FPSTimeElapsed = 0.f;
    
    m_TimeElapsed = 0.f;
    for (int i{}; i < m_FrameCount; ++i) m_TimeElapsed += m_FrameTime[i];
    if (m_FrameCount > 0) m_TimeElapsed /= m_FrameCount;
}

float Timer::GetTimeElapsed()
{
    return m_TimeElapsed;
}
