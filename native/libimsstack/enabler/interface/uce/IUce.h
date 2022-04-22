#ifndef _IUCE_H_
#define _IUCE_H_
#include <stdio.h>
#include <string.h>

#include "IMSList.h"
#include "IUUceService.h"
#include "AString.h"

typedef enum
{
    eUCE_RAT_INVALID = -1,
    eUCE_RAT_GERAN   = 0,
    eUCE_RAT_HRPD,
    eUCE_RAT_UTRAN,
    eUCE_RAT_EHRPD,
    eUCE_RAT_LTE,
    eUCE_RAT_LTE_NO_VOPS,
    eUCE_RAT_WIFI,
    eUCE_RAT_NR,
    eUCE_RAT_NR_NO_VOPS
}UCE_NETWORK_ENTYPE;

class IUcePubCmdPrm { // UCE_SEND_PUBLISH_CMD
public :
    inline IUcePubCmdPrm() {
        m_nKey = 0;
        m_nExtended = 1;
        m_nCapability = 0;
    }
    inline virtual ~IUcePubCmdPrm() { }

public :
    IMS_UINT32  m_nKey;
    IMS_UINT32  m_nExtended;
    IMS_UINT32  m_nCapability;
    AString     m_strPidfXml;
    AString     m_strEtag;
};

//subscribe info
class IUceSingleSubCmdPrm { // UCE_SEND_SINGLE_SUBSCRIBE_CMD
public :
    inline IUceSingleSubCmdPrm() {
        m_nKey  = 0;
        m_nSize = 1;
    }
    inline virtual ~IUceSingleSubCmdPrm() {}

public :
    IMS_UINT32  m_nKey;
    IMS_UINT32  m_nSize;
    AString     m_strUser;
};

class IUceListSubCmdPrm { // UCE_SEND_LIST_SUBSCRIBE_CMD
public:
    inline IUceListSubCmdPrm() {
        m_nKey  = 0;
    }
    inline virtual ~IUceListSubCmdPrm() {}

public :
    IMS_UINT32  m_nKey;
    IMS_UINT32  m_nSize;
    IMSList <AString>   userList;
};

class IUceOptionsCmdPrm { // UCE_SEND_OPTIONS_CMD
public :
    inline IUceOptionsCmdPrm() {
        m_nKey  = 0;
        m_nMyCaps = 0;
    }
    inline virtual ~IUceOptionsCmdPrm() {}

public :
    IMS_UINT32  m_nKey;
    AString m_strRemoteUri;
    IMS_UINT32  m_nMyCaps;
};

class IUceOptionsRespCmdPrm { // UCE_SEND_OPTIONS_RESP_CMD
public :
    inline IUceOptionsRespCmdPrm() {
        m_nKey = 0;
        m_nResponseCode = 0;
        m_nMyCaps = 0;
    }
    inline virtual ~IUceOptionsRespCmdPrm() {}

public :
    IMS_UINT32  m_nKey;
    IMS_SINT32  m_nResponseCode;
    AString     m_strReason;
    IMS_UINT32  m_nMyCaps;
};

class IUcePubResponseIndPrm { // UCE_PUBLISH_RESPONSE_IND
public:
    inline IUcePubResponseIndPrm() {
        m_nKey = 0;
        m_nCapability = 0;
        m_nResponseCode = 0;
        m_nReasonHeaderCause = 0;
        m_nNeedToRetry = 0;
    }
    inline virtual ~IUcePubResponseIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nResponseCode;
    IMS_UINT32      m_nCapability;
    AString         m_strReason;
    IMS_UINT32      m_nReasonHeaderCause;
    AString         m_strReasonHeaderText;
    AString         m_strEtag;
    IMS_UINT32      m_nNeedToRetry;
};

class IUcePubUpdatedIndPrm { // UCE_PUBLISH_UPDATED_IND
public:
    inline IUcePubUpdatedIndPrm() {
        m_nCapability = 0;
        m_nResponseCode = 0;
        m_nReasonHeaderCause = 0;
        m_nNeedToRetry = 0;
    }
    inline virtual ~IUcePubUpdatedIndPrm() {}

public :
    IMS_UINT32      m_nCapability;
    IMS_UINT32      m_nResponseCode;
    AString         m_strReason;
    IMS_UINT32      m_nReasonHeaderCause;
    AString         m_strReasonHeaderText;
    AString         m_strEtag;
    IMS_UINT32      m_nNeedToRetry;
};

