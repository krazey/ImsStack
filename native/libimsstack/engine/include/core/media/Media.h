/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _MEDIA_H_
#define _MEDIA_H_

#include "offeranswer/SdpMediaParameter.h"
#include "media/IMediaState.h"
#include "media/MediaDescriptor.h"

class Service;
class ISdpOaState;
class MediaProposal;
class IOnMediaListener;

class Media : public IMediaState
{
public:
    Media(IN Service* pService_, IN ISdpOaState* piOAState_);

protected:
    virtual ~Media();

private:
    Media(IN CONST Media& objRHS);
    Media& operator=(IN CONST Media& objRHS);

public:
    // IMedia interface
    virtual IMS_SINT32 GetType() const = 0;

    IMS_SINT32 GetDirection() const;
    const IMSList<MediaDescriptor*>& GetMediaDescriptors() const;
    MediaProposal* GetProposal(IN IMS_BOOL bIMSExtension = IMS_TRUE) const;
    IMS_SINT32 GetState() const;
    IMS_SINT32 GetUpdateState() const;
    IMS_RESULT SetDirection(IN IMS_SINT32 nDirection);
    void SetMediaListener(IN IOnMediaListener* piListener);
    //// IMS extensions
    MediaDescriptor* GetMediaDescriptor() const;
    void RemoveMediaDescriptor(IN IMS_UINT32 nPosition);
    void SetMid(IN IMS_SINT32 nMid);

    IMS_BOOL Equals(IN Media* pMedia) const;
    IMS_BOOL IsDirectionOnlyUpdated() const;
    IMS_BOOL IsInitializationDone() const;

    // When an error occurred in Session handling, Session will invoke this method.
    // It will cleanup all the resources which assign to this media.
    void CleanupMedia();
    // Inside of RemoveMedia() in Session, Session will invoke this method.
    // It will set the media port to zero.
    void RemoveMedia();
    // After SessionUpdate has completed and if the session is not negotiated,
    // then it will restore the media state to the previous state.
    void RestoreMedia();
    // Notify the session transition to the media
    void TransitMedia(IN IMS_SINT32 nSessionTransition, IN IMS_SINT32 nOAStatus);

protected:
    // IMediaState interface
    virtual const AString& GetConnectionAddress() const;
    virtual IMS_SINT32 GetMediaState() const;
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const;
    virtual const AString& GetPeerConnectionAddress() const;
    virtual SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const;
    virtual SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid);

    virtual MediaProposal* CreateMediaProposal(IN ISdpOaState* piOAState) = 0;
    virtual IMS_BOOL PreviewInitInstance();
    virtual IMS_BOOL PostInitInstance();
    virtual void PreviewCleanupMedia();
    virtual void PostCleanupMedia();
    virtual void PreviewRemoveMedia();
    virtual void PostRemoveMedia();

    IMS_BOOL InitInstance(IN IMS_SINT32 nCountOfDescriptor, IN IMS_SINT32 nDirection);
    IMS_BOOL InitInstance(IN CONST IMSList<IMS_SINT32>& objMids);
    IMS_BOOL IsMediaAccepted() const;
    IMS_BOOL IsMediaProposed() const;
    Service* GetService() const;
    void SetInitializationDone(IN IMS_BOOL bInitializationDone);
    void SetState(IN IMS_SINT32 nState);
    void SetUpdateState(IN IMS_SINT32 nState);

private:
    void UpdateMediaDescriptors();

    static IMS_SINT32 ConvertDirectionSDPToMedia(IN IMS_SINT32 nDirection);
    static IMS_SINT32 ConvertDirectionMediaToSDP(IN IMS_SINT32 nDirection);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* UpdateStateToString(IN IMS_SINT32 nUpdateState);

public:
    // Status of Offer / Answer
    enum
    {
        OFFER_SENT = 0,
        OFFER_RECEIVED,
        ANSWER_SENT,
        ANSWER_RECEIVED,
        NO_SDP
    };

    // Status of session establishment
    enum
    {
        SESSION_START = 0,
        SESSION_STARTED,
        SESSION_START_FAILED,

        SESSION_EARLY_UPDATE,

        SESSION_UPDATE,
        SESSION_UPDATED,
        SESSION_UPDATE_FAILED,

        SESSION_TERMINATED
    };

    // Types of direction (media flow); Refer to IMedia
    enum
    {
        DIRECTION_NONE = (-1),
        DIRECTION_INACTIVE = 0,
        DIRECTION_RECEIVE,
        DIRECTION_SEND,
        DIRECTION_SEND_RECEIVE
    };

    // Types of main media state; Refer to IMedia interface
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING,
        STATE_ACTIVE,
        STATE_DELETED,
        STATE_PROPOSAL
    };

    // Types of update state on STATE_ACTIVE state; Refer to IMedia
    enum
    {
        UPDATE_INVALID = 0,
        UPDATE_UNCHANGED = 1,
        UPDATE_MODIFIED,
        UPDATE_REMOVED
    };

    //// IMS extensions

    // Rule types for creation of media descriptor; Refer to IMedia
    enum
    {
        // Media descriptor will not be created.
        // It can be used in case the application does not want to send SDP in INVITE request.
        DESCRIPTOR_NONE = (-1),
        // Media descriptor will be created according to the media profile.
        DESCRIPTOR_FROM_PROFILE = 0
        // The value greater than 0 means the count of media descriptor
        // when creating Media object.
        // This value can be used in Session::CreateMedia(...) method.
    };

private:
    friend class MediaFactory;

    Service* pService;
    ISdpOaState* piOAState;

    IMS_SINT32 nState;
    IMS_SINT32 nUpdateState;
    IMS_SINT32 nDirection;
    IMSList<MediaDescriptor*> objDescriptors;
    IOnMediaListener* piListener;

    IMS_BOOL bFlag_DirectionOnlyUpdated;
    IMS_BOOL bFlag_InitializationDone;
    IMS_BOOL bFlag_InitialOfferReceived;
    MediaProposal* pMediaProposal;
};

#endif  // _MEDIA_H_
