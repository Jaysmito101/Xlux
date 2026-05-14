#pragma once

#include "Core/Types.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

namespace xlux {


template <typename T, U32 PointerCount>
struct XLUX_API alignas(std::hardware_destructive_interference_size) HazardThreadState {
  Array<Atomic<RawPtr<T>>, PointerCount> hazardPointers;
  List<RawPtr<T>> retiredPointers;
  List<RawPtr<T>> scratchBuffer;
};

template <typename T, U32 ThreadCount, U32 PointerCount, U32 RetireThreshold = 2 * ThreadCount * PointerCount>
requires (ThreadCount > 0)
class XLUX_API HazardPointer {
 public:
  HazardPointer() {
    for (auto& threadState : state) {
      for (auto& pointer : threadState.hazardPointers) {
        pointer.store(nullptr, std::memory_order::relaxed);
      }      
      threadState.retiredPointers.reserve(ThreadCount * PointerCount);
      threadState.retiredPointers.clear();

      threadState.scratchBuffer.reserve(ThreadCount * PointerCount * 4);
      threadState.scratchBuffer.clear();
    }
  }

  // NOTE: we assume that this is only freed once all the hazard pointers are 
  // released
  ~HazardPointer() {
    for (const auto& threadState : state) {
      assert(
        std::count_if(
          threadState.hazardPointers.begin(), threadState.hazardPointers.end(),
          [](const auto& item) {
            return item.load(std::memory_order::relaxed) != nullptr;
          }) == 0);


      for (const auto& ptr : threadState.retiredPointers) {
        if (ptr) {
          delete ptr;
        }
      }
    }

    // we could have this incase we wanted to ensure we dont have duplicates here
    // but ideally that shouldnt be the case anyways
    // std::sort(pointerTracker.begin(), pointerTracker.end());
    // auto last = std::unique(pointerTracker.begin(), pointerTracker.end());

    // for (auto it = pointerTracker.begin(); it < pointerTracker.end(); it++) {
    //   delete *it;
    // }    
  }

  RawPtr<T> Protect(const Atomic<RawPtr<T>>& ptr, U32 threadId, U32 slotId) {
    assert(threadId < ThreadCount);
    assert(slotId < PointerCount);

    RawPtr<T> p = nullptr;
    do {
      p = ptr.load(std::memory_order::acquire);
      state[threadId].hazardPointers[slotId].store(p, std::memory_order::seq_cst);
    } while (p && p != ptr.load(std::memory_order::seq_cst));
    return p;
  }

  void Release(U32 threadId, U32 slotId) {
    assert(threadId < ThreadCount);
    assert(slotId < PointerCount);

    state[threadId].hazardPointers[slotId].store(
        nullptr, std::memory_order::release);
  }

  void Clear(U32 threadId) {
    assert(threadId < ThreadCount);

    for (U32 slotId = 0; slotId < PointerCount; ++slotId) {
      Release(threadId, slotId);
    }
  }

  void Retire(RawPtr<T> ptr, U32 threadId) {
    assert(threadId < ThreadCount);

    state[threadId].retiredPointers.push_back(ptr);
    if (state[threadId].retiredPointers.size() >= RetireThreshold) {
      Scan(threadId);
    }
  }

 private:
  void Scan(U32 threadId) {
    auto& activePointersTracker = state[threadId].scratchBuffer;
    activePointersTracker.clear();

    
    for (const auto& list : state) {
      for (const auto& pointer : list.hazardPointers) {
        RawPtr<T> ptr = pointer.load(std::memory_order::seq_cst);
        if (ptr) {
          activePointersTracker.push_back(ptr);
        }
      }
    }

    std::sort(activePointersTracker.begin(), activePointersTracker.end());

    auto& retiredList = state[threadId].retiredPointers;
    for (U32 i = 0 ; i < retiredList.size() ;) {
      if (!std::binary_search(activePointersTracker.begin(), activePointersTracker.end(), retiredList[i])) {
        delete retiredList[i];
        retiredList[i] = retiredList.back();
        retiredList.pop_back();
      } else {
        i++;
      }
    }
  }

 private:
  Array<HazardThreadState<T, PointerCount>, ThreadCount> state;
};
}  // namespace xlux