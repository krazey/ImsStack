#ifndef _INTERFACE_MEDIA_H_
#define _INTERFACE_MEDIA_H_

#include "media/IMediaDescriptor.h"

class IMediaListener;

/**
 * @brief The IMedia represents a generic media object of a session.
 *      Methods are provided common for all media types.
 *
 * When a calling terminal creates a session, a number of media objects can be added.
 * When the session is started, the IMS engine creates a media offer based on the properties
 * of the included media, and sends it to the remote endpoint.\n
 * If the session is accepted, the remote endpoint has a sufficient amount of information to
 * allow it to create the corresponding media objects. In this way, both endpoints get
 * the same view of the media transfer.\n
 * A common and useful scenario for IMS applications is to stream video and audio
 * to render in real-time.\n
 * To support efficient implementations here, an application can pass the stream to
 * a platform-supplied standard IPlayer that supports an appropriate common codec
 * to do the rendering.\n
 * The IMedia is used to represent the generic concept of a media in an IMS session.
 * Methods are available that address the signalling plane of the media, while other methods
 * support the media plane.
 *
 * While IMedia is a generic media flow-independent interface, the four media types interfaces
 * extend IMedia according to the transport protocol. StreamMedia uses RTP/AVP
 * for the basic profile, and RTP/AVPF also in the "mmtel" profile, FramedMedia uses TCP/MSRP,
 * IBasicUnreliableMedia uses UDP, and IBasicReliableMedia is based on TCP.\n
 * A media object part of a session implements one of the four media type interface.
 * Because of this mapping, each media type has its specific uses.
 */
class IMedia
{
public:
    /**
     * @brief Returns the current direction of this IMedia.
     *
     * @return Current direction.\n
     *         #DIRECTION_INACTIVE\n
     *         #DIRECTION_RECEIVE\n
     *         #DIRECTION_SEND\n
     *         #DIRECTION_SEND_RECEIVE
     */
    virtual IMS_SINT32 GetDirection() const = 0;

    /**
     * @brief Returns the media descriptor(s) associated with this IMedia.
     *
     * @return List of pointer to IMediaDescriptor.
     */
    virtual IMSList<IMediaDescriptor*> GetMediaDescriptors() const = 0;

    /**
     * @brief Returns a fictitious media that is only meant to track changes that are about
     *        to be made to the media.
     *
     * After the ISession has been accepted or rejected, this proposed media should be
     * considered discarded.
     *
     * @param bIMSExtension Flag to indicate that IMS extension is supported or not
     * @return Pointer to IMedia.
     */
    virtual IMedia* GetProposal(IN IMS_BOOL bIMSExtension = IMS_TRUE) const = 0;

    /**
     * @brief Returns the current state of this IMedia.
     *
     * @return Current state.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE\n
     *         #STATE_DELETED\n
     *         #STATE_PROPOSAL
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Returns the current update state of this IMedia.
     *
     * @return Current update state.\n
     *         #UPDATE_INVALID\n
     *         #UPDATE_UNCHANGED\n
     *         #UPDATE_MODIFIED\n
     *         #UPDATE_REMOVED
     */
    virtual IMS_SINT32 GetUpdateState() const = 0;

    /**
     * @brief Sets the direction of this IMedia.
     *
     * If a IMedia is changed in an established ISession, the application has the responsibility
     * to call Update(...) on the ISession.\n
     * NOTE:
     * If the IMedia is in STATE_ACTIVE, the direction will be set on the proposal media
     * until the ISession has been updated. The proposal media can be retrieved
     * with the GetProposal(...) on the IMedia.
     *
     * @param nDirection Direction of the IMedia\n
     *                   #DIRECTION_INACTIVE\n
     *                   #DIRECTION_RECEIVE\n
     *                   #DIRECTION_SEND\n
     *                   #DIRECTION_SEND_RECEIVE
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) = 0;

    //// IMS extensions

    /**
     * @brief Returns the first media descriptor associated with this IMedia.
     *
     * @return Pointer to IMediaDescriptor.
     */
    virtual IMediaDescriptor* GetMediaDescriptor() const = 0;

    /**
     * @brief Returns the media type of this media.
     *
     * @return Type of the current media (MEDIA_TYPE_xxx in IMSCore.h).
     */
    virtual IMS_SINT32 GetType() const = 0;

    /**
     * @brief Removes the media descriptor in the specified index.
     */
    virtual void RemoveMediaDescriptor(IN IMS_UINT32 nPosition) = 0;

public:
    /// Types of direction (media flow)
    enum
    {
        /// IMS extension - not to specify the direction information in the SDP
        DIRECTION_NONE = (-1),
        /// "inactive" direction, meaning it can't send or receive content
        DIRECTION_INACTIVE = 0,
        /// "recvonly" direction, meaning it can receive content, but not send
        DIRECTION_RECEIVE,
        /// "sendonly" direction, meaning it can send content, but not receive
        DIRECTION_SEND,
        /// "sendrecv" direction, meaning it can send and receive content, bi-directional media
        DIRECTION_SEND_RECEIVE
    };

    /// Types of main media state
    enum
    {
        /// IMedia is created and added to a session.\n
        /// IMedia is not active and there can currently be no content transfer within this IMedia
        STATE_INACTIVE = 1,
        /// IMedia exists in a session and a media offer has been sent to the remote endpoint\n
        /// A new IMedia that is added in an incoming invite/update also resides in this state.\n
        /// IMedia is not active and there can currently be no content transfer within this IMedia
        STATE_PENDING,
        /// IMedia exists in a session and the media offer has been accepted by both parties\n
        /// IMedia is active and content can be transferred within this IMedia
        STATE_ACTIVE,
        /// IMedia has been part of an existing session but is now removed,
        /// or the IMedia has been rejected or deleted by the IMS engine.\n
        /// IMedia is not active and there can currently be no content transfer within this IMedia.
        STATE_DELETED,
        /// IMedia is a fictitous media and is only meant to be inspected to track changes
        /// during a session update.\n
        /// After the session update this IMedia should be considered discarded.
        STATE_PROPOSAL
    };

    /// Types of update state on STATE_ACTIVE state
    enum
    {
        UPDATE_INVALID = 0,
        /// The sub-state specifies that this IMedia is unchanged since session establishment
        /// or the last session update.
        UPDATE_UNCHANGED = 1,
        /// The sub-state specifies that this IMedia is proposed to be modified and must be
        /// negotiated before the modifications can be deployed.\n
        /// The modifications can be traced by inspecting the proposed IMedia
        /// by calling GetProposal().
        UPDATE_MODIFIED,
        /// The sub-state specifies that this IMedia is proposed to be removed from the session
        /// and must be negotiated before removal can be made.
        UPDATE_REMOVED
    };

    //// IMS extensions

    /// Rule types for creation of media descriptor
    enum
    {
        /// Media descriptor will not be created.\n
        /// It can be used in case the application does not want to send SDP in INVITE request.
        DESCRIPTOR_NONE = (-1),
        /// Media descriptor will be created according to the media profile.
        DESCRIPTOR_FROM_PROFILE = 0
        /// The value greater than 0 means the count of media descriptor
        /// when creating Media object.\n
        /// This value can be used in Session::CreateMedia(...) method.
    };
};

#endif  // _INTERFACE_MEDIA_H_
