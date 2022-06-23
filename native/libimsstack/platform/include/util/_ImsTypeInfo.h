/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             From Android
    </table>

    Description

*/

#ifndef _IMS_TYPE_INFO_H_
#define _IMS_TYPE_INFO_H_

#include "ImsNew.h"

//
//    Types traits
//
template <typename T>
struct TRAIT_TRIVIAL_CTOR
{
    enum
    {
        VALUE = IMS_FALSE
    };
};

template <typename T>
struct TRAIT_TRIVIAL_DTOR
{
    enum
    {
        VALUE = IMS_FALSE
    };
};

template <typename T>
struct TRAIT_TRIVIAL_COPY
{
    enum
    {
        VALUE = IMS_FALSE
    };
};

template <typename T>
struct TRAIT_TRIVIAL_ASSIGN
{
    enum
    {
        VALUE = IMS_FALSE
    };
};

template <typename T>
struct TRAIT_POINTER
{
    enum
    {
        VALUE = IMS_FALSE
    };
};

template <typename T>
struct TRAIT_POINTER<T*>
{
    enum
    {
        VALUE = IMS_TRUE
    };
};

#define IMS_BASIC_TYPES_TRAITS(T)  \
    template <>                    \
    struct TRAIT_TRIVIAL_CTOR<T>   \
    {                              \
        enum                       \
        {                          \
            VALUE = IMS_TRUE       \
        };                         \
    };                             \
    template <>                    \
    struct TRAIT_TRIVIAL_DTOR<T>   \
    {                              \
        enum                       \
        {                          \
            VALUE = IMS_TRUE       \
        };                         \
    };                             \
    template <>                    \
    struct TRAIT_TRIVIAL_COPY<T>   \
    {                              \
        enum                       \
        {                          \
            VALUE = IMS_TRUE       \
        };                         \
    };                             \
    template <>                    \
    struct TRAIT_TRIVIAL_ASSIGN<T> \
    {                              \
        enum                       \
        {                          \
            VALUE = IMS_TRUE       \
        };                         \
    };

#define IMS_TYPE_TRAITS(T, CTOR, DTOR, COPY, ASSIGN) \
    template <>                                      \
    struct TRAIT_TRIVIAL_CTOR<T>                     \
    {                                                \
        enum                                         \
        {                                            \
            VALUE = CTOR                             \
        };                                           \
    };                                               \
    template <>                                      \
    struct TRAIT_TRIVIAL_DTOR<T>                     \
    {                                                \
        enum                                         \
        {                                            \
            VALUE = DTOR                             \
        };                                           \
    };                                               \
    template <>                                      \
    struct TRAIT_TRIVIAL_COPY<T>                     \
    {                                                \
        enum                                         \
        {                                            \
            VALUE = COPY                             \
        };                                           \
    };                                               \
    template <>                                      \
    struct TRAIT_TRIVIAL_ASSIGN<T>                   \
    {                                                \
        enum                                         \
        {                                            \
            VALUE = ASSIGN                           \
        };                                           \
    };

template <typename T>
struct TRAITS
{
    enum
    {
        IS_POINTER = TRAIT_POINTER<T>::VALUE,
        HAS_TRIVIAL_CTOR = (IS_POINTER || TRAIT_TRIVIAL_CTOR<T>::VALUE),
        HAS_TRIVIAL_DTOR = (IS_POINTER || TRAIT_TRIVIAL_DTOR<T>::VALUE),
        HAS_TRIVIAL_COPY = (IS_POINTER || TRAIT_TRIVIAL_COPY<T>::VALUE),
        HAS_TRIVIAL_ASSIGN = (IS_POINTER || TRAIT_TRIVIAL_ASSIGN<T>::VALUE)
    };
};

