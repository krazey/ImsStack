/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _CAP_PROPERTY_H_
#define _CAP_PROPERTY_H_

#include "private/ImsProperty.h"

class CapPropertyPrivate;

/*

Class CapProperty

Example

See Also

*/
class CapProperty : public ImsProperty
{
public:
    CapProperty();
    CapProperty(IN IMS_SINT32 nSectorId_, IN IMS_SINT32 nMessageType_);
    CapProperty(IN const CapProperty& objRHS);
    virtual ~CapProperty();

public:
    CapProperty& operator=(IN const CapProperty& objRHS);

public:
    // ImsProperty class
    virtual IMS_BOOL Equals(IN const AString& strValue) const;

    void AddValue(IN const AString& strValue);
    const AStringArray& GetValues() const;
    void SetKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType);

    static AString CreateCapKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType);
    static AString MessageTypeToString(IN IMS_SINT32 nMessageType);
    static AString SectorIdToString(IN IMS_SINT32 nSectorId);
    static IMS_SINT32 StringToMessageType(IN const AString& strMessageType);
    static IMS_SINT32 StringToSectorId(IN const AString& strSectorId);

public:
    // Sector Id
    enum
    {
        SECTOR_INVALID = 0,
        SECTOR_SESSION,
        SECTOR_FRAMED,
        SECTOR_STREAM_AUDIO,
        SECTOR_STREAM_VIDEO,
        SECTOR_MAX
    };

    // Message type
    enum
    {
        MESSAGE_TYPE_INVALID = 0,
        MESSAGE_TYPE_REQUEST,
        MESSAGE_TYPE_RESPONSE,
        MESSAGE_TYPE_REQUEST_RESPONSE,
        MESSAGE_TYPE_MAX
    };

    static const IMS_CHAR* SECTOR_STRING[SECTOR_MAX];
    static const IMS_CHAR* MESSAGE_TYPE_STRING[MESSAGE_TYPE_MAX];

private:
    CapPropertyPrivate* pCapPP;
};

#endif  // _CAP_PROPERTY_H_
