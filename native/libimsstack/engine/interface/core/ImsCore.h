#ifndef _IMS_CORE_H_
#define _IMS_CORE_H_

#include "IMSTypeDef.h"

/**
 * @brief This class defines the constant values for ICoreService ("imscore" protocol).
 */
class ImsCore
{
public:
    //// Connection related constants
    /// "imscore"
    static const IMS_CHAR CONNECTION_SCHEME[];

    //// App & Service related constants

    //// ServiceMethod related constants

    //// Media related constants
    /// "StreamMedia"
    static const IMS_CHAR MEDIA_STREAM[];
    /// "FramedMedia"
    static const IMS_CHAR MEDIA_FRAMED[];
    /// "BasicReliableMedia"
    static const IMS_CHAR MEDIA_BASIC_RELIABLE[];
    /// "BasicUnreliableMedia"
    static const IMS_CHAR MEDIA_BASIC_UNRELIABLE[];

    /// Types of media
    enum
    {
        MEDIA_TYPE_STREAM = 0,
        MEDIA_TYPE_FRAMED,
        MEDIA_TYPE_BASIC_RELIABLE,
        MEDIA_TYPE_BASIC_UNRELIABLE
    };
};

#endif  // _IMS_CORE_H_