template <typename T, typename U>
struct AGGREGATE_TRAITS
{
    enum
    {
        IS_POINTER = IMS_FALSE,
        HAS_TRIVIAL_CTOR = (TRAITS<T>::HAS_TRIVIAL_CTOR && TRAITS<U>::HAS_TRIVIAL_CTOR),
        HAS_TRIVIAL_DTOR = (TRAITS<T>::HAS_TRIVIAL_DTOR && TRAITS<U>::HAS_TRIVIAL_DTOR),
        HAS_TRIVIAL_COPY = (TRAITS<T>::HAS_TRIVIAL_COPY && TRAITS<U>::HAS_TRIVIAL_COPY),
        HAS_TRIVIAL_ASSIGN = (TRAITS<T>::HAS_TRIVIAL_ASSIGN && TRAITS<U>::HAS_TRIVIAL_ASSIGN)
    };
};

//-------------------------------------------------------------------------------------------------

//
// Basic types traits
//
IMS_BASIC_TYPES_TRAITS(void);
IMS_BASIC_TYPES_TRAITS(IMS_BOOL);
IMS_BASIC_TYPES_TRAITS(IMS_SINT8);
IMS_BASIC_TYPES_TRAITS(IMS_UINT8);
IMS_BASIC_TYPES_TRAITS(IMS_SINT16);
IMS_BASIC_TYPES_TRAITS(IMS_UINT16);
IMS_BASIC_TYPES_TRAITS(IMS_SINT32);
IMS_BASIC_TYPES_TRAITS(IMS_UINT32);
IMS_BASIC_TYPES_TRAITS(IMS_SLONG);
IMS_BASIC_TYPES_TRAITS(IMS_ULONG);
// IMS_BASIC_TYPES_TRAITS(IMS_CHAR);
// IMS_BASIC_TYPES_TRAITS(IMS_UCHAR);
// IMS_BASIC_TYPES_TRAITS(IMS_WCHAR);
// IMS_BASIC_TYPES_TRAITS(IMS_BYTE);
// IMS_BASIC_TYPES_TRAITS(IMS_SINT64);
// IMS_BASIC_TYPES_TRAITS(IMS_UINT64);
IMS_BASIC_TYPES_TRAITS(IMS_FLOAT);
IMS_BASIC_TYPES_TRAITS(IMS_DOUBLE);

//-------------------------------------------------------------------------------------------------

//
// Compare and order types
//

template <typename T>
inline IMS_SINT32 STRICTLY_ORDER_TYPE(const T& lhs, const T& rhs)
{
    return (lhs < rhs) ? 1 : 0;
}

template <typename T>
inline IMS_SINT32 COMPARE_TYPE(const T& lhs, const T& rhs)
{
    return STRICTLY_ORDER_TYPE(rhs, lhs) - STRICTLY_ORDER_TYPE(lhs, rhs);
}

//
// Create, Destroy, Copy and Assign types ...
//

template <typename T>
inline void CONSTRUCT_TYPE(T* p, IMS_SIZE_T n)
{
    if (!TRAITS<T>::HAS_TRIVIAL_CTOR)
    {
        while (n--)
        {
            new (p++) T;
        }
    }
}

template <typename T>
inline void DESTRUCT_TYPE(T* p, IMS_SIZE_T n)
{
    if (!TRAITS<T>::HAS_TRIVIAL_DTOR)
    {
        while (n--)
        {
            p->~T();
            p++;
        }
    }
}

template <typename T>
inline void COPY_TYPE(T* d, const T* s, IMS_SIZE_T n)
{
    if (!TRAITS<T>::HAS_TRIVIAL_COPY)
    {
        while (n--)
        {
            new (d) T(*s);
            d++, s++;
        }
    }
    else
    {
        IMS_MEM_Memcpy(d, s, n * sizeof(T));
    }
}

