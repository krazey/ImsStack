/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.imsservice.mmtel.call;

import android.os.Message;
import android.os.RemoteException;
import android.telephony.CallQuality;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.ims.aidl.IImsCallSessionListener;
import android.telephony.ims.stub.ImsCallSessionImplBase.State;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsVideoCallProvider;
import com.android.imsstack.util.Log;

import java.util.List;

/**
 * IMS call session interface wrapper. It contains only simple logic to call remote methods of
 * IImsCallSessions object and receives the notification.
 * The users of this class can be notified call events by implement
 * {@link ImsCallSessionWrapper#Listener}.
 */
public final class ImsCallSessionWrapper {
    @NonNull private final IImsCallSession mIImsCallSession;

    /** Constructor. */
    @SuppressWarnings("deprecation") // setListener
    public ImsCallSessionWrapper(@NonNull IImsCallSession callSession,
            @NonNull IImsCallSessionListener listener) {
        mIImsCallSession = callSession;
        try {
            mIImsCallSession.setListener(listener);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Closes the object. This object is not usable after being closed.
     */
    public void close() {
        try {
            mIImsCallSession.close();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Gets the call ID of the session.
     *
     * @return The call ID.
     */
    public @Nullable String getCallId() {
        try {
            return mIImsCallSession.getCallId();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Gets the call profile that this session is associated with.
     *
     * @return The call profile that this session is associated with.
     */
    public @Nullable ImsCallProfile getCallProfile() {
        try {
            return mIImsCallSession.getCallProfile();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Gets the local call profile that this session is associated with.
     *
     * @return The local call profile that this session is associated with.
     */
    public @Nullable ImsCallProfile getLocalCallProfile() {
        try {
            return mIImsCallSession.getLocalCallProfile();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Gets the remote call profile that this session is associated with.
     *
     * @return The remote call profile that this session is associated with.
     */
    public @Nullable ImsCallProfile getRemoteCallProfile() {
        try {
            return mIImsCallSession.getRemoteCallProfile();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Gets the value associated with the specified property of this session.
     *
     * @return The string value associated with the specified property.
     */
    public @Nullable String getProperty(@Nullable String name) {
        try {
            return mIImsCallSession.getProperty(name);
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Gets the session state. The value returned must be one of the states in.
     * {@link ImsCallSessionImplBase#State}.
     *
     * @return The session state.
     */
    public int getState() {
        try {
            return mIImsCallSession.getState();
        } catch (RemoteException e) {
            loge(e.toString());
            return State.INVALID;
        }
    }

    /**
     * Checks if the session is in a call.
     *
     * @return {@code true} if the session is in a call.
     */
    public boolean isInCall() {
        try {
            return mIImsCallSession.isInCall();
        } catch (RemoteException e) {
            loge(e.toString());
            return false;
        }
    }

    /**
     * Mutes or unmutes the mic for the active call.
     *
     * @param muted {@code true} if the call is muted, {@code false} otherwise.
     */
    public void setMute(boolean muted) {
        try {
            mIImsCallSession.setMute(muted);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Initiates an IMS call with the specified target and call profile.
     * The session listener is called back upon defined session events.
     * The method is only valid to call when the session state is in IDLE.
     *
     * @param callee Dialed string to make the call to.
     * @param profile Call profile to make the call with the specified service type,
     *                call type and media information.
     * @see Listener#callSessionStarted
     * @see Listener#callSessionStartFailed
     */
    public void start(@Nullable String callee, @Nullable ImsCallProfile profile) {
        try {
            mIImsCallSession.start(callee, profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Initiates an IMS call with the specified participants and call profile.
     * The session listener is called back upon defined session events.
     * The method is only valid to call when the session state is in IDLE.
     *
     * @param participants Participant list to initiate an IMS conference call.
     * @param profile Call profile to make the call with the specified service type,
     *                call type and media information.
     * @see Listener#callSessionStarted
     * @see Listener#callSessionStartFailed
     */
    public void startConference(String[] participants, @Nullable ImsCallProfile profile) {
        try {
            mIImsCallSession.startConference(participants, profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Accepts an incoming call or session update.
     *
     * @param callType Call type specified in {@link ImsCallProfile} to be answered.
     * @param profile Stream media profile {@link ImsStreamMediaProfile} to be answered.
     * @see Listener#callSessionStarted
     */
    public void accept(int callType, @Nullable ImsStreamMediaProfile profile) {
        try {
            mIImsCallSession.accept(callType, profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Deflects an incoming call.
     *
     * @param deflectNumber Number to deflect the call.
     */
    public void deflect(@Nullable String deflectNumber) {
        try {
            mIImsCallSession.deflect(deflectNumber);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Rejects an incoming call or session update.
     *
     * @param reason Reason code to reject an incoming call.
     * @see Listener#callSessionStartFailed
     */
    public void reject(int reason) {
        try {
            mIImsCallSession.reject(reason);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Transfers an established call to given number.
     *
     * @param number Number to transfer the call.
     * @param isConfirmationRequired If {@code true}, indicates a confirmed transfer,
     *                               if {@code false} it indicates an unconfirmed transfer.
     */
    public void transfer(@Nullable String number, boolean isConfirmationRequired) {
        try {
            mIImsCallSession.transfer(number, isConfirmationRequired);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Transfers an established call to another call session.
     *
     * @param transferToSession The other ImsCallSession to transfer the ongoing session to.
     */
    public void consultativeTransfer(@Nullable IImsCallSession transferToSession) {
        try {
            mIImsCallSession.consultativeTransfer(transferToSession);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Terminates a call.
     *
     * @see Listener#callSessionTerminated
     */
    public void terminate(int reason) {
        try {
            mIImsCallSession.terminate(reason);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Puts a call on hold. When it succeeds, {@link Listener#callSessionHeld} is called.
     *
     * @param profile Stream media profile {@link ImsStreamMediaProfile} to hold the call.
     * @see Listener#callSessionHeld
     * @see Listener#callSessionHoldFailed
     */
    public void hold(@Nullable ImsStreamMediaProfile profile) {
        try {
            mIImsCallSession.hold(profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Continues a call that's on hold. When it succeeds, {@link Listener#callSessionResumed}
     * is called.
     *
     * @param profile Stream media profile {@link ImsStreamMediaProfile} to resume the call.
     * @see Listener#callSessionResumed
     * @see Listener#callSessionResumeFailed
     */
    public void resume(@Nullable ImsStreamMediaProfile profile) {
        try {
            mIImsCallSession.resume(profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Merges the active & hold call. When the merge starts,
     * {@link Listener#callSessionMergeStarted} is called.
     * {@link Listener#callSessionMergeComplete} is called if the merge is successful, and
     * {@link Listener#callSessionMergeFailed} is called if the merge fails.
     *
     * @see Listener#callSessionMergeStarted
     * @see Listener#callSessionMergeComplete
     * @see Listener#callSessionMergeFailed
     */
    public void merge() {
        try {
            mIImsCallSession.merge();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Updates the current call's properties (ex. call mode change: video upgrade / downgrade).
     *
     * @param callType Call type specified in {@link ImsCallProfile} to be updated.
     * @param profile Stream media profile {@link ImsStreamMediaProfile} to be updated.
     * @see Listener#callSessionUpdated
     * @see Listener#callSessionUpdateFailed
     */
    public void update(int callType, @Nullable ImsStreamMediaProfile profile) {
        try {
            mIImsCallSession.update(callType, profile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Extends this call to the conference call with the specified recipients.
     *
     * @param participants Participant list to be invited to the conference call after extending the
     *                     call.
     * @see Listener#sessionConferenceExtended
     * @see Listener#sessionConferenceExtendFailed
     */
    public void extendToConference(String[] participants) {
        try {
            mIImsCallSession.extendToConference(participants);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Requests the conference server to invite additional participants to the conference.
     *
     * @param participants Participant list to be invited to the conference call.
     * @see Listener#sessionInviteParticipantsRequestDelivered
     * @see Listener#sessionInviteParticipantsRequestFailed
     */
    public void inviteParticipants(String[] participants) {
        try {
            mIImsCallSession.inviteParticipants(participants);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Requests the conference server to remove the specified participants from the conference.
     *
     * @param participants Participant list to be removed from the conference call.
     * @see Listener#sessionRemoveParticipantsRequestDelivered
     * @see Listener#sessionRemoveParticipantsRequestFailed
     */
    public void removeParticipants(String[] participants) {
        try {
            mIImsCallSession.removeParticipants(participants);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Sends a DTMF code. According to <a href="http://tools.ietf.org/html/rfc2833">RFC 2833</a>,
     * event 0 ~ 9 maps to decimal value 0 ~ 9, '*' to 10, '#' to 11, event 'A' ~ 'D' to 12 ~ 15,
     * and event flash to 16. Currently, event flash is not supported.
     *
     * @param c The DTMF to send. '0' ~ '9', 'A' ~ 'D', '*', '#' are valid inputs.
     * @param result If non-null, the {@link Message} to send when the operation is completed. This
     *               is done by using the associated {@link android.os.Messenger} in
     *               {@link Message#replyTo}.
     */
    public void sendDtmf(char c, @Nullable Message result) {
        try {
            mIImsCallSession.sendDtmf(c, result);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Starts a DTMF code. According to <a href="http://tools.ietf.org/html/rfc2833">RFC 2833</a>,
     * event 0 ~ 9 maps to decimal value 0 ~ 9, '*' to 10, '#' to 11, event 'A' ~ 'D' to 12 ~ 15,
     * and event flash to 16. Currently, event flash is not supported.
     *
     * @param c The DTMF to send. '0' ~ '9', 'A' ~ 'D', '*', '#' are valid inputs.
     */
    public void startDtmf(char c) {
        try {
            mIImsCallSession.startDtmf(c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Stops a DTMF code.
     */
    public void stopDtmf() {
        try {
            mIImsCallSession.stopDtmf();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Sends an USSD message.
     *
     * @param ussdMessage USSD message to send.
     */
    public void sendUssd(@Nullable String ussdMessage) {
        try {
            mIImsCallSession.sendUssd(ussdMessage);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Returns a binder for the video call provider implementation contained within the IMS service
     * process. This binder is used by the VideoCallProvider subclass in Telephony which
     * intermediates between the propriety implementation and Telecomm/InCall.
     */
    public @Nullable IImsVideoCallProvider getVideoCallProvider() {
        try {
            return mIImsCallSession.getVideoCallProvider();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Determines if the current session is multiparty.
     *
     * @return {@code true} if the session is multiparty.
     */
    public boolean isMultiparty() {
        try {
            return mIImsCallSession.isMultiparty();
        } catch (RemoteException e) {
            loge(e.toString());
            return false;
        }
    }

    /**
     * Issues a RTT modify request.
     *
     * @param toProfile The profile with requested changes made.
     */
    public void sendRttModifyRequest(@Nullable ImsCallProfile toProfile) {
        try {
            mIImsCallSession.sendRttModifyRequest(toProfile);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Responds to the remote RTT modify request.
     *
     * @param status {@code true}: Accepts the request.
     *               {@code false}: Declines the request.
     */
    public void sendRttModifyResponse(boolean status) {
        try {
            mIImsCallSession.sendRttModifyResponse(status);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Sends a RTT message.
     *
     * @param rttMessage RTT message to be sent.
     */
    public void sendRttMessage(@Nullable String rttMessage) {
        try {
            mIImsCallSession.sendRttMessage(rttMessage);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Sends RTP header extension(s).
     *
     * @param extensions The header extensions to be sent.
     */
    public void sendRtpHeaderExtensions(@Nullable List<RtpHeaderExtension> extensions) {
        try {
            mIImsCallSession.sendRtpHeaderExtensions(extensions);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Delivers the bitrate for the indicated media type, direction and bitrate to the upper layer.
     *
     * @param mediaType MediaType is used to identify media stream such as audio or video.
     * @param direction Direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate received from the NW through the Recommended
     *        bitrate MAC Control Element message and ImsStack converts this value from MAC bitrate
     *        to audio/video codec bitrate (defined in TS26.114).
     */
    public void callSessionNotifyAnbr(int mediaType, int direction, int bitsPerSecond) {
        try {
            mIImsCallSession.callSessionNotifyAnbr(mediaType, direction, bitsPerSecond);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "ImsCallSessionWrapper: " + s);
    }

    /**
     * A class implementing the IImsCallSessionListener interface to handle IMS call session events.
     * This class provides default (empty) implementations of the interface.
     */
    public static class ImsCallSessionListener extends IImsCallSessionListener.Stub {
        /**
         * Called when a call session is initiating.
         *
         * @param profile The call profile {@link ImsCallProfile} containing information about the
         *                call session.
         */
        @Override
        public void callSessionInitiating(ImsCallProfile profile) {}

        /**
         * Called when a call session is in progress.
         *
         * @param profile The media profile {@link ImsCallProfile} containing information about the
         *                call session.
         */
        @Override
        public void callSessionProgressing(ImsStreamMediaProfile profile) {}

        /**
         * Called when a call session is successfully initiated.
         *
         * @param profile The call profile {@link ImsCallProfile} containing information about the
         *                call session.
         */
        @Override
        public void callSessionInitiated(ImsCallProfile profile) {}

        /**
         * Called when initiating a call session fails.
         *
         * @param reasonInfo Information about the reason {@link ImsReasonInfo} for the failure.
         */
        @Override
        public void callSessionInitiatingFailed(ImsReasonInfo reasonInfo) {}

        /**
         * Called when the initiation of a call session fails after it has been initiated.
         *
         * @param reasonInfo Information about the reason {@link ImsReasonInfo} for the failure.
         */
        @Override
        public void callSessionInitiatedFailed(ImsReasonInfo reasonInfo) {}

        /**
         * Called when a call session is terminated.
         *
         * @param reasonInfo Information about the reason {@link ImsReasonInfo} for the termination.
         */
        @Override
        public void callSessionTerminated(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionHeld(ImsCallProfile profile) {}

        @Override
        public void callSessionHoldFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionHoldReceived(ImsCallProfile profile) {}

        @Override
        public void callSessionResumed(ImsCallProfile profile) {}

        @Override
        public void callSessionResumeFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionResumeReceived(ImsCallProfile profile) {}

        @Override
        public void callSessionMergeStarted(IImsCallSession newSession, ImsCallProfile profile) {}

        @Override
        public void callSessionMergeComplete(IImsCallSession newSession) {}

        @Override
        public void callSessionMergeFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionUpdated(ImsCallProfile profile) {}

        @Override
        public void callSessionUpdateFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionUpdateReceived(ImsCallProfile profile) {}

        @Override
        public void callSessionConferenceExtended(IImsCallSession newSession,
                ImsCallProfile profile) {}

        @Override
        public void callSessionConferenceExtendFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionConferenceExtendReceived(IImsCallSession newSession,
                ImsCallProfile profile) {}

        @Override
        public void callSessionInviteParticipantsRequestDelivered() {}

        @Override
        public void callSessionInviteParticipantsRequestFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionRemoveParticipantsRequestDelivered() {}

        @Override
        public void callSessionRemoveParticipantsRequestFailed(ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionConferenceStateUpdated(ImsConferenceState state) {}

        /**
         * Called when the a call session has received a USSD message.
         *
         * @param mode The mode of the USSD message, either
         *             {@link ImsCallSessionImplBase#USSD_MODE_NOTIFY} or
         *             {@link ImsCallSessionImplBase#USSD_MODE_REQUEST}.
         * @param ussdMessage The USSD message.
         */
        @Override
        public void callSessionUssdMessageReceived(int mode, String ussdMessage) {}

        @Override
        public void callSessionMayHandover(int srcNetworkType, int targetNetworkType) {}

        @Override
        public void callSessionHandover(int srcNetworkType, int targetNetworkType,
                ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionHandoverFailed(int srcNetworkType, int targetNetworkType,
                ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionTtyModeReceived(int mode) {}

        public void callSessionMultipartyStateChanged(boolean isMultiParty) {}

        @Override
        public void callSessionSuppServiceReceived(ImsSuppServiceNotification suppServiceInfo) {}

        @Override
        public void callSessionRttModifyRequestReceived(ImsCallProfile callProfile) {}

        @Override
        public void callSessionRttModifyResponseReceived(int status) {}

        @Override
        public void callSessionRttMessageReceived(String rttMessage) {}

        @Override
        public void callSessionRttAudioIndicatorChanged(ImsStreamMediaProfile profile) {}

        @Override
        public void callSessionTransferred() {}

        @Override
        public void callSessionTransferFailed(@Nullable ImsReasonInfo reasonInfo) {}

        @Override
        public void callSessionDtmfReceived(char dtmf) {}

        @Override
        public void callQualityChanged(CallQuality callQuality) {}

        @Override
        public void callSessionRtpHeaderExtensionsReceived(
                @NonNull List<RtpHeaderExtension> extensions) {}

        @Override
        public void callSessionSendAnbrQuery(int mediaType, int direction, int bitsPerSecond) {}
    }
}
