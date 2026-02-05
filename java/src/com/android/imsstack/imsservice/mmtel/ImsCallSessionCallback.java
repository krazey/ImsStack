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

package com.android.imsstack.imsservice.mmtel;

import android.os.DeadObjectException;
import android.telephony.CallQuality;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.ims.stub.ImsCallSessionImplBase;

import com.android.imsstack.util.ImsLog;

import java.util.Set;

public class ImsCallSessionCallback {
    private ImsCallSessionListener mListener = null;

    public ImsCallSessionCallback() {
    }

    public boolean hasListener() {
        return (mListener != null);
    }

    public void setListener(ImsCallSessionListener listener) {
        mListener = listener;
    }

    public void invokeInitiating(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            logi("invokeInitiating :: " + profile);

            mListener.callSessionInitiating(profile);
        } catch (Throwable t) {
            log(t, "invokeInitiating");
            closeSession(session, t);
        }
    }

    public void invokeProgressing(final ImsCallSessionImplBase session,
            final ImsStreamMediaProfile mediaProfile) {
        try {
            if (mListener == null) {
                return;
            }

            logi("invokeProgressing :: " + mediaProfile);

            mListener.callSessionProgressing(mediaProfile);
        } catch (Throwable t) {
            log(t, "invokeProgressing");
            closeSession(session, t);
        }
    }

    public void invokeStarted(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            logi("invokeStarted :: " + profile.getMediaProfile());

            mListener.callSessionInitiated(profile);
        } catch (Throwable t) {
            log(t, "invokeStarted");
            closeSession(session, t);
        }
    }

    public void invokeStartFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            logi("invokeStartFailed :: " + reasonInfo);

            mListener.callSessionInitiatingFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeStartFailed");
            closeSession(session, t);
        }
    }

    public void invokeTerminated(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        if (mListener == null) {
            return;
        }

        try {
            logi("invokeTerminated :: " + reasonInfo);
            mListener.callSessionTerminated(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeTerminated");
            closeSession(session, t);
        }
    }

    public void invokeHeld(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            logi("invokeHeld :: " + profile);

            mListener.callSessionHeld(profile);
        } catch (Throwable t) {
            log(t, "invokeHeld");
            closeSession(session, t);
        }
    }

    public void invokeHoldFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeHoldFailed :: " + reasonInfo);

            mListener.callSessionHoldFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeHoldFailed");
            closeSession(session, t);
        }
    }

    public void invokeHoldReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeHoldReceived :: " + profile);

            mListener.callSessionHoldReceived(profile);
        } catch (Throwable t) {
            log(t, "invokeHoldReceived");
            closeSession(session, t);
        }
    }

    public void invokeResumed(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeResumed :: " + profile);

            mListener.callSessionResumed(profile);
        } catch (Throwable t) {
            log(t, "invokeResumed");
            closeSession(session, t);
        }
    }

    public void invokeResumeFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeResumeFailed :: " + reasonInfo);

            mListener.callSessionResumeFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeResumeFailed");
            closeSession(session, t);
        }
    }

    public void invokeResumeReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeResumeReceived :: " + profile);

            mListener.callSessionResumeReceived(profile);
        } catch (Throwable t) {
            log(t, "invokeResumeReceived");
            closeSession(session, t);
        }
    }

    public void invokeMergeStarted(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeMergeStarted :: session=" + session
                    + ", confSession=" + confSession);

            mListener.callSessionMergeStarted(confSession, profile);
        } catch (Throwable t) {
            log(t, "invokeMergeStarted");
            closeSession(session, t);
        }
    }

    public void invokeMergeComplete(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase newSession) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeMergeComplete :: session="
                    + ((newSession != null) ? newSession : session));

            mListener.callSessionMergeComplete(newSession);
        } catch (Throwable t) {
            log(t, "invokeMergeComplete");
            closeSession(session, t);

            if (newSession != null) {
                closeSession(newSession, t);
            }
        }
    }

    public void invokeMergeFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeMergeFailed :: " + reasonInfo);

            mListener.callSessionMergeFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeMergeFailed");
            closeSession(session, t);
        }
    }

    public void invokeUpdated(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            // This is for removing the timing issue
            // to update the call extras for conference session.
            if (mListener == null) {
                log("invokeUpdated :: no listener");
                return;
            }

            log("callSessionUpdated :: " + profile);

            mListener.callSessionUpdated(profile);
        } catch (Throwable t) {
            log(t, "invokeUpdated");
            closeSession(session, t);
        }
    }

    public void invokeUpdateFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeUpdateFailed :: " + reasonInfo);

            mListener.callSessionUpdateFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeUpdateFailed");
            closeSession(session, t);
        }
    }

    public void invokeUpdateReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeUpdateReceived :: " + profile);

            mListener.callSessionUpdateReceived(profile);
        } catch (Throwable t) {
            log(t, "invokeUpdateReceived");
            closeSession(session, t);
        }
    }

    public void invokeConferenceExtended(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeConferenceExtended :: " + session);

            mListener.callSessionConferenceExtended(confSession, profile);
        } catch (Throwable t) {
            log(t, "invokeConferenceExtended");
            closeSession(session, t);

            if (confSession != null) {
                closeSession(confSession, t);
            }
        }
    }

    public void invokeConferenceExtendFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeConferenceExtendFailed :: " + reasonInfo);

            mListener.callSessionConferenceExtendFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeConferenceExtendFailed");
            closeSession(session, t);
        }
    }

    public void invokeConferenceExtendReceived(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeConferenceExtendReceived :: " + session);

            mListener.callSessionConferenceExtendReceived(confSession, profile);
        } catch (Throwable t) {
            log(t, "invokeConferenceExtendReceived");
            closeSession(session, t);

            if (confSession != null) {
                closeSession(confSession, t);
            }
        }
    }

    public void invokeInviteParticipantsRequestDelivered(final ImsCallSessionImplBase session) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeInviteParticipantsRequestDelivered :: " + session);

            mListener.callSessionInviteParticipantsRequestDelivered();
        } catch (Throwable t) {
            log(t, "invokeInviteParticipantsRequestDelivered");
            closeSession(session, t);
        }
    }

    public void invokeInviteParticipantsRequestFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeInviteParticipantsRequestFailed :: " + reasonInfo);

            mListener.callSessionInviteParticipantsRequestFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeInviteParticipantsRequestFailed");
            closeSession(session, t);
        }
    }

    public void invokeRemoveParticipantsRequestDelivered(final ImsCallSessionImplBase session) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRemoveParticipantsRequestDelivered :: " + session);

            mListener.callSessionRemoveParticipantsRequestDelivered();
        } catch (Throwable t) {
            log(t, "invokeRemoveParticipantsRequestDelivered");
            closeSession(session, t);
        }
    }

    public void invokeRemoveParticipantsRequestFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRemoveParticipantsRequestFailed :: " + reasonInfo);

            mListener.callSessionRemoveParticipantsRequestFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeRemoveParticipantsRequestFailed");
            closeSession(session, t);
        }
    }

    public void invokeConferenceStateUpdated(final ImsCallSessionImplBase session,
            final ImsConferenceState confState) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeConferenceStateUpdated :: " + confState);

            mListener.callSessionConferenceStateUpdated(confState);
        } catch (Throwable t) {
            log(t, "invokeConferenceStateUpdated");
            closeSession(session, t);
        }
    }

    public void invokeUssdMessageReceived(final ImsCallSessionImplBase session,
            final int mode, final String ussdMessage) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeUssdMessageReceived :: " + ussdMessage);

            mListener.callSessionUssdMessageReceived(mode, ussdMessage);
        } catch (Throwable t) {
            log(t, "invokeUssdMessageReceived");
            closeSession(session, t);
        }
    }

    public void invokeMultipartyStateChanged(final ImsCallSessionImplBase session,
            final boolean isMultiparty) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeMultipartyStateChanged :: " + isMultiparty);

            mListener.callSessionMultipartyStateChanged(isMultiparty);
        } catch (Throwable t) {
            log(t, "invokeMultipartyStateChanged");
            closeSession(session, t);
        }
    }

    public void invokeCallSessionTransferred(final ImsCallSessionImplBase session) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeCallSessionTransferred :: " + session);

            mListener.callSessionTransferred();
        } catch (Throwable t) {
            log(t, "invokeCallSessionTransferred");
            closeSession(session, t);
        }
    }

    public void invokeCallSessionTransferFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeCallSessionTransferFailed :: " + reasonInfo);

            mListener.callSessionTransferFailed(reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeCallSessionTransferFailed");
            closeSession(session, t);
        }
    }

    // @QUALCOMM_API
    public void invokeHandover(final ImsCallSessionImplBase session,
            final int srcAccessTech, final int targetAccessTech) {
        try {
            if (mListener == null) {
                return;
            }

            ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED,
                    ImsReasonInfo.CODE_UNSPECIFIED, null);

            mListener.onHandover(
                    srcAccessTech, targetAccessTech, reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeHandover");
            closeSession(session, t);
        }
    }

    // @QUALCOMM_API
    public void invokeHandoverFailed(final ImsCallSessionImplBase session,
            final int srcAccessTech, final int targetAccessTech,
            final ImsReasonInfo reasonInfo) {
        try {
            if (mListener == null) {
                return;
            }

            mListener.onHandoverFailed(
                    srcAccessTech, targetAccessTech, reasonInfo);
        } catch (Throwable t) {
            log(t, "invokeHandoverFailed");
            closeSession(session, t);
        }
    }

    // @QUALCOMM_API
    public void invokeTtyModeReceived(final ImsCallSessionImplBase session,
            final int mode) {
        try {
            if (mListener == null) {
                return;
            }

            mListener.callSessionTtyModeReceived(mode);
        } catch (Throwable t) {
            log(t, "invokeTtyModeReceived");
            closeSession(session, t);
        }
    }

    // @QUALCOMM_API
    public void invokeDeflected(final ImsCallSessionImplBase session) {
    }

    // @QUALCOMM_API
    public void invokeDeflectFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
    }

    // @QUALCOMM_API
    public void invokeSuppServiceReceived(final ImsCallSessionImplBase session,
            final ImsSuppServiceNotification issn) {

        try {
            if (mListener == null) {
                return;
            }

            mListener.callSessionSuppServiceReceived(issn);
        } catch (Throwable t) {
            log(t, "invokeSuppServiceReceived");
            closeSession(session, t);
        }
    }

    public void invokeRttModifyRequestReceived(final ImsCallSessionImplBase session,
            ImsCallProfile callProfile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRttModifyRequestReceived :: " + callProfile);

            mListener.callSessionRttModifyRequestReceived(callProfile);
        } catch (Throwable t) {
            log(t, "invokeRttModifyRequestReceived");
            closeSession(session, t);
        }
    }

    public void invokeRttModifyResponseReceived(final ImsCallSessionImplBase session,
            final int status) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRttModifyResponseReceived :: " + status);

            mListener.callSessionRttModifyResponseReceived(status);
        } catch (Throwable t) {
            log(t, "invokeRttModifyResponseReceived");
            closeSession(session, t);
        }
    }

    public void invokeRttMessageReceived(final ImsCallSessionImplBase session,
            final String data) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRttMessageReceived");

            mListener.callSessionRttMessageReceived(data);
        } catch (Throwable t) {
            log(t, "invokeRttMessageReceived");
            closeSession(session, t);
        }
    }

    public void invokeRttAudioIndicatorChanged(final ImsCallSessionImplBase session,
            final ImsStreamMediaProfile profile) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRttAudioIndicatorChanged :: " + profile);

            mListener.callSessionRttAudioIndicatorChanged(profile);
        } catch (Throwable t) {
            log(t, "invokeRttAudioIndicatorChanged");
            closeSession(session, t);
        }
    }

    public void invokeDtmfReceived(final ImsCallSessionImplBase session, char dtmf) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeDtmfReceived :: " + dtmf);

            mListener.callSessionDtmfReceived(dtmf);
        } catch (Throwable t) {
            log(t, "invokeDtmfReceived");
            closeSession(session, t);
        }
    }

    /**
     * Called to report a change of call quality.
     *
     * @param callQuality Defined in android.telephony.CallQuality
     *
    */
    public void invokeCallQualityChanged(CallQuality callQuality) {
        try {
            if (mListener == null) {
                return;
            }

            mListener.callQualityChanged(callQuality);
        } catch (Throwable t) {
            log(t, "invokeCallQualityChanged");
        }
    }

    /**
     * Called to report incoming RTP header extension data.
     *
     * @param extensions The header extension data eceived.
     */
    public void invokeRtpHeaderExtensionsReceived(Set<RtpHeaderExtension> extensions) {
        try {
            if (mListener == null) {
                return;
            }

            log("invokeRtpHeaderExtensionsReceived :: " + extensions);

            mListener.callSessionRtpHeaderExtensionsReceived(extensions);
        } catch (Throwable t) {
            log(t, "invokeRtpHeaderExtensionsReceived");
        }
    }

    /**
     * Called to deliver the Anbr query.
     *
     * @param mediaType is used to identify media stream such as audio or video.
     * @param direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate requested by the other party UE through
     *        RTP CMR, RTCPAPP or TMMBR, and ImsStack converts this value to the MAC bitrate
     *        (defined in TS36.321, range: 0 ~ 8000 kbit/s).
     */
    public void invokeSendAnbrQuery(int mediaType, int direction, int bitsPerSecond) {
        try {
            if (mListener == null) {
                return;
            }
            mListener.callSessionSendAnbrQuery(mediaType, direction, bitsPerSecond);
        } catch (Throwable t) {
            log(t, "invokeSendAnbrQuery");
        }
    }

    private void closeSession(ImsCallSessionImplBase session, Throwable t) {
        Throwable cause = t.getCause();

        if (t instanceof DeadObjectException
                || (cause != null && cause instanceof DeadObjectException)) {
            try {
                session.close();
            } catch (Throwable tt) {
                log(tt, "closeSession Exception: " + tt.toString());
            }
        }
    }

    private void log(Throwable t, String message) {
        if (t instanceof DeadObjectException) {
            mListener = null;
        } else if (mListener != null) {
            ImsLog.e("[ISIL] " + message + t.getMessage(), t);
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
