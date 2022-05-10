#ifndef MTS_CONFIGURATION_MANAGER_H_
#define MTS_CONFIGURATION_MANAGER_H_

#include "configuration/MtsCarrierConfigItems.h"
#include "ICarrierConfigListener.h"

class ICarrierConfig;

class MtsConfigurationManager final : public ICarrierConfigListener
{
public:
    MtsConfigurationManager();
    ~MtsConfigurationManager();
    MtsConfigurationManager(IN const MtsConfigurationManager&) = delete;
    MtsConfigurationManager& operator=(IN const MtsConfigurationManager&) = delete;

    void Init();
    void UpdateMtsConfig(IN const ICarrierConfig* piCc);

    // ICarrierConfigListener
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    // sms configurations
    IMS_BOOL IsSmsCsfbRetryOnFailure() const;  // KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL
    IMS_SINT32 GetSmsOverImsFormat() const;    // KEY_SMS_OVER_IMS_FORMAT_INT

private:
    MtsCarrierConfigItems m_objCarrierConfig;
};

#endif
