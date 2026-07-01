#include "AllocationCounter.h"

#include <cstdlib>
#include <new>

namespace VSTFactory::Test
{

std::atomic<std::size_t> g_allocationCount { 0 };
thread_local bool g_countingEnabled = false;

void resetAllocationCount() noexcept
{
    g_allocationCount.store (0, std::memory_order_relaxed);
}

std::size_t getAllocationCount() noexcept
{
    return g_allocationCount.load (std::memory_order_relaxed);
}

void setCountingEnabled (bool enabled) noexcept
{
    g_countingEnabled = enabled;
}

} // namespace VSTFactory::Test

void* operator new (std::size_t size)
{
    if (VSTFactory::Test::g_countingEnabled)
        VSTFactory::Test::g_allocationCount.fetch_add (1, std::memory_order_relaxed);

    if (void* ptr = std::malloc (size))
        return ptr;

    throw std::bad_alloc();
}

void* operator new[] (std::size_t size)
{
    return operator new (size);
}

void operator delete (void* ptr) noexcept
{
    std::free (ptr);
}

void operator delete[] (void* ptr) noexcept
{
    operator delete (ptr);
}

void operator delete (void* ptr, std::size_t) noexcept
{
    operator delete (ptr);
}

void operator delete[] (void* ptr, std::size_t) noexcept
{
    operator delete[] (ptr);
}
