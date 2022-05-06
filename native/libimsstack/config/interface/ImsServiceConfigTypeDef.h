#ifndef IMS_SERVICE_CONFIG_TYPE_DEF_H_
#define IMS_SERVICE_CONFIG_TYPE_DEF_H_

#include "AString.h"

/// Defines an application identifier for IMS service.
enum class ImsAppId
{
    UNKNOWN = 0,
    /// MTC (Multimedia Telephony Call)
    MTC = 1,
    /// MTS (Multimedia Telephony SMS)
    MTS = 2,
    /// UCE (User Capability Exchange)
    UCE = 3,
    /// SipDelegate (used for RCS Single Registration)
    SIP_DELEGATE = 4
};

/// Defines a service identifier for IMS service.
/// The application id can be derived from these service id.
enum class ImsServiceId
{
    UNKNOWN = 0,

    /// MTC
    MTC = 11,
    MTC_EMERGENCY = 12,

    /// MTS
    MTS = 21,
    MTS_EMERGENCY = 22,

    /// UCE
    UCE = 31,

    /// SIP_DELEGATE
    SIP_DELEGATE = 41
};

// Data type for carrying the identifiers for IMS service.
struct ImsServiceIdentifier
{
    ImsAppId eAppId;
    ImsServiceId eServiceId;
};

struct ImsServiceProfile
{
    // Count of IMS services.
    IMS_UINT32 nCount;
    // List of IMS service's identifiers.
    ImsServiceIdentifier* pServiceIds;
};

// Data type for carrying the IMS service's identifier as a string.
class ImsServiceName
{
public:
    inline ImsServiceName(IN const AString& strAppId, IN const AString& strServiceId) :
            m_strAppId(strAppId),
            m_strServiceId(strServiceId)
    {
    }
    inline ImsServiceName(IN const ImsServiceName& objOther) :
            m_strAppId(objOther.m_strAppId),
            m_strServiceId(objOther.m_strServiceId)
    {
    }
    inline ~ImsServiceName() {}

    inline ImsServiceName& operator=(IN const ImsServiceName& objOther)
    {
        if (this != &objOther)
        {
            m_strAppId = objOther.m_strAppId;
            m_strServiceId = objOther.m_strServiceId;
        }

        return (*this);
    }

    inline const AString& GetAppId() const { return m_strAppId; }
    inline const AString& GetServiceId() const { return m_strServiceId; }

private:
    AString m_strAppId;
    AString m_strServiceId;
};

#endif
