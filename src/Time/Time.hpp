#pragma once

namespace PicoEngine
{

class Time
{
  public:
    Time();
    ~Time() = default;

    Time(const Time&)            = delete;
    Time& operator=(const Time&) = delete;
    Time(Time&&)                 = delete;
    Time& operator=(Time&&)      = delete;

    void Update();
    void Reset();

    float GetDeltaTime() const noexcept
    {
        return m_deltaTime;
    }

    float GetUnscaledDeltaTime() const noexcept
    {
        return m_unscaledDeltaTime;
    }

    double GetTime() const noexcept
    {
        return m_time;
    }

    double GetUnscaledTime() const noexcept
    {
        return m_unscaledTime;
    }

    uint64_t GetFrameCount() const noexcept
    {
        return m_frameCount;
    }

    float GetTimeScale() const noexcept
    {
        return m_timeScale;
    }

    void SetTimeScale(float scale);

    float GetMaxDeltaTime() const noexcept
    {
        return m_maxDeltaTime;
    }

    void SetMaxDeltaTime(float maxDelta);

  private:
    std::chrono::steady_clock::time_point m_lastTick;
    float                                 m_deltaTime         = 0.0f;
    float                                 m_unscaledDeltaTime = 0.0f;
    double                                m_time              = 0.0;
    double                                m_unscaledTime      = 0.0;
    uint64_t                              m_frameCount        = 0;
    float                                 m_timeScale         = 1.0f;
    float                                 m_maxDeltaTime      = 0.1f;
    bool                                  m_firstUpdate       = true;
};

} // namespace PicoEngine
