/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100614  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RELIABLE_PROV_RESPONSE_HELPER_H_
#define _RELIABLE_PROV_RESPONSE_HELPER_H_

class ReliableProvResponseHelper
{
public:
    explicit ReliableProvResponseHelper(IN IMS_BOOL bIsMobileOriginated_);
    ~ReliableProvResponseHelper();

private:
    ReliableProvResponseHelper(IN CONST ReliableProvResponseHelper& objRHS);
    ReliableProvResponseHelper& operator=(IN CONST ReliableProvResponseHelper& objRHS);

public:
    IMS_SINT32 GetState() const;
    void Initialize(IN ISipMessage* piSIPMsg);
    IMS_BOOL SetRAckHeader(IN_OUT ISipMessage*& piSIPMsg) const;
    IMS_BOOL SetRSeqHeader(IN_OUT ISipMessage*& piSIPMsg) const;
    IMS_BOOL UpdateOnMessageReceived(IN ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnMessageSent(IN ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnOperationFailed();

private:
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_IDLE = 0,

        // UAC behavior
        STATE_RPR_RECEIVED,
        STATE_PRACK_SENT,

        // UAS behavior
        STATE_RPR_SENT,
        STATE_PRACK_RECEIVED,

        STATE_MAX
    };

private:
    IMS_BOOL bIsMobileOriginated;
    IMS_SINT32 nState;

    // The first value will selectes between 1 and 2^31 - 1
    IMS_UINT32 nRSeqNumber;
    // CSeq number
    IMS_UINT32 nCSeqNumber;

    SipMethod objMethod;
};

#endif  // _RELIABLE_PROV_RESPONSE_HELPER_H_
