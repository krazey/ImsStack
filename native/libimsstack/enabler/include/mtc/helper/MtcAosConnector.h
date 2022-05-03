#ifndef MTC_AOS_CONNECTOR_H_
#define MTC_AOS_CONNECTOR_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"

class MtcAosConnector
{
public:
    inline MtcAosConnector(IN IImsAos& objImsAos, IN IImsAosInfo& objImsAosInfo) :
            m_objImsAos(objImsAos),
            m_objImsAosInfo(objImsAosInfo)
    {
    }
    inline ~MtcAosConnector() {}
    MtcAosConnector(IN const MtcAosConnector&) = delete;
    MtcAosConnector& operator=(IN const MtcAosConnector&) = delete;

    // IImsAos interface wrappers.
    inline IMS_UINT32 GetFeatures() const { return m_objImsAos.GetFeatures(); }
    inline IMS_UINT32 GetSuspendedReason() const { return m_objImsAos.GetSuspendedReason(); }
    inline IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const
    {
        return m_objImsAos.IsFeatureConnected(nFeature);
    }
    inline IMS_BOOL IsImsConnected() const { return m_objImsAos.IsImsConnected(); }
    inline IMS_BOOL IsImsSuspended() const { return m_objImsAos.IsImsSuspended(); }
    inline void UpdateFeature(IN IMS_UINT32 nFeatures) const
    {
        m_objImsAos.UpdateFeature(nFeatures);
    }
    inline IMS_BOOL Control(IN IMS_UINT32 nType) const { return m_objImsAos.Control(nType); }

    // IImsAosInfo interface wrappers.
    inline AString GetAssociatedUri() const { return m_objImsAosInfo.GetAssociatedUri(); }
    inline IMS_UINT32 GetConnectionType() const { return m_objImsAosInfo.GetConnectionType(); }
    inline IMS_UINT32 GetImsState() const { return m_objImsAosInfo.GetImsState(); }
    inline IMS_UINT32 GetIpcanType() const { return m_objImsAosInfo.GetIpcanType(); }
    inline AString GetLastPathHeaderValue() const
    {
        return m_objImsAosInfo.GetLastPathHeaderValue();
    }
    inline AString GetLocalAddress() const { return m_objImsAosInfo.GetLocalAddress(); }
    inline IMS_UINT32 GetLocalPort() const { return m_objImsAosInfo.GetLocalPort(); }
    inline IMS_UINT32 GetRegisteredNetworkType() const
    {
        return m_objImsAosInfo.GetRegisteredNetworkType();
    }
    inline AString GetPathHeaderValue() const { return m_objImsAosInfo.GetPathHeaderValue(); }
    inline AString GetPcscfAddress() const { return m_objImsAosInfo.GetPcscfAddress(); }
    inline IMS_UINT32 GetPcscfPort() const { return m_objImsAosInfo.GetPcscfPort(); }
    inline IMS_UINT32 GetRegistrationMode() const { return m_objImsAosInfo.GetRegistrationMode(); }
    inline AString GetSupportedHeaderValue() const
    {
        return m_objImsAosInfo.GetSupportedHeaderValue();
    }
    inline AString GetServiceRouteHeaderValue() const
    {
        return m_objImsAosInfo.GetServiceRouteHeaderValue();
    }

    inline void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized)
    {
        return m_objImsAosInfo.NotifyEmergencyCallState(bIsInitialized);
    }

private:
    IImsAos& m_objImsAos;
    IImsAosInfo& m_objImsAosInfo;
};

#endif
