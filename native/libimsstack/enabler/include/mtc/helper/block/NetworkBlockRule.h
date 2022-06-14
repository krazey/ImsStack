#ifndef NETWORK_BLOCK_RULE_H_
#define NETWORK_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "call/IMtcCall.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallContext;
class INetworkWatcher;

class NetworkBlockRule final : public IMtcBlockRule
{
public:
    explicit NetworkBlockRule(IN IMtcCallContext& objContext);
    virtual ~NetworkBlockRule();
    NetworkBlockRule(IN const NetworkBlockRule&) = delete;
    NetworkBlockRule& operator=(IN const NetworkBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_BOOL IsInEpdg(IN const IMtcService& objService);
    IMS_BOOL IsWifiRegistered(IN MtcAosConnector* pAosConnector);

    INetworkWatcher& GetNetWatcherInfo(IN IMS_SINT32 nSlotId);

    const IMtcService& m_objService;
    INetworkWatcher& m_objNetWatcherInfo;
    const PeerType m_ePeerType;
};

#endif