class IUcePubCmdErrorIndPrm { // UCE_PUBLISH_CMD_ERROR_IND
public:
    inline IUcePubCmdErrorIndPrm() {
        m_nKey = 0;
        m_nCommandError = 0;
    }
    inline virtual ~IUcePubCmdErrorIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nCommandError;
};

class IUceSubResponseIndPrm { // UCE_SUBSCRIBE_RESPONSE_IND
public:
    inline IUceSubResponseIndPrm() {
        m_nKey = 0;
        m_nResponseCode = 0;
        m_nReasonHeaderCause = 0;
    }
    inline virtual ~IUceSubResponseIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nResponseCode;
    AString         m_strReason;
    IMS_UINT32      m_nReasonHeaderCause;
    AString         m_strReasonHeaderText;
};

class IUcePreNotifyIndPrm { // UCE_PRESENCE_NOTIFY_IND
public:
    inline IUcePreNotifyIndPrm() {
        m_nKey = 0;
        m_nCount = 0;
    }
    inline virtual ~IUcePreNotifyIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nCount;
    IMSList <AString> m_lstPidfXmls;
};

class IUceSubCmdErrorIndPrm { // UCE_SUBSCRIBE_CMD_ERROR_IND
public:
    inline IUceSubCmdErrorIndPrm() {
        m_nKey = 0;
        m_nCommandError = 0;
    }
    inline virtual ~IUceSubCmdErrorIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nCommandError;
};

class IUceTerminatedReason
{
public :
    inline IUceTerminatedReason() {}
    inline ~IUceTerminatedReason() {}

public :
    AString   m_strContact;
    AString   m_strReason;
};

class IUceSubResourceTerminatedIndPrm { // UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND
public:
    inline IUceSubResourceTerminatedIndPrm() {
        m_nKey = 0;
        m_nCount = 0;
    }
    inline virtual ~IUceSubResourceTerminatedIndPrm() {
        for(IMS_UINT32 i = 0; i < m_lstTerminateContacts.GetSize(); i++) {
            if(m_lstTerminateContacts.GetAt(i) != null)
            delete m_lstTerminateContacts.GetAt(i);
        }
    }

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nCount;
    IMSList <IUceTerminatedReason*>  m_lstTerminateContacts;
};

class IUceSubTerminatedIndPrm { // UCE_SUBSCRIBE_TERMINATED_IND
public:
    inline IUceSubTerminatedIndPrm() {
        m_nKey = 0;
        m_nRetryAfterMillsecond = 0;
    }
    inline virtual ~IUceSubTerminatedIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    AString         m_strReason;
    IMS_UINT32      m_nRetryAfterMillsecond;
};

class IUceOptionsResponseIndPrm { // UCE_OPTIONS_RESPONSE_IND
public:
    inline IUceOptionsResponseIndPrm() {
        m_nKey = 0;
        m_nResponseCode = 0;
        m_nTheirCaps = 0;
    }
    inline virtual ~IUceOptionsResponseIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nResponseCode;
    AString         m_strReason;
    IMS_UINT32      m_nTheirCaps;
};

class IUceOptionsCmdErrorIndPrm { // UCE_OPTIONS_CMD_ERROR_IND
public:
    inline IUceOptionsCmdErrorIndPrm() {
        m_nKey = 0;
        m_nCommandError = 0;
    }
    inline virtual ~IUceOptionsCmdErrorIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    IMS_UINT32      m_nCommandError;
};

class IUceOptionsReceivedIndPrm { // UCE_OPTIONS_RECEIVED_IND
public:
    inline IUceOptionsReceivedIndPrm() {
        m_nKey = 0;
        m_nRemoteCaps = 0;
    }
    inline virtual ~IUceOptionsReceivedIndPrm() {}

public :
    IMS_UINT32      m_nKey;
    AString         m_strRemote;
    IMS_UINT32      m_nRemoteCaps;
};
#endif //_IUCE_H_
