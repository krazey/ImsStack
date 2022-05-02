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



class SIPMessageTracker
    : public ISIPMessageTracker
{
public:
    SIPMessageTracker();
    virtual ~SIPMessageTracker();

private:
    SIPMessageTracker(IN CONST SIPMessageTracker &objRHS);
    SIPMessageTracker& operator=(IN CONST SIPMessageTracker &objRHS);

public:
    IMS_BOOL IsMessageTrackerEnabled() const;

    void NotifyMessageReceived(IN CONST SIPMethod &objMethod,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId);
    void NotifyMessageSent(IN CONST SIPMethod &objMethod,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId, IN IMS_SINT32 nErrorCode = 0);

private:
    // ISIPMessageTracker class
    virtual IMS_BOOL AddFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing);
    virtual void RemoveFilter(IN CONST SIPMethod &objMethod);
    virtual void RemoveFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing);
    virtual void RemoveAllFilters();
    virtual void SetListener(IN ISIPMessageTrackerListener *piListener);

private:
    class MessageFilter
    {
    public:
        inline MessageFilter()
            : objMethod(SIPMethod())
            , nStatusCode(0)
        {}
        inline MessageFilter(IN CONST SIPMethod &objMethod_, IN IMS_SINT32 nStatusCode_)
            : objMethod(objMethod_)
            , nStatusCode(nStatusCode_)
        {}
        inline ~MessageFilter()
        {}

    private:
        MessageFilter(IN CONST MessageFilter &objRHS);
        MessageFilter& operator=(IN CONST MessageFilter &objRHS);

    public:
        inline IMS_BOOL Equals(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode)
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

        inline const SIPMethod& GetMethod() const
        { return objMethod; }
        inline IMS_SINT32 GetStatusCode() const
        { return nStatusCode; }

    private:
        SIPMethod objMethod;
        IMS_SINT32 nStatusCode;
    };

    IMSList<MessageFilter*> objIncomingFilters;
    IMSList<MessageFilter*> objOutgoingFilters;

    ISIPMessageTrackerListener *piListener;
};

#endif // _SIP_MESSAGE_TRACKER_H_
