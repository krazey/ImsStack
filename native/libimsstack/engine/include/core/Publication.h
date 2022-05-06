/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100423  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _PUBLICATION_H_
#define _PUBLICATION_H_

#include "ServiceMethod.h"
#include "util/IRefreshable.h"

class IRefreshListener;
class IOnPublicationListener;
class PubState;
class PublicationRefreshHelper;

class Publication : public ServiceMethod, public IRefreshable
{
public:
    Publication(IN Service* pService, IN CONST AString& strEvent_);
    virtual ~Publication();

private:
    Publication(IN CONST Publication& objRHS);
    Publication& operator=(IN CONST Publication& objRHS);

public:
    // Method class
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // IPublication interface
    const AString& GetEvent() const;
    IMS_SINT32 GetState() const;
    IMS_RESULT Publish(IN CONST ByteArray& objState, IN CONST AString& strContentType);
    void SetListener(IN IOnPublicationListener* piListener);
    IMS_RESULT Unpublish();

    //// IMS extensions
    void SetRefreshListener(IN IRefreshListener* piListener);
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Handle the exceptions
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL InitInstance();

    // Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IRefreshable interface
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual IMS_BOOL Refreshable_RefreshStarted();
    virtual void Refreshable_RefreshTerminated();

private:
    void CloseConnection();
    void ReceiveResponse(IN ISipClientConnection* piSCC);
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to IPublication interface
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    // Policy for publication refresh
    enum
    {
        // No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        // Default policy; Select the refresh time according to 3GPP spec.
        //     nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100; default 50)
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        // Set the remain time before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Interval value when the refresh duration is equal or less
        //              than the criteria interval
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        // Set the ratio before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100)
        //    nValueGT : Ratio when the refresh duration is greater
        //              than the criteria interval (1 ~ 100)
        // Ex) Expires: 3600, Ratio: 10
        //        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

protected:
    enum
    {
        AMSG_PUBLICATION_DELIVERED = AMSG_USER,
        AMSG_PUBLICATION_DELIVERY_FAILED,
        AMSG_PUBLICATION_TERMINATED,
        //[2012/11/5]hyunho.shin : add event about refresh
        AMSG_PUBLICATION_REFRESH_STARTED,
        AMSG_PUBLICATION_REFRESH_COMPLETED,
        //[2012/11/5]hyunho.shin : end
        AMSG_PUBLICATION_MAX
    };

private:
    // State of Publication
    IMS_SINT32 nState;
    // Event package name
    AString strEvent;
    // Listener for this publication
    IOnPublicationListener* piListener;

    // Publication state information
    PubState* pPubState;
    // Publication refresh timer
    IRefreshListener* piRefreshListener;
    PublicationRefreshHelper* pRefreshHelper;
};

#endif  // _PUBLICATION_H_
