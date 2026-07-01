#pragma once

#include <atomic>
#include <cstddef>

namespace VSTFactory::Test
{

extern std::atomic<std::size_t> g_allocationCount;
extern thread_local bool g_countingEnabled;

void resetAllocationCount() noexcept;
std::size_t getAllocationCount() noexcept;
void setCountingEnabled (bool enabled) noexcept;

} // namespace VSTFactory::Test
