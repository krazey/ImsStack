#ifndef MTS_SIP_FORM_UTILS_H_
#define MTS_SIP_FORM_UTILS_H_

#include "AString.h"

class MtsDialingPlan;

class MtsSipFormUtils final
{
public:
    MtsSipFormUtils(IN IMS_SINT32 nSlotId);
    ~MtsSipFormUtils();

    static MtsSipFormUtils* GetInstance(IN IMS_SINT32 nSlotId);
    IMS_BOOL FormDestination(IN const IMS_CHAR* szMDN, IN const IMS_BOOL bIsAckorError,
            IN const AString& strLastIpSmgw, OUT AString& strDest);
    AString FormContentTypeEnumToStr(IN IMS_UINT32 nType);
    IMS_UINT32 FormContentTypeStrToEnum(IN AString strContentType);
    void UpdateFormatFromDb();
    IMS_BOOL UpdatePsiFromDb();
    IMS_SINT32 GetSlotId();
    AString ValidateAndUpdatePsi();
    IMS_BOOL IsTelUrlParam(IN const AString& strParam) const;
    IMS_BOOL IsNumberFormat(IN const AString& strDial) const;
    IMS_BOOL IsIpAddress(IN const AString& strIp) const;
    IMS_SINT32 CheckScheme(IN const AString& strScheme) const;

protected:
    MtsDialingPlan* GetDialingPlan(IN IMS_SINT32 nSlotId);

private:
    static IMS_BOOL IsVisualSeparator(IN IMS_CHAR ch);

public:
    enum
    {
        SCHEME_UNKNOWN = 0,
        SCHEME_TEL,
        SCHEME_SIP,
        SCHEME_SIPS
    };

public:
    IMS_UINT32 m_nMtsFormat;

protected:
    MtsDialingPlan* m_pMtsDialingPlan;
    AString m_strPsi;
    IMS_SINT32 m_nSlotId;
};

#endif
