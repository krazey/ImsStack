#ifndef CONFERENCE_MANAGER_H_
#define CONFERENCE_MANAGER_H_

#include "IMSMap.h"
#include "conferencecall/IConferenceControllerListener.h"
#include "conferencecall/ConferenceController.h"
#include "call/IMtcCallManager.h"
#include "helper/ObjectAsyncDestroyer.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/CallConnectionIdManager.h"

class IMtcContext;
class CallStateProxy;
enum class ConferenceType;

class ConferenceManager final : public IConferenceControllerListener
{
public:
    explicit ConferenceManager(IN IMtcContext& objContext);
    ~ConferenceManager();
    ConferenceManager(IN const ConferenceManager&) = delete;
    ConferenceManager& operator=(IN const ConferenceManager&) = delete;

    // IConferenceControllerListener interface implementation
    void OnClosed(IN ConferenceController* pController) override;

    IConferenceController& CreateController(IN CallKey nCallKey, IN ConferenceType eType);
    IConferenceController* GetController(IN IMS_UINTP nCallKey) const;

private:
    void ReleaseController(IN ConferenceController* pController);

private:
    IMtcContext& m_objContext;
    IMSMap<CallKey, ConferenceController*> m_objControllers;
    ObjectAsyncDestroyer<ConferenceController> m_objDestroyer;
    CallConnectionIdManager m_objCallConnectionIdManager;
};

enum class ConferenceType
{
    PARTICIPANT,
    GROUP_CALL,  // aka. start conference
    MERGE_CALL,
    EXPAND_CALL
};

#endif
