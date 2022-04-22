#include "INetworkWatcher.h"
#include "ServicePhoneInfo.h"
#include "helper/MtcAosConnector.h"
#include "helper/block/NetworkBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
NetworkBlockRule::NetworkBlockRule(
        IN const IMtcService& objService, IN INetWatcherInfo& objNetWatcherInfo) :
        m_objService(objService),
        m_objNetWatcherInfo(objNetWatcherInfo)
{
}

PUBLIC VIRTUAL
NetworkBlockRule::~NetworkBlockRule()
{
}

PUBLIC VIRTUAL
NetworkBlockRule::Result NetworkBlockRule::Check(IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (IsInEpdg(m_objService) || IsWifiRegistered(m_objService.GetAosConnector()))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_UINT32 nNetworkType = m_objNetWatcherInfo.GetNetRadioTechType();
    if (nNetworkType == NW_REPORT_RADIO_LTE ||
            nNetworkType == NW_REPORT_RADIO_NR)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : Network type[%d] is not applicable", nNetworkType, 0, 0);
    return Result(
            Result::Status::BLOCKED,
            FailReason(REJECT_REASON_SESSION_NOTACCEPTABLEHERE));
}

PRIVATE
IMS_BOOL NetworkBlockRule::IsInEpdg(IN const IMtcService& objService)
{
    return objService.IsWlanIpCanType();
}

PRIVATE
IMS_BOOL NetworkBlockRule::IsWifiRegistered(IN MtcAosConnector* pAosConnector)
{
    IMS_UINT32 nAosRegisteredNetworkType =
            pAosConnector ? pAosConnector->GetRegisteredNetworkType() : NW_REPORT_RADIO_INVALID;

    return nAosRegisteredNetworkType == NW_REPORT_RADIO_WLAN;
}
