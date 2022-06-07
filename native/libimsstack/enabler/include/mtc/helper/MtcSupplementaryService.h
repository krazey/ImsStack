#ifndef MTC_SUPPLEMENTARY_SERVICE_H_
#define MTC_SUPPLEMENTARY_SERVICE_H_

#include "IMSMap.h"
#include "MtcDef.h"

class ISession;
class IMessage;
class ISipHeader;
class SipAddress;
class MtcConfigurationProxy;

class MtcSupplementaryService final
{
public:
    explicit MtcSupplementaryService(IN MtcConfigurationProxy& objConfigurationProxy,
            IN IMSMap<SuppType, SuppService*> objSuppServices = IMSMap<SuppType, SuppService*>());
    ~MtcSupplementaryService();
    MtcSupplementaryService(const MtcSupplementaryService&) = delete;
    MtcSupplementaryService& operator=(const MtcSupplementaryService&) = delete;

    void UpdateOutgoingServices(IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void UpdateTip(IN IMessage* piMessage);

    IMS_BOOL UpdateIncomingServices(IN IMessage* piMessage);
    IMS_BOOL UpdateCallerId(IN IMessage* piMessage);
    IMS_BOOL UpdateCnap(IN IMessage* piMessage);
    IMS_BOOL UpdateCnapEx(IN IMessage* piMessage);
    IMS_BOOL UpdateMmc(IN IMessage* piMessage);
    IMS_BOOL UpdateGtt(IN IMessage* piMessage);
    IMS_BOOL UpdateCdivCause(IN IMessage* piMessage);
    IMS_BOOL UpdateCdivHistory(IN IMessage* piMessage);
    IMS_BOOL UpdateCw(IN IMessage* piMessage);
    IMS_BOOL UpdateVm(IN IMessage* piMessage);
    IMS_BOOL UpdateAnswerHold(IN IMessage* piMessage);
    IMS_BOOL UpdateMcid(IN IMessage* piMessage);
    IMS_BOOL UpdateDualNumber(IN IMessage* piMessage);
    IMS_BOOL UpdateCallingNumVerification(IN IMessage* piMessage);
    void Delete(IN SuppType eType);
    void DeleteServices();
    const SuppService* Get(IN SuppType eType);
    const IMSMap<SuppType, SuppService*>& GetServices();
    void Add(IN SuppType eSuppType, IN AString strValue);
    void Add(IN SuppType eSuppType, IN IMS_SINT32 nValue);
    void Add(IN SuppType eSuppType, IN IMS_BOOL bValue);

private:
    ISipHeader* GetHistoryInfoHeader(IN IMessage* piMessage);
    IMS_BOOL GetCdivCause(IN const SipAddress* pAddress, OUT IMS_SINT32& nCause);
    IMS_BOOL GetCdivTarget(IN const SipAddress* pAddress, OUT AString& strTarget);
    IMS_SINT32 ConvertCdivCause(IN IMS_SINT32 nCause);
    IMS_SINT32 GetCallingNumVerificationResult(IN AString& strValue);
    IMS_SINT32 GetCnvHeaderType(IN IMessage* piMessage);
    void LoadConfig();
    IMS_BOOL IsExist(IN SuppType suppType);

private:
    IMSMap<SuppType, SuppService*> m_objSuppService;
    MtcConfigurationProxy& m_objConfigurationProxy;
    IMS_SINT32 m_nCnapType;

    static const IMS_CHAR STR_VERSTAT[];
    static const IMS_CHAR STR_VERSTAT_TN_VALIDATION_PASSED[];
    static const IMS_CHAR STR_VERSTAT_TN_VALIDATION_FAILED[];
};
#endif
