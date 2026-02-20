#pragma once

#define LUMA_DEFINE_BITMASK_ENUM(enumType)                                                 \
    inline enumType operator&(enumType a, enumType b) noexcept                             \
    {                                                                                      \
        return static_cast<enumType>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b)); \
    }                                                                                      \
    inline enumType operator|(enumType a, enumType b) noexcept                             \
    {                                                                                      \
        return static_cast<enumType>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); \
    }                                                                                      \
    inline bool operator||(bool a, enumType b) noexcept                                    \
    {                                                                                      \
        return a || static_cast<uint64_t>(b) != 0;                                         \
    }                                                                                      \
    inline bool operator&&(bool a, enumType b) noexcept                                    \
    {                                                                                      \
        return a && static_cast<uint64_t>(b) != 0;                                         \
    }                                                                                      \
    inline bool operator!(enumType a) noexcept                                             \
    {                                                                                      \
        return static_cast<uint64_t>(a) == 0;                                              \
    }