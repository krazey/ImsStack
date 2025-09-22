/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AOS_ENUM_OPERATORS_
#define AOS_ENUM_OPERATORS_

#include <type_traits>

/**
 * @brief Enables ALL standard bitwise operators for an enum class.
 *
 * This macro defines bitwise operators (&, |, ^, ~, <<, >>, etc.) for a given
 * enum class. It ensures type safety by returning an instance of the enum class
 * for enum-to-enum operations, allowing for chaining.
 *
 * IMPORTANT: The EnumType MUST have an unsigned underlying type
 *            (e.g., `enum class MyFlags : unsigned int`).
 * Using a signed underlying type may lead to undefined or implementation-defined behavior
 * for bit shift operations (<<, >>), especially with negative values.
 * A compile-time assertion (`static_assert`) is included to enforce this requirement.
 */
#define DEFINE_ENUM_BITMASK_OPERATORS(EnumType)                                                    \
                                                                                                   \
    /* --- Compile-time check: Ensure underlying type is unsigned --- */                           \
    static_assert(std::is_unsigned_v<std::underlying_type_t<EnumType>>,                            \
            "EnumType used with DEFINE_ENUM_BITMASK_OPERATORS must have an "                       \
            "unsigned underlying type");                                                           \
                                                                                                   \
    /* --- Bitwise OR (|) --- */                                                                   \
    inline constexpr EnumType operator|(EnumType lhs, EnumType rhs)                                \
    {                                                                                              \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) |          \
                static_cast<std::underlying_type_t<EnumType>>(rhs));                               \
    }                                                                                              \
    inline EnumType& operator|=(EnumType& lhs, EnumType rhs)                                       \
    {                                                                                              \
        lhs = lhs | rhs;                                                                           \
        return lhs;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* --- Bitwise AND (&) --- */                                                                  \
    inline constexpr EnumType operator&(EnumType lhs, EnumType rhs)                                \
    {                                                                                              \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) &          \
                static_cast<std::underlying_type_t<EnumType>>(rhs));                               \
    }                                                                                              \
    inline EnumType& operator&=(EnumType& lhs, EnumType rhs)                                       \
    {                                                                                              \
        lhs = lhs & rhs;                                                                           \
        return lhs;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* --- Bitwise XOR (^) --- */                                                                  \
    inline constexpr EnumType operator^(EnumType lhs, EnumType rhs)                                \
    {                                                                                              \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) ^          \
                static_cast<std::underlying_type_t<EnumType>>(rhs));                               \
    }                                                                                              \
    inline EnumType& operator^=(EnumType& lhs, EnumType rhs)                                       \
    {                                                                                              \
        lhs = lhs ^ rhs;                                                                           \
        return lhs;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* --- Bitwise NOT (~) --- */                                                                  \
    inline constexpr EnumType operator~(EnumType e)                                                \
    {                                                                                              \
        return static_cast<EnumType>(~static_cast<std::underlying_type_t<EnumType>>(e));           \
    }                                                                                              \
                                                                                                   \
    /* --- Bitwise Shift Left (<<) --- */                                                          \
    inline constexpr EnumType operator<<(EnumType lhs, int shift)                                  \
    {                                                                                              \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) << shift); \
    }                                                                                              \
    inline EnumType& operator<<=(EnumType& lhs, int shift)                                         \
    {                                                                                              \
        lhs = lhs << shift;                                                                        \
        return lhs;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* --- Bitwise Shift Right (>>) --- */                                                         \
    inline constexpr EnumType operator>>(EnumType lhs, int shift)                                  \
    {                                                                                              \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) >> shift); \
    }                                                                                              \
    inline EnumType& operator>>=(EnumType& lhs, int shift)                                         \
    {                                                                                              \
        lhs = lhs >> shift;                                                                        \
        return lhs;                                                                                \
    }

/**
 * @brief Enables ALL standard bitwise operators for an enum class,
 * including mixed-mode operations with its underlying integer type.
 *
 * This macro includes all operators from DEFINE_ENUM_BITMASK_OPERATORS
 * and adds mixed-mode (Integer <op> Enum) operations.
 *
 * IMPORTANT: Similar to DEFINE_ENUM_BITMASK_OPERATORS, the EnumType
 * MUST have an unsigned underlying type. This is enforced by the
 * included base macro's `static_assert`.
 */
#define DEFINE_ENUM_BITMASK_OPERATORS_EX(EnumType)                                       \
    /* 1. Include all the base (Enum-centric) operators first. */                        \
    DEFINE_ENUM_BITMASK_OPERATORS(EnumType)                                              \
                                                                                         \
    /* --- Mixed-Mode Bitwise OR (|) Operations (Return Underlying Integer Type) --- */  \
    inline constexpr std::underlying_type_t<EnumType> operator|(                         \
            std::underlying_type_t<EnumType> lhs, EnumType rhs)                          \
    {                                                                                    \
        return lhs | static_cast<std::underlying_type_t<EnumType>>(rhs);                 \
    }                                                                                    \
    inline constexpr std::underlying_type_t<EnumType> operator|(                         \
            EnumType lhs, std::underlying_type_t<EnumType> rhs)                          \
    {                                                                                    \
        return static_cast<std::underlying_type_t<EnumType>>(lhs) | rhs;                 \
    }                                                                                    \
    inline std::underlying_type_t<EnumType>& operator|=(                                 \
            std::underlying_type_t<EnumType>& lhs, EnumType rhs)                         \
    {                                                                                    \
        lhs = lhs | rhs;                                                                 \
        return lhs;                                                                      \
    }                                                                                    \
                                                                                         \
    /* --- Mixed-Mode Bitwise AND (&) Operations (Return Underlying Integer Type) --- */ \
    inline constexpr std::underlying_type_t<EnumType> operator&(                         \
            std::underlying_type_t<EnumType> lhs, EnumType rhs)                          \
    {                                                                                    \
        return lhs & static_cast<std::underlying_type_t<EnumType>>(rhs);                 \
    }                                                                                    \
    inline constexpr std::underlying_type_t<EnumType> operator&(                         \
            EnumType lhs, std::underlying_type_t<EnumType> rhs)                          \
    {                                                                                    \
        return static_cast<std::underlying_type_t<EnumType>>(lhs) & rhs;                 \
    }                                                                                    \
    inline std::underlying_type_t<EnumType>& operator&=(                                 \
            std::underlying_type_t<EnumType>& lhs, EnumType rhs)                         \
    {                                                                                    \
        lhs = lhs & rhs;                                                                 \
        return lhs;                                                                      \
    }                                                                                    \
                                                                                         \
    /* --- Mixed-Mode Bitwise XOR (^) Operations (Return Underlying Integer Type) --- */ \
    inline constexpr std::underlying_type_t<EnumType> operator^(                         \
            std::underlying_type_t<EnumType> lhs, EnumType rhs)                          \
    {                                                                                    \
        return lhs ^ static_cast<std::underlying_type_t<EnumType>>(rhs);                 \
    }                                                                                    \
    inline constexpr std::underlying_type_t<EnumType> operator^(                         \
            EnumType lhs, std::underlying_type_t<EnumType> rhs)                          \
    {                                                                                    \
        return static_cast<std::underlying_type_t<EnumType>>(lhs) ^ rhs;                 \
    }                                                                                    \
    inline std::underlying_type_t<EnumType>& operator^=(                                 \
            std::underlying_type_t<EnumType>& lhs, EnumType rhs)                         \
    {                                                                                    \
        lhs = lhs ^ rhs;                                                                 \
        return lhs;                                                                      \
    }

#endif  // AOS_ENUM_OPERATORS_
