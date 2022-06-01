/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170621  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_FACTORY_PROXY_H_
#define _SIP_FACTORY_PROXY_H_

#include "IMSTypeDef.h"

class ISipTokenGenerator;
class SIPIPSecState;
class SIPMessageTracker;
class SIPPacketTracker;
class SipRoutingRejectNotifier;
class SIPRTConfigHelper;
class SIPTransportHelper;
class SIPFactoryProxyPrivate;

class SIPFactoryProxy
{
private:
    SIPFactoryProxy();
    ~SIPFactoryProxy();

    SIPFactoryProxy(IN const SIPFactoryProxy& objRHS);
    SIPFactoryProxy& operator=(IN const SIPFactoryProxy& objRHS);

public:
    SIPIPSecState* GetIPSecState(IN IMS_SINT32 nSlotId);
    SIPMessageTracker* GetMessageTracker(IN IMS_SINT32 nSlotId);
    SIPPacketTracker* GetPacketTracker(IN IMS_SINT32 nSlotId);
    SipRoutingRejectNotifier* GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId);
    SIPRTConfigHelper* GetRtConfigHelper(IN IMS_SINT32 nSlotId);
    SIPTransportHelper* GetTransportHelper(IN IMS_SINT32 nSlotId);
    void SetTokenGenerator(IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator);

    IMS_BOOL IsIPSecStateEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsMessageTrackerEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsPacketTrackerEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsRoutingRejectNotifierEnabled(IN IMS_SINT32 nSlotId) const;

public:
    static void CreateInstance();
    static void DestroyInstance();
    static SIPFactoryProxy* GetInstance();

private:
    static SIPFactoryProxy* pFactoryProxy;

    SIPFactoryProxyPrivate* pPrivate;
};

#endif  // _SIP_FACTORY_PROXY_H_
