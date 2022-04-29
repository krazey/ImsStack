#ifndef IMSWMS_WMS_LITE_TYPE_DEF_H_
#define IMSWMS_WMS_LITE_TYPE_DEF_H_



#ifndef IMSWMS_NULL
#define IMSWMS_NULL                (0)
#endif

#ifndef IMSWMS_TRUE
#define IMSWMS_TRUE                ((1) == (1))
#endif

#ifndef IMSWMS_FALSE
#define IMSWMS_FALSE               ((1) == (0))
#endif

#ifndef IMSWMS_SUCCESS
#define IMSWMS_SUCCESS             (0)
#endif

#ifndef IMSWMS_FAILURE
#define IMSWMS_FAILURE             (-1)
#endif

#ifndef null
#define null                    (0)
#endif

#ifndef GLOBAL
#define GLOBAL
#endif

#ifndef LOCAL
#define LOCAL                   static
#endif

#ifndef PROTECTED
#define PROTECTED
#endif

#ifndef PUBLIC
#define PUBLIC
#endif

#ifndef PRIVATE
#define PRIVATE
#endif

#ifndef ABSTRACT
#define ABSTRACT
#endif

#ifndef VIRTUAL
#define VIRTUAL
#endif

//Input argument in function declaration
#ifndef IN
#define IN
#endif

// Output argument in function declaration
#ifndef OUT
#define OUT
#endif

//Input & Output argument in function declaration
#ifndef IN_OUT
#define IN_OUT
#endif

// Constant argument in function declaration
#ifndef CONST
#define CONST                   const
#endif

#ifndef IMSWMS_UINT8
typedef unsigned char           IMSWMS_UINT8;
#endif

#ifndef IMSWMS_UINT16
typedef unsigned short          IMSWMS_UINT16;
#endif

#ifndef IMSWMS_UINT32
typedef unsigned int            IMSWMS_UINT32;
#endif

#ifndef IMSWMS_ULONG
typedef unsigned long           IMSWMS_ULONG;
#endif

#ifndef IMSWMS_SINT8
typedef signed char             IMSWMS_SINT8;
#endif

#ifndef IMSWMS_SINT16
typedef signed short            IMSWMS_SINT16;
#endif

#ifndef IMSWMS_SINT32
typedef signed int              IMSWMS_SINT32;
#endif
/*
#ifndef IMSWMS_SLONG
typedef signed long             IMSWMS_SLONG;
#endif
*/

#ifndef IMSWMS_PVOID
typedef void*                   IMSWMS_PVOID;
#endif

#ifndef IMSWMS_BOOL
#ifdef __cplusplus
typedef bool                    IMSWMS_BOOL;
#else
typedef int                     IMSWMS_BOOL;
#endif // __cplusplus
#endif

#ifndef IMSWMS_CHAR
typedef char                    IMSWMS_CHAR;
#endif
/*
#ifndef IMSWMS_UCHAR
typedef unsigned char           IMSWMS_UCHAR;
#endif

#ifndef IMSWMS_WCHAR
typedef unsigned short          IMSWMS_WCHAR;
#endif
*/
#ifndef IMSWMS_BYTE
typedef unsigned char           IMSWMS_BYTE;
#endif
/*
#ifndef IMSWMS_SINT64
typedef signed long             IMSWMS_SINT64;
#endif

#ifndef IMSWMS_UINT64
typedef unsigned long           IMSWMS_UINT64;
#endif

#ifndef IMSWMS_FLOAT
typedef float                   IMSWMS_FLOAT;
#endif

#ifndef IMSWMS_DOUBLE
typedef double                  IMSWMS_DOUBLE;
#endif
*/

#ifndef IMSWMS_RESULT
typedef IMSWMS_SINT32              IMSWMS_RESULT;
#endif



// START :: definitions for 64-bit platform
#ifdef __IMS_LP64__

#ifndef IMSWMS_SIZE_T
typedef unsigned long           IMSWMS_SIZE_T;
#endif

#ifndef IMSWMS_SINTP
typedef signed long             IMSWMS_SINTP;
#endif

#ifndef IMSWMS_UINTP
typedef unsigned long           IMSWMS_UINTP;
#endif

#else // __IMSWMS_LP64__

#ifndef IMSWMS_SIZE_T
typedef unsigned int            IMSWMS_SIZE_T;
#endif

#ifndef IMSWMS_SINTP
typedef signed int              IMSWMS_SINTP;
#endif

#ifndef IMSWMS_UINTP
typedef unsigned int            IMSWMS_UINTP;
#endif

#endif // __IMSWMS_LP64__

// END :: definitions for 64-bit platform

// Send MO Request
#define IMSWMS_SOLUTION_URI_LEN                        128

class IWMSSmsSendRequestParam
{
    public:
        IMSWMS_UINT32       m_nSmsType;
        IMSWMS_CHAR         m_szDestAddr[IMSWMS_SOLUTION_URI_LEN+1];
        IMSWMS_UINT32       m_nSmsDataLen;
        IMSWMS_BYTE         m_baSmsData[512];
        IMSWMS_SINT32       m_nMsgId;
        IMSWMS_BOOL         m_bAckOrError;
        IMSWMS_SINT32       m_nSeqId;
        IMSWMS_SINT32       m_nSlotId;
};

#endif
