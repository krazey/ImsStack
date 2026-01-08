/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MEDIA_H_
#define MEDIA_H_

#include "offeranswer/SdpMediaParameter.h"

#include "ImsCore.h"
#include "media/IMediaState.h"
#include "media/MediaDescriptor.h"

class IOnMediaListener;
class ISdpOaState;
class MediaProposal;
class Service;

class Media : public IMediaState
{
public:
    Media(IN Service* pService, IN ISdpOaState* piOaState);

protected:
    ~Media() override;

public:
    Media(IN const Media&) = delete;
    Media& operator=(IN const Media&) = delete;

public:
    // IMedia interface
    virtual IMS_SINT32 GetType() const = 0;

    inline IMS_SINT32 GetDirection() const { return m_nDirection; }
    inline const ImsList<MediaDescriptor*>& GetMediaDescriptors() const { return m_objDescriptors; }
    MediaProposal* GetProposal() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_SINT32 GetUpdateState() const;
    IMS_RESULT SetDirection(IN IMS_SINT32 nDirection);
    inline void SetMediaListener(IN IOnMediaListener* piListener) { m_piListener = piListener; }
    MediaDescriptor* GetMediaDescriptor() const;
    void RemoveMediaDescriptor(IN IMS_UINT32 nPosition);
    void SetMid(IN IMS_SINT32 nMid);

    IMS_BOOL Equals(IN const Media* pMedia) const;
    inline IMS_BOOL IsDirectionOnlyUpdated() const { return m_bDirectionOnlyUpdated; }
    inline IMS_BOOL IsInitializationDone() const { return m_bInitializationDone; }

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
    void TransitMedia(IN IMS_SINT32 nSessionTransition, IN IMS_SINT32 nOaStatus);

protected:
    // IMediaState interface
    const AString& GetConnectionAddress() const override;
    IMS_SINT32 GetMediaState() const override;
    SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const override;
    const AString& GetPeerConnectionAddress() const override;
    SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const override;
    SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid) override;

    virtual MediaProposal* CreateMediaProposal(IN ISdpOaState* piOaState) = 0;
    inline virtual IMS_BOOL PreviewInitInstance() { return IMS_TRUE; }
    inline virtual IMS_BOOL PostInitInstance() { return IMS_TRUE; }
    inline virtual void PreviewCleanupMedia() {}
    inline virtual void PostCleanupMedia() {}
    inline virtual void PreviewRemoveMedia() {}
    inline virtual void PostRemoveMedia() {}

    IMS_BOOL InitInstance(IN IMS_SINT32 nCountOfDescriptor, IN IMS_SINT32 nDirection);
    IMS_BOOL InitInstance(IN const ImsList<IMS_SINT32>& objMids);
    IMS_BOOL IsMediaAccepted() const;
    IMS_BOOL IsMediaProposed() const;
    inline Service* GetService() const { return m_pService; }
    inline void SetInitializationDone(IN IMS_BOOL bInitializationDone)
    {
        m_bInitializationDone = bInitializationDone;
    }
    void SetState(IN IMS_SINT32 nState);
    void SetUpdateState(IN IMS_SINT32 nState);

private:
    static IMS_SINT32 ConvertDirectionSdpToMedia(IN IMS_SINT32 nDirection);
    static IMS_SINT32 ConvertDirectionMediaToSdp(IN IMS_SINT32 nDirection);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* UpdateStateToString(IN IMS_SINT32 nUpdateState);

public:
    /// Status of Offer / Answer
    enum
    {
        OFFER_SENT = 0,
        OFFER_RECEIVED,
        ANSWER_SENT,
        ANSWER_RECEIVED,
        NO_SDP
    };

    /// Status of session establishment
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

    /// Types of direction (media flow); Refer to IMedia
    enum
    {
        DIRECTION_NONE = (-1),
        DIRECTION_INACTIVE = 0,
        DIRECTION_RECEIVE,
        DIRECTION_SEND,
        DIRECTION_SEND_RECEIVE
    };

    /// Types of main media state; Refer to IMedia interface
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING,
        STATE_ACTIVE,
        STATE_DELETED,
        STATE_PROPOSAL
    };

    /// Types of update state on STATE_ACTIVE state; Refer to IMedia
    enum
    {
        UPDATE_INVALID = 0,
        UPDATE_UNCHANGED = 1,
        UPDATE_MODIFIED,
        UPDATE_REMOVED
    };

    /// Rule types for creation of media descriptor; Refer to IMedia
    enum
    {
        /// Media descriptor will not be created.
        /// It can be used in case the application does not want to send SDP in INVITE request.
        DESCRIPTOR_NONE = (-1),
        /// Media descriptor will be created according to the media profile.
        DESCRIPTOR_FROM_PROFILE = 0
        /// The value greater than 0 means the count of media descriptor
        /// when creating Media object.
        /// This value can be used in Session::CreateMedia(...) method.
    };

private:
    friend class MediaFactory;

    Service* m_pService;
    ISdpOaState* m_piOaState;
    IMS_SINT32 m_nState;
    IMS_SINT32 m_nUpdateState;
    IMS_SINT32 m_nDirection;
    ImsList<MediaDescriptor*> m_objDescriptors;
    IOnMediaListener* m_piListener;
    IMS_BOOL m_bDirectionOnlyUpdated;
    IMS_BOOL m_bInitializationDone;
    IMS_BOOL m_bInitialOfferReceived;
    MediaProposal* m_pMediaProposal;
};

#endif
