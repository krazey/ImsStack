#ifndef INTERFACE_CONFERENCE_CONTROLLER_H_
#define INTERFACE_CONFERENCE_CONTROLLER_H_

class ConferenceParticipantList;

enum class IndividualCallState
{
    IDLE,
    JOINING,
    JOINED,
    INVITED
};

class IConferenceController
{
public:
enum /*class Command*/
{
    GROUPCALL,
    MERGE,
    EXPAND,
    ADD,
    REMOVE,
    JOINED
};
    virtual ~IConferenceController() {}
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<IMS_UINT32, SuppService*>& objSuppServices) = 0;
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers) = 0;
    virtual IMS_SINT32 GetState() const = 0;
    virtual IndividualCallState GetCallStatusInConference(IN CallKey nKey) const = 0;
};

#endif
