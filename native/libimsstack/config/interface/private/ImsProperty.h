/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _IMS_PROPERTY_H_
#define _IMS_PROPERTY_H_

#include "ImsRegistry.h"

/*

Class ImsProperty

Example

See Also

*/
class ImsProperty
{
public:
    explicit ImsProperty(IN IMS_SINT32 nKey_, IN const AString& strKey_ = AString::ConstNull());
    ImsProperty(IN const ImsProperty& objRHS);
    virtual ~ImsProperty();

public:
    ImsProperty& operator=(IN const ImsProperty& objRHS);

public:
    virtual IMS_BOOL Equals(IN const AString& strValue) const;
    virtual IMS_BOOL Equals(IN const ImsProperty& objOther) const;

    static AStringArray Decode(IN const AString& strValue);
    static AString Encode(IN const AStringArray& objValues);
    static IMS_BOOL CheckDuplicate(IN const AStringArray& objValues, IN IMS_BOOL bCaseSensitive);
    static AString KeyToString(IN IMS_SINT32 nKey);
    static IMS_SINT32 StringToKey(IN const AString& strKey);
    static IMS_BOOL TrimAndCheckProperties(
            IN const ImsRegistry& objRegistry, OUT ImsRegistry& objNewRegistry);

    // DEBUG
    static AString ToString(IN const AStringArray& objProperty);

public:
    // Type of property key
    enum
    {
        // User-defined property
        PKEY_CUSTOM = 0,

        //////
        // Basic Properties
        //////

        // Media types to stream audio and/or video
        PKEY_STREAM = 1,
        // Messaging (MSRP)
        PKEY_FRAMED,
        // MIME content type to transfer media content
        PKEY_BASIC,
        // Event packages
        PKEY_EVENT,
        // Composed capabilities of core services
        PKEY_CORE_SERVICE,
        // Support QoS aware BasicMedia
        PKEY_QOS,

        //////
        // Advanced Properties
        //////

        // Headers to be added in SIP registration message
        PKEY_REG,
        // Write-access headers in SIP message
        PKEY_WRITE,
        // Read-access headers in SIP message
        PKEY_READ,
        // Configure capability
        PKEY_CAP,
        // Media profile
        PKEY_MPROF,
        // Connection model for MSRP used in FramedMedia
        PKEY_CONNECTION,

        PKEY_MAX
    };

    // Key string of Registry property
    static const IMS_CHAR* PKEY_STRING[PKEY_MAX];

    // Type of StreamMedia : Audio, Video
    static const IMS_CHAR STREAM_MEDIA_TYPE_AUDIO[];
    static const IMS_CHAR STREAM_MEDIA_TYPE_VIDEO[];

protected:
    IMS_SINT32 nKey;
    AString strKey;
};

#endif  // _IMS_PROPERTY_H_
