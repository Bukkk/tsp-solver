#pragma once

#include <cstdint>
#include <endian.h>
#include <memory>

namespace ds {

/**
 * @brief macierz na heapie
 * zoptymalizowana pod akcesowanie od lewej do prawej
 *
 * @tparam T
 * @tparam Size
 */
template <typename ValueType>
class heap_matrix {
    ValueType* mem_ {};
    uint64_t size_ {};

public:
    heap_matrix() = default;

    heap_matrix(uint64_t size)
        : size_{size}
    {
        mem_ = new ValueType[size_ * size_] {};
    }

    ~heap_matrix()
    {
        if (mem_) {
            delete [] mem_;
        }
    }

    heap_matrix(const heap_matrix& other)
        : size_ { other.size_ }
    {
        mem_ = new ValueType[size_ * size_] {};

        for (uint64_t y = 0; y < size_; ++y) {
            for (uint64_t x = 0; x < size_; ++x) {
                at(x, y) = other.at(x, y);
            }
        }
    }

    heap_matrix& operator=(const heap_matrix& other)
    {
        if (this == &other) {
            return *this;
        }

        if (mem_) {
            delete [] mem_;
        }

        mem_ = new ValueType[other.size_ * other.size_] {};
        size_ = other.size_;

        for (uint64_t y = 0; y < size_; ++y) {
            for (uint64_t x = 0; x < size_; ++x) {
                at(x, y) = other.at(x, y);
            }
        }

        return *this;
    }

    heap_matrix(heap_matrix&& dying)
        : mem_{dying.mem_}
        , size_{dying.size_}
    {
        dying.mem_ = nullptr;
    }

    heap_matrix& operator=(heap_matrix&& dying)
    {
        if (this == &dying) {
            return *this;
        }

        if (mem_) {
            delete [] mem_;
        }

        mem_ = dying.mem_;
        size_ = dying.size_;
        
        dying.mem_ = nullptr;

        return *this;
    }

    constexpr auto size() const -> uint64_t
    {
        return size_;
    }

    /**
     * @brief referencja miejsca w macierzy x rosnie w prawo y w dol
     *
     * @param x
     * @param y
     * @return T&
     */
    auto at(uint64_t x, uint64_t y) const -> ValueType&
    {
        return mem_[y * size_ + x];
    }
};
}