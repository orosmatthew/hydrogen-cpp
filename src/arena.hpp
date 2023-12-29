#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>

class ArenaAllocator final {
public:
    explicit ArenaAllocator(const std::size_t max_num_bytes)
        : m_size { max_num_bytes }
        , m_buffer { new std::byte[max_num_bytes] }
        , m_offset { m_buffer }
    {
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    ArenaAllocator(ArenaAllocator&& other) noexcept
        : m_size { std::exchange(other.m_size, 0) }
        , m_buffer { std::exchange(other.m_buffer, nullptr) }
        , m_offset { std::exchange(other.m_offset, nullptr) }
    {
    }

    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept
    {
        std::swap(m_size, other.m_size);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_offset, other.m_offset);
        return *this;
    }

    template <typename T>
    [[nodiscard]] T* alloc()
    {
        std::size_t remaining_num_bytes = m_size - static_cast<std::size_t>(m_offset - m_buffer);
        auto pointer = static_cast<void*>(m_offset);
        const auto aligned_address = std::align(alignof(T), sizeof(T), pointer, remaining_num_bytes);
        if (aligned_address == nullptr) {
            throw std::bad_alloc {};
        }
        m_offset = static_cast<std::byte*>(aligned_address) + sizeof(T);
        return static_cast<T*>(aligned_address);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        const auto allocated_memory = alloc<T>();
        return new (allocated_memory) T { std::forward<Args>(args)... };
    }

    ~ArenaAllocator()
    {
        // No destructors are called for the stored objects. Thus, memory
        // leaks are possible (e.g. when storing std::vector objects or
        // other non-trivially destructable objects in the allocator).
        // Although this could be changed, it would come with additional
        // runtime overhead and therefore is not implemented.
        delete[] m_buffer;
    }

private:
    std::size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};