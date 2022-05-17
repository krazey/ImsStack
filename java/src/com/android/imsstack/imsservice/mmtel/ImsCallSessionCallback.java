/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150520    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel;

import android.os.DeadObjectException;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.stub.ImsCallSessionImplBase;

import com.android.imsstack.util.ImsLog;

import java.util.concurrent.Executor;

public class ImsCallSessionCallback {
    private final Executor mExecutor;
    private ImsCallSessionListener mListener = null;

    public ImsCallSessionCallback(Executor executor) {
        mExecutor = executor;
    }

    public boolean hasListener() {
        return (mListener != null);
    }

    public void setListener(ImsCallSessionListener listener) {
        mListener = listener;
    }

    public void invokeInitiating(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeProgressing(final ImsCallSessionImplBase session,
            final ImsStreamMediaProfile mediaProfile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeStarted(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeStartFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeTerminated(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    logi("invokeTerminated :: " + reasonInfo);

                    mListener.callSessionTerminated(reasonInfo);
                } catch (Throwable t) {
                    log(t, "invokeTerminated");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeHeld(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionHeld(profile);
                } catch (Throwable t) {
                    log(t, "invokeHeld");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeHoldFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeHoldReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionHoldReceived(profile);
                } catch (Throwable t) {
                    log(t, "invokeHoldReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeResumed(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionResumed(profile);
                } catch (Throwable t) {
                    log(t, "invokeResumed");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeResumeFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeResumeReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionResumeReceived(profile);
                } catch (Throwable t) {
                    log(t, "invokeResumeReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeMergeStarted(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeMergeComplete(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase newSession) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeMergeFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeUpdated(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    // This is for removing the timing issue
                    // to update the call extras for conference session.
                    if (mListener == null) {
                        log("invokeUpdated :: no listener");
                        return;
                    }

                    mListener.callSessionUpdated(profile);
                } catch (Throwable t) {
                    log(t, "invokeUpdated");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeUpdateFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeUpdateReceived(final ImsCallSessionImplBase session,
            final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionUpdateReceived(profile);
                } catch (Throwable t) {
                    log(t, "invokeUpdateReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeConferenceExtended(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionConferenceExtended(confSession, profile);
                } catch (Throwable t) {
                    log(t, "invokeConferenceExtended");
                    closeSession(session, t);

                    if (confSession != null) {
                        closeSession(confSession, t);
                    }
                }
            }
        });
    }

    public void invokeConferenceExtendFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeConferenceExtendReceived(final ImsCallSessionImplBase session,
            final ImsCallSessionImplBase confSession, final ImsCallProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionConferenceExtendReceived(confSession, profile);
                } catch (Throwable t) {
                    log(t, "invokeConferenceExtendReceived");
                    closeSession(session, t);

                    if (confSession != null) {
                        closeSession(confSession, t);
                    }
                }
            }
        });
    }

    public void invokeInviteParticipantsRequestDelivered(final ImsCallSessionImplBase session) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionInviteParticipantsRequestDelivered();
                } catch (Throwable t) {
                    log(t, "invokeInviteParticipantsRequestDelivered");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeInviteParticipantsRequestFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeRemoveParticipantsRequestDelivered(final ImsCallSessionImplBase session) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionRemoveParticipantsRequestDelivered();
                } catch (Throwable t) {
                    log(t, "invokeRemoveParticipantsRequestDelivered");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeRemoveParticipantsRequestFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeConferenceStateUpdated(final ImsCallSessionImplBase session,
            final ImsConferenceState confState) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionConferenceStateUpdated(confState);
                } catch (Throwable t) {
                    log(t, "invokeConferenceStateUpdated");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeUssdMessageReceived(final ImsCallSessionImplBase session,
            final int mode, final String ussdMessage) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionUssdMessageReceived(mode, ussdMessage);
                } catch (Throwable t) {
                    log(t, "invokeUssdMessageReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeMultipartyStateChanged(final ImsCallSessionImplBase session,
            final boolean isMultiparty) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionMultipartyStateChanged(isMultiparty);
                } catch (Throwable t) {
                    log(t, "invokeMultipartyStateChanged");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeCallSessionTransferred(final ImsCallSessionImplBase session) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionTransferred();
                } catch (Throwable t) {
                    log(t, "invokeCallSessionTransferred");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeCallSessionTransferFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionTransferFailed(reasonInfo);
                } catch (Throwable t) {
                    log(t, "invokeCallSessionTransferFailed");
                    closeSession(session, t);
                }
            }
        });
    }

    // @QUALCOMM_API
    public void invokeHandover(final ImsCallSessionImplBase session,
            final int srcAccessTech, final int targetAccessTech) {
        // FIXME: need to implement
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UNSPECIFIED,
                            ImsReasonInfo.CODE_UNSPECIFIED, null);

                    mListener.callSessionHandover(
                            srcAccessTech, targetAccessTech, reasonInfo);
                } catch (Throwable t) {
                    log(t, "invokeHandover");
                    closeSession(session, t);
                }
            }
        });
    }

    // @QUALCOMM_API
    public void invokeHandoverFailed(final ImsCallSessionImplBase session,
            final int srcAccessTech, final int targetAccessTech,
            final ImsReasonInfo reasonInfo) {
        // FIXME: need to implement
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionHandoverFailed(
                            srcAccessTech, targetAccessTech, reasonInfo);
                } catch (Throwable t) {
                    log(t, "invokeHandoverFailed");
                    closeSession(session, t);
                }
            }
        });
    }

    // @QUALCOMM_API
    public void invokeTtyModeReceived(final ImsCallSessionImplBase session,
            final int mode) {
        // FIXME: need to implement
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    // @QUALCOMM_API
    public void invokeDeflected(final ImsCallSessionImplBase session) {
        // FIXME: need to implement
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    // FIXME: M_OS_GII
                    //mListener.callSessionDeflected();
                } catch (Throwable t) {
                    log(t, "invokeDeflected");
                    closeSession(session, t);
                }
            }
        });
    }

    // @QUALCOMM_API
    public void invokeDeflectFailed(final ImsCallSessionImplBase session,
            final ImsReasonInfo reasonInfo) {
        // FIXME: need to implement
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    // FIXME: M_OS_GII
                    //mListener.callSessionDeflectFailed(reasonInfo);
                } catch (Throwable t) {
                    log(t, "invokeDeflectFailed");
                    closeSession(session, t);
                }
            }
        });
    }

    // @QUALCOMM_API
    public void invokeSuppServiceReceived(final ImsCallSessionImplBase session,
            final ImsSuppServiceNotification issn) {

        postAndRunTask(new Runnable() {
            @Override
            public void run() {
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
        });
    }

    public void invokeRttModifyRequestReceived(final ImsCallSessionImplBase session,
            ImsCallProfile callProfile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionRttModifyRequestReceived(callProfile);
                } catch (Throwable t) {
                    log(t, "invokeRttModifyRequestReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeRttModifyResponseReceived(final ImsCallSessionImplBase session,
            final int status) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionRttModifyResponseReceived(status);
                } catch (Throwable t) {
                    log(t, "invokeRttModifyResponseReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeRttMessageReceived(final ImsCallSessionImplBase session,
            final String data) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionRttMessageReceived(data);
                } catch (Throwable t) {
                    log(t, "invokeRttMessageReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeRttAudioIndicatorChanged(final ImsCallSessionImplBase session,
            final ImsStreamMediaProfile profile) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionRttAudioIndicatorChanged(profile);
                } catch (Throwable t) {
                    log(t, "invokeRttAudioIndicatorChanged");
                    closeSession(session, t);
                }
            }
        });
    }

    public void invokeDtmfReceived(final ImsCallSessionImplBase session, char dtmf) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (mListener == null) {
                        return;
                    }

                    mListener.callSessionDtmfReceived(dtmf);
                } catch (Throwable t) {
                    log(t, "invokeDtmfReceived");
                    closeSession(session, t);
                }
            }
        });
    }

    private void closeSession(ImsCallSessionImplBase session, Throwable t) {
        Throwable cause = t.getCause();

        if (t instanceof DeadObjectException
                || (cause != null && cause instanceof DeadObjectException)) {
            try {
                session.close();
            } catch (Throwable tt) {
                tt.printStackTrace();
            }
        }
    }

    private void postAndRunTask(Runnable task) {
        mExecutor.execute(task);
    }

    private void log(Throwable t, String message) {
        if (t instanceof DeadObjectException) {
            mListener = null;
        } else if (mListener != null) {
            ImsLog.e("[GII-IMPL] " + message + t, t);
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
