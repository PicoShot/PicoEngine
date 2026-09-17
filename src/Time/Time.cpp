#include "Time/Time.hpp"

#include "Debug/Debug.hpp"

namespace PicoEngine
{

Time::Time() : m_lastTick(std::chrono::steady_clock::now())
{
}

void Time::Update()
{
    const auto now        = std::chrono::steady_clock::now();
    double     rawSeconds = std::chrono::duration<double>(now - m_lastTick).count();
    m_lastTick            = now;

    if (m_firstUpdate)
    {
        rawSeconds    = 0.0;
        m_firstUpdate = false;
    }
    if (rawSeconds < 0.0)
        rawSeconds = 0.0;

    const double clamped = std::min(rawSeconds, static_cast<double>(m_maxDeltaTime));
    m_unscaledDeltaTime  = static_cast<float>(clamped);
    m_deltaTime          = static_cast<float>(clamped * static_cast<double>(m_timeScale));
    m_unscaledTime += clamped;
    m_time += clamped * static_cast<double>(m_timeScale);
    ++m_frameCount;
}

void Time::Reset()
{
    m_lastTick          = std::chrono::steady_clock::now();
    m_deltaTime         = 0.0f;
    m_unscaledDeltaTime = 0.0f;
    m_time              = 0.0;
    m_unscaledTime      = 0.0;
    m_frameCount        = 0;
    m_firstUpdate       = true;
    LOG_DEBUG("Time reset");
}

void Time::SetTimeScale(float scale)
{
    PICO_ASSERT(scale >= 0.0f, "Time scale must be non-negative (got {})", scale);
    m_timeScale = scale;
}

void Time::SetMaxDeltaTime(float maxDelta)
{
    PICO_ASSERT(maxDelta > 0.0f, "Max delta time must be positive (got {})", maxDelta);
    m_maxDeltaTime = maxDelta;
}

} // namespace PicoEngine
