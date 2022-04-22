/*
    Author
    IMSers
    <table>
    Date      Description
    --------  ----------
    20100209  Created
    </table>

    Description
     This file defines a type-casting method for IMS client platform without any dependency
     of compiler.
*/

#ifndef _IMS_TYPE_CAST_H_
#define _IMS_TYPE_CAST_H_

#if defined(WINCE) || defined(WIN32)

#define    IMS_RTTI_ENABLED
#endif // defined(WINCE) || defined(WIN32)

#ifdef IMS_RTTI_ENABLED

#define    CONST_CAST(TYPE,VALUE)                (const_cast<TYPE>(VALUE))
#define    DYNAMIC_CAST(TYPE,VALUE)              (dynamic_cast<TYPE>(VALUE))
#define    REINTERPRET_CAST(TYPE,VALUE)          (reinterpret_cast<TYPE>(VALUE))
#define    STATIC_CAST(TYPE,VALUE)               (static_cast<TYPE>(VALUE))

#else

// C-style type casting
#define    CONST_CAST(TYPE,VALUE)                (const_cast<TYPE>(VALUE))
#define    DYNAMIC_CAST(TYPE,VALUE)              ((TYPE)(VALUE))
#define    REINTERPRET_CAST(TYPE,VALUE)          (reinterpret_cast<TYPE>(VALUE))
#define    STATIC_CAST(TYPE,VALUE)               (static_cast<TYPE>(VALUE))

#endif // IMS_RTTI_ENABLED

#endif // _IMS_TYPE_CAST_H_