template <typename T>
inline void ASSIGN_TYPE(T* d, const T* s, IMS_SIZE_T n)
{
    if (!TRAITS<T>::HAS_TRIVIAL_ASSIGN)
    {
        while (n--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        IMS_MEM_Memcpy(d, s, n * sizeof(T));
    }
}

template <typename T>
inline void SET_TYPE(T* d, const T* what, IMS_SIZE_T n)
{
    if (!TRAITS<T>::HAS_TRIVIAL_COPY)
    {
        while (n--)
        {
            new (d) T(*what);
            d++;
        }
    }
    else
    {
        while (n--)
        {
            *d++ = *what;
        }
    }
}

template <typename T>
inline void MOVE_FORWARD_TYPE(T* d, const T* s, IMS_SIZE_T n = 1)
{
    if (!TRAITS<T>::HAS_TRIVIAL_COPY || !TRAITS<T>::HAS_TRIVIAL_DTOR)
    {
        d += n;
        s += n;
        while (n--)
        {
            --d, --s;
            if (!TRAITS<T>::HAS_TRIVIAL_COPY)
            {
                new (d) T(*s);
            }
            else
            {
                *d = *s;
            }
            if (!TRAITS<T>::HAS_TRIVIAL_DTOR)
            {
                s->~T();
            }
        }
    }
    else
    {
        IMS_MEM_Memmove(d, s, n * sizeof(T));
    }
}

template <typename T>
inline void MOVE_BACKWARD_TYPE(T* d, const T* s, IMS_SIZE_T n = 1)
{
    if (!TRAITS<T>::HAS_TRIVIAL_COPY || !TRAITS<T>::HAS_TRIVIAL_DTOR)
    {
        while (n--)
        {
            if (!TRAITS<T>::HAS_TRIVIAL_COPY)
            {
                new (d) T(*s);
            }
            else
            {
                *d = *s;
            }
            if (!TRAITS<T>::HAS_TRIVIAL_DTOR)
            {
                s->~T();
            }
            d++, s++;
        }
    }
    else
    {
        IMS_MEM_Memmove(d, s, n * sizeof(T));
    }
}

//-------------------------------------------------------------------------------------------------

//
// A key - value pair
//

template <typename KEY, typename VALUE>
struct KEY_VALUE_PAIR_T
{
    KEY key;
    VALUE value;

    KEY_VALUE_PAIR_T() {}
    KEY_VALUE_PAIR_T(const KEY_VALUE_PAIR_T& rhs) :
            key(rhs.key),
            value(rhs.value)
    {
    }
    KEY_VALUE_PAIR_T(const KEY& k, const VALUE& v) :
            key(k),
            value(v)
    {
    }
    KEY_VALUE_PAIR_T(const KEY& k) :
            key(k)
    {
    }
    inline IMS_BOOL operator<(const KEY_VALUE_PAIR_T& rhs) const
    {
        return (STRICTLY_ORDER_TYPE(key, rhs.key) == 1);
    }
};

// template<>
template <typename K, typename V>
struct TRAIT_TRIVIAL_CTOR<KEY_VALUE_PAIR_T<K, V>>
{
    enum
    {
        VALUE = AGGREGATE_TRAITS<K, V>::HAS_TRIVIAL_CTOR
    };
};

// template<>
template <typename K, typename V>
struct TRAIT_TRIVIAL_DTOR<KEY_VALUE_PAIR_T<K, V>>
{
    enum
    {
        VALUE = AGGREGATE_TRAITS<K, V>::HAS_TRIVIAL_DTOR
    };
};

// template<>
template <typename K, typename V>
struct TRAIT_TRIVIAL_COPY<KEY_VALUE_PAIR_T<K, V>>
{
    enum
    {
        VALUE = AGGREGATE_TRAITS<K, V>::HAS_TRIVIAL_COPY
    };
};

// template<>
template <typename K, typename V>
struct TRAIT_TRIVIAL_ASSIGN<KEY_VALUE_PAIR_T<K, V>>
{
    enum
    {
        VALUE = AGGREGATE_TRAITS<K, V>::HAS_TRIVIAL_ASSIGN
    };
};

#endif  // _IMS_TYPE_INFO_H_
