/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "private/CapProperty.h"

class CapPropertyPrivate
{
public:
    inline CapPropertyPrivate() :
            nSectorId(CapProperty::SECTOR_INVALID),
            nMessageType(CapProperty::MESSAGE_TYPE_INVALID)
    {
    }

private:
    friend class CapProperty;

    IMS_SINT32 nSectorId;
    IMS_SINT32 nMessageType;
    AStringArray objSDPFields;
};

PUBLIC GLOBAL const IMS_CHAR* CapProperty::SECTOR_STRING[CapProperty::SECTOR_MAX] = {
        "",
        "Session",
        "Framed",
        "StreamAudio",
        "StreamVideo",
};

PUBLIC GLOBAL const IMS_CHAR* CapProperty::MESSAGE_TYPE_STRING[CapProperty::MESSAGE_TYPE_MAX] = {
        "",
        "Req",
        "Resp",
        "Req_Resp",
};

PUBLIC
CapProperty::CapProperty() :
        ImsProperty(ImsProperty::PKEY_CAP),
        pCapPP(new CapPropertyPrivate())
{
}

PUBLIC
CapProperty::CapProperty(IN IMS_SINT32 nSectorId_, IN IMS_SINT32 nMessageType_) :
        ImsProperty(ImsProperty::PKEY_CAP),
        pCapPP(new CapPropertyPrivate())
{
    pCapPP->nSectorId = nSectorId_;
    pCapPP->nMessageType = nMessageType_;
}

PUBLIC
CapProperty::CapProperty(IN const CapProperty& objRHS) :
        ImsProperty(objRHS),
        pCapPP(new CapPropertyPrivate())
{
    pCapPP->nSectorId = objRHS.pCapPP->nSectorId;
    pCapPP->nMessageType = objRHS.pCapPP->nMessageType;
    pCapPP->objSDPFields = objRHS.pCapPP->objSDPFields;
}

PUBLIC VIRTUAL CapProperty::~CapProperty()
{
    if (pCapPP != IMS_NULL)
    {
        delete pCapPP;
    }
}

PUBLIC
CapProperty& CapProperty::operator=(IN const CapProperty& objRHS)
{
    if (this != &objRHS)
    {
        pCapPP->nSectorId = objRHS.pCapPP->nSectorId;
        pCapPP->nMessageType = objRHS.pCapPP->nMessageType;
        pCapPP->objSDPFields = objRHS.pCapPP->objSDPFields;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL CapProperty::Equals(IN const AString& strValue) const
{
    AString strCapKey = CreateCapKey(pCapPP->nSectorId, pCapPP->nMessageType);

    return strCapKey.Equals(strValue);
}

PUBLIC
void CapProperty::AddValue(IN const AString& strValue)
{
    pCapPP->objSDPFields.AddElement(strValue);
}

PUBLIC
const AStringArray& CapProperty::GetValues() const
{
    return pCapPP->objSDPFields;
}

PUBLIC
void CapProperty::SetKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType)
{
    pCapPP->nSectorId = nSectorId;
    pCapPP->nMessageType = nMessageType;
}

PUBLIC GLOBAL AString CapProperty::CreateCapKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType)
{
    if ((nSectorId == SECTOR_INVALID) || (nMessageType == MESSAGE_TYPE_INVALID) ||
            (nMessageType == MESSAGE_TYPE_REQUEST_RESPONSE))
    {
        return AString();
    }

    AString strCapKey;
    IMS_SINT32 nCapKey = IMS_SINT32(nSectorId + (nMessageType << 4));

    return strCapKey.SetNumber(nCapKey, 16);
}

PUBLIC GLOBAL AString CapProperty::MessageTypeToString(IN IMS_SINT32 nMessageType)
{
    if ((nMessageType > MESSAGE_TYPE_INVALID) && (nMessageType < MESSAGE_TYPE_MAX))
    {
        return AString(MESSAGE_TYPE_STRING[nMessageType]);
    }

    return AString();
}

PUBLIC GLOBAL AString CapProperty::SectorIdToString(IN IMS_SINT32 nSectorId)
{
    if ((nSectorId > SECTOR_INVALID) && (nSectorId < SECTOR_MAX))
    {
        return AString(SECTOR_STRING[nSectorId]);
    }

    return AString();
}

PUBLIC GLOBAL IMS_SINT32 CapProperty::StringToMessageType(IN const AString& strMessageType)
{
    for (IMS_SINT32 i = (MESSAGE_TYPE_INVALID + 1); i < MESSAGE_TYPE_MAX; ++i)
    {
        if (strMessageType.Equals(MESSAGE_TYPE_STRING[i]))
        {
            return i;
        }
    }

    return MESSAGE_TYPE_INVALID;
}

PUBLIC GLOBAL IMS_SINT32 CapProperty::StringToSectorId(IN const AString& strSectorId)
{
    for (IMS_SINT32 i = (SECTOR_INVALID + 1); i < SECTOR_MAX; ++i)
    {
        if (strSectorId.Equals(SECTOR_STRING[i]))
        {
            return i;
        }
    }

    return SECTOR_INVALID;
}
