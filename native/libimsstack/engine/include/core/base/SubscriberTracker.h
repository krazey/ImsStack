/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110517  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUBSCRIBER_TRACKER_H_
#define _SUBSCRIBER_TRACKER_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "SipAddress.h"
#include "ISubscriberInfoListener.h"

class IMutex;

class SubscriberTracker : public ISubscriberInfoListener
{
private:
    SubscriberTracker();
    SubscriberTracker(IN const SubscriberTracker& objRHS);
    SubscriberTracker& operator=(IN const SubscriberTracker& objRHS);

public:
    virtual ~SubscriberTracker();

public:
    const AString& GetSubscriberId(IN IMS_SINT32 nSlotId, IN const AString& strAOR) const;
    const AString& GetSubscriberId(IN IMS_SINT32 nSlotId, IN const SipAddress* pAOR) const;

    static SubscriberTracker* GetInstance();

protected:
    // ISubscriberInfoListener class
    virtual void SubscriberInfo_UpdateIMPU(IN IMS_SINT32 nSlotId, IN const AString& strId,
            IN const AString& strOld, IN const AString& strNew);

private:
    IMSMap<AString, IMSList<SipAddress*>>* GetSubscribers(IN IMS_SINT32 nSlotId) const;
    void Initialize();
    void InitForSlot(IN IMS_SINT32 nSlotId);

private:
    friend class EngineLoader;

    IMutex* piLock;
    // < SubscriberConfig id, IMPUs >
    // DEFAULT_ID : "default"
    IMSMap<AString, IMSList<SipAddress*>>* pSubscriberMaps;
};

#endif  // _SUBSCRIBER_TRACKER_H_
