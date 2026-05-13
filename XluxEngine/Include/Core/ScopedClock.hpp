#pragma once

#include <chrono>

#include "Core/Logger.hpp"
#include "Core/Types.hpp"

namespace  xlux {
    class XLUX_API ScopedClock {
    public:
        ScopedClock(const String& name) : m_Name(name), m_StartTime(std::chrono::high_resolution_clock::now()) {}

        ~ScopedClock() {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::milli>(endTime - m_StartTime).count();
            log::Info("{} Popped: {} ms", m_Name, duration);
        }

        XLUX_FORCE_INLINE void Reset() {
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        XLUX_FORCE_INLINE float Elapsed() const {
            auto endTime = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<float, std::milli>(endTime - m_StartTime).count();
        }

        XLUX_FORCE_INLINE void Checkpoint(const String& checkpointName) const {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::milli>(endTime - m_StartTime).count();
            log::Info("{} Checkpoint '{}': {} ms", m_Name, checkpointName, duration);
        }

    private:
        String m_Name;
        std::chrono::high_resolution_clock::time_point m_StartTime;
    };
}