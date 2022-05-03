#ifndef CALL_CONNECTION_ID_MANAGER_H_
#define CALL_CONNECTION_ID_MANAGER_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "IMtcCallStateListener.h"

class IMtcContext;
class IConferenceController;

class CallConnectionIdManager final : public IMtcCallStateListener
{
public:
    explicit CallConnectionIdManager(IN IMtcContext& objContext);
    ~CallConnectionIdManager();
    CallConnectionIdManager(IN const CallConnectionIdManager&) = delete;
    CallConnectionIdManager& operator=(IN const CallConnectionIdManager&) = delete;

    // IMtcCallStateListener interface implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;
    inline IMS_BOOL IsSynchronousCallRequired() override { return IMS_TRUE; }

    void OnConferenceCallStarted(IN IConferenceController* piController, IN IMS_BOOL bStarted);
    void OnConferenceParticipantDisconnected(IN IMS_UINT32 nConnectionId);

    IMS_SINT32 GetIndex(IN CallKey nKey);
    CallKey GetCallKey(IN IMS_UINT32 nConnectionId);

private:
    // TODO: GetNewConnectionId?
    IMS_UINT32 GetNewIndex();
    IMS_SINT32 GetListIndexByCallKey(IN CallKey nCallKey);
    IMS_SINT32 GetListIndexByConnectionId(IN IMS_UINT32 nConnectionId);
    void AddKeyConnectionId(IN CallKey nCallKey);
    void RemoveKeyConnectionId(IN IMS_SINT32 nIndex);
    IMS_BOOL IsConferenceParticipant(IN CallKey nCallKey);
    AString GetIds();

private:
    struct CallKeyConnection
    {
    public:
        inline explicit CallKeyConnection(IN CallKey nKey_, IN IMS_UINT32 nConnectionId_)
        {
            nKey = nKey_;
            nConnectionId = nConnectionId_;
        }
        inline ~CallKeyConnection() {}
        CallKeyConnection(IN const CallKeyConnection&) = delete;
        CallKeyConnection& operator=(IN const CallKeyConnection&) = delete;

    public:
        CallKey nKey;
        IMS_UINT32 nConnectionId;
    };

    IMtcContext& m_objContext;
    IMSList<CallKeyConnection*> m_objCallKeyConnections;
    IMSList<IConferenceController*> m_objControllers;
};

#endif
