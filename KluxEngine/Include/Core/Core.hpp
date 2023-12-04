#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_MACOS
#endif

#if defined(PLATFORM_WINDOWS)
// windows includes
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#elif defined(PLATFORM_LINUX)
// linux includes
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#endif



// std includes
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <limits>

// core includes
#include "core/Types.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include "core/EventManager.hpp"
