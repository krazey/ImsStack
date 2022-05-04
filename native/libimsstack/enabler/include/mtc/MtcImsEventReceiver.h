#ifndef MTC_IMS_EVENT_RECEIVER_H_
#define MTC_IMS_EVENT_RECEIVER_H_

#include "IEventListener.h"
#include "IMSList.h"
#include "IMSMap.h"
#include "IMSTypeDef.h"

class IMtcImsEventListener;

using ImsEvent = IMS_SINT32;

/*
 * This class receives event from Java layer defined in ImsEventDef.h. The users can read
 * cached values or receive notification when the event occurs.
 * For supported events, see `RegisterSupportedEvents()`.
 */
class MtcImsEventReceiver final : public IEventListener
{
public:
    static const IMS_UINT32 UNKNOWN_VALUE;

    explicit MtcImsEventReceiver(IN IMS_SINT32 nSlotId);
    ~MtcImsEventReceiver();
    MtcImsEventReceiver(const MtcImsEventReceiver&) = delete;
    MtcImsEventReceiver& operator=(const MtcImsEventReceiver&) = delete;

    /**
     * Gets cached WParam value for the given event.
     *
     * @param nEvent Event to inspect.
     * @return Stored parameter of the event.
     *         `UNKNOWN_VALUE` if the event haven't occurred or is not supported.
     */
    IMS_UINT32 GetWParam(IN ImsEvent nEvent);

    /**
     * Gets cached LParam value for the given event.
     *
     * @param nEvent Event to inspect.
     * @return Stored parameter of the event.
     *         `UNKNOWN_VALUE` if the event haven't occurred or is not supported.
     */
    IMS_UINT32 GetLParam(IN ImsEvent nEvent);

    /**
     * Adds a listener for the event. The listener will be notified if the value for the event is
     * changed. A listener can associated to one or more events.
     * The listener must remove itself by calling `RemoveListener()` before destroy.
     * Nothing happens if the event is not supported.
     *
     * @param pListener Listener to be notified.
     * @param nEvent Event to listen.
     */
    void AddListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent);

    /**
     * Removes a listener for the associated events.
     * Nothing happens if the listener is not associated to the event.
     *
     * @param pListener Listener to be removed.
     * @param nEvent Event to be deregistered.
     */
    void RemoveListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent);

    void Event_NotifyEvent(
            IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

private:
    struct EventEntry
    {
        IMSList<IMtcImsEventListener*> lstListeners;
        IMS_UINT32 nWParam = UNKNOWN_VALUE;
        IMS_UINT32 nLParam = UNKNOWN_VALUE;
    };

    void RegisterSupportedEvents();
    void RegisterEvent(IN ImsEvent nEvent);
    void DeregisterEvent(IN ImsEvent nEvent);

    EventEntry* GetEntry(IN ImsEvent nEvent);

    IMS_SINT32 m_nSlotId;
    IMSMap<ImsEvent, EventEntry*> m_objEvents;
};

class IMtcImsEventListener
{
public:
    virtual ~IMtcImsEventListener() {}

    /**
     * Notifies the event. See ImsEventDef.h for the events and corresponding parameters.
     *
     * @param nEvent occurred event.
     * @param nWParam Parameter for the event.
     * @param nLParam Additional parameter for the event.
     */
    virtual void OnImsEventNotified(
            IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) = 0;
};

#endif
