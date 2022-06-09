#ifndef MTC_EXTENSION_H_
#define MTC_EXTENSION_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "call/extension/IMtcExtension.h"

class IMessage;

/**
 * This class provides basic methods for general extensions without any extension-specific logic.
 */
class MtcExtension : public IMtcExtension
{
public:
    explicit MtcExtension(IN const AString& strOptionTag);
    explicit MtcExtension(IN const MtcExtension& objRhs);
    virtual ~MtcExtension();
    MtcExtension& operator=(IN const MtcExtension&) = delete;

    IMtcExtension* Clone() const override;
    IMS_BOOL IsAvailableOnRemote() const override;
    IMS_BOOL IsRequiredOnRemote() const override;
    const AString& GetOptionTag() const override;

    void FormatRequest(IN IMS_UINT32 nMethod, IN_OUT IMessage& objRequest) override;
    void FormatResponse(IN IMS_UINT32 nMethod, IN_OUT IMessage& objResponse) override;
    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) override;
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) override;

private:
    void UpdateFromRequireAndSupportedHeader(IN const IMessage& objMessage);

    AString m_strOptionTag;

    IMS_BOOL m_bRequiredOnRemote;
    IMS_BOOL m_bSupportedOnRemote;
};

#endif
