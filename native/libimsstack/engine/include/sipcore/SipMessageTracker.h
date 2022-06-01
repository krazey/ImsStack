/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120204  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_MESSAGE_TRACKER_H_
#define _SIP_MESSAGE_TRACKER_H_

#include "ISipMessageTracker.h"

class SIPMessageTracker : public ISipMessageTracker
{
public:
    SIPMessageTracker();
    virtual ~SIPMessageTracker();

private:
    SIPMessageTracker(IN CONST SIPMessageTracker& objRHS);
    SIPMessageTracker& operator=(IN CONST SIPMessageTracker& objRHS);

public:
    IMS_BOOL IsMessageTrackerEnabled() const;

    void NotifyMessageReceived(
            IN CONST SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN CONST AString& strCallId);
    void NotifyMessageSent(IN CONST SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
            IN CONST AString& strCallId, IN IMS_SINT32 nErrorCode = 0);

private:
    // ISipMessageTracker class
    virtual IMS_BOOL AddFilter(
            IN CONST SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing);
    virtual void RemoveFilter(IN CONST SipMethod& objMethod);
    virtual void RemoveFilter(
            IN CONST SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing);
    virtual void RemoveAllFilters();
    virtual void SetListener(IN ISipMessageTrackerListener* piListener);

private:
    class MessageFilter
    {
    public:
        inline MessageFilter() :
                objMethod(SipMethod()),
                nStatusCode(0)
        {
        }
        inline MessageFilter(IN CONST SipMethod& objMethod_, IN IMS_SINT32 nStatusCode_) :
                objMethod(objMethod_),
                nStatusCode(nStatusCode_)
        {
        }
        inline ~MessageFilter() {}

    private:
        MessageFilter(IN CONST MessageFilter& objRHS);
        MessageFilter& operator=(IN CONST MessageFilter& objRHS);

    public:
        inline IMS_BOOL Equals(IN CONST SipMethod& objMethod, IN IMS_SINT32 nStatusCode)
        {
            if (!this->objMethod.Equals(objMethod))
            {
                return IMS_FALSE;
            }

            if (this->nStatusCode != nStatusCode)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        inline const SipMethod& GetMethod() const { return objMethod; }
        inline IMS_SINT32 GetStatusCode() const { return nStatusCode; }

    private:
        SipMethod objMethod;
        IMS_SINT32 nStatusCode;
    };

    IMSList<MessageFilter*> objIncomingFilters;
    IMSList<MessageFilter*> objOutgoingFilters;

    ISipMessageTrackerListener* piListener;
};

#endif  // _SIP_MESSAGE_TRACKER_H_
