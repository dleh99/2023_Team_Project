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
    // �Լ��� ���۵� �ð� üũ
    m_CurrentTime = std::chrono::system_clock::now();

    // ���� �������� ������ �ð��� ���� ������ �ð� ���ϱ�

    auto TimeElapsed = m_CurrentTime - m_LastTime;
    float TimeElapsed_f = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(TimeElapsed).count();

    float nowTimeElapsed = TimeElapsed_f * m_frameScale;

    // ���� �ð��� ���� �������� ������ �ð��� ����
    m_LastTime = m_CurrentTime;

    // ���� ���� ������ �ð��� ��� ������ ó�� �ð��� 1�ʺ��� �۴ٸ�
    // ���� ������ ó�� �ð��� ����
    if (fabsf(nowTimeElapsed - m_TimeElapsed) < 1.f) {
        ::memmove(&m_FrameTime[1], m_FrameTime, (MAX_FRAME_COUNT - 1) * sizeof(float));
        m_FrameTime[0] = nowTimeElapsed;
        if (m_FrameCount < MAX_FRAME_COUNT) m_FrameCount++;
    }

    // ���� ������ Ÿ���� 1�� ���� ��� ��⿡ ����
    m_FPSTimeElapsed += nowTimeElapsed;

    // Ƽ�� ��Ƽ� 1�� �޼�
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
