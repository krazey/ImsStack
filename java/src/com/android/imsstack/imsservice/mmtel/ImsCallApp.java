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

import android.content.Context;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;

import androidx.annotation.NonNull;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcStateUtils;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.imsservice.mmtel.base.ImsApp;
import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.internal.annotations.VisibleForTesting;

import java.util.concurrent.Executor;

public class ImsCallApp extends ImsApp {
    private final Object mLock = new Object();
    private final ImsCallContext mCallContext;
    private final ImsRegistrationTracker mRegTracker;

    private ImsFeatureManager mFeatureManager = null;
    private ImsCallManager mCallManager = null;
    private ImsEcbmImpl mEcbm = null;
    private ImsSmsImpl mSms = null;
    private ImsMultiEndpointImpl mMultiEndpoint = null;
    private ImsUtImpl mUt = null;
    private boolean mInitCompleted = false;

    public ImsCallApp(ImsContext imsContext, ImsRegistrationTracker regTracker,
            IMmTelFeatureCapabilityListener featureCapabilityListener,
            IMmTelCallListener callListener) {
        super(imsContext.getPhoneId());

        mCallContext = new ImsCallContext(imsContext.getContext(), imsContext.getExecutor(),
                imsContext.getLooper(), this);
        mRegTracker = regTracker;

        mFeatureManager = new ImsFeatureManager(mCallContext, featureCapabilityListener);
        mCallManager = new ImsCallManager(mCallContext,
                mCallContext.getMtcApp(), callListener);

        // FIXME: remove later if SS control configuration is properly provided
        getUtInterface();

        mFeatureManager.setRegistrationTracker(mRegTracker);
        // Service feature capabilities updates
        mFeatureManager.updateFeaturesOnServiceUpDown(true);

        mInitCompleted = true;
    }

    @VisibleForTesting
    public ImsCallApp(int phoneId, Context context, Executor executor,
            ImsRegistrationTracker regTracker,
            IMmTelFeatureCapabilityListener featureCapabilityListener,
            IMmTelCallListener callListener, ImsCallContext callContext,
            ImsCallManager callManager, ImsFeatureManager featureManager) {
        super(phoneId);

        mCallContext = callContext;
        mRegTracker = regTracker;
        mFeatureManager = featureManager;
        mCallManager = callManager;
        mInitCompleted = true;
    }

    @Override
    public void close() {
        logi("close :: phoneId=" + getPhoneId());

        if (mEcbm != null) {
            mEcbm.dispose();
            mEcbm = null;
        }

        if (mSms != null) {
            mSms.dispose();
            mSms = null;
        }

        if (mMultiEndpoint != null) {
            mMultiEndpoint.dispose();
            mMultiEndpoint = null;
        }

        if (mUt != null) {
            try {
                mUt.dispose();
            } catch (Throwable t) {
                ImsLog.e("UtInterface Exception: " + t.toString());
            }

            mFeatureManager.setUtInterface(null);
            mUt = null;
        }

        if (mCallManager != null) {
            mCallManager.dispose();
            mCallManager = null;
        }

        if (mFeatureManager != null) {
            mFeatureManager.setRegistrationTracker(null);
            mFeatureManager.dispose();
            mFeatureManager = null;
        }

        mCallContext.dispose();

        mInitCompleted = false;
    }

    @Override
    public boolean isConnected(int serviceType, int callType) {
        // Ignore the service type in this moment...
        if (serviceType == ImsCallProfile.SERVICE_TYPE_NORMAL) {
            if (callType <= 0) {
                return mRegTracker.isCallRegistered();
            }

            // __TEST_MODE__ :: call over WiFi
            if (callType == ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO) {
                if (mRegTracker.isCallVoiceAndVideoRegistered()) {
                    return true;
                }
            } else if (callType == ImsCallProfile.CALL_TYPE_VOICE) {
                if ((mRegTracker.isCallVoiceRegistered()
                            || mRegTracker.isCallVoiceAndVideoRegistered())) {
                    return true;
                }
            } else if (callType == ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE) {
                if (mRegTracker.isCallVoiceAndVideoRegistered()) {
                    return true;
                }
            } else if ((callType == ImsCallProfile.CALL_TYPE_VT)
                    || (callType == ImsCallProfile.CALL_TYPE_VT_TX)
                    || (callType == ImsCallProfile.CALL_TYPE_VT_RX)
                    || (callType == ImsCallProfile.CALL_TYPE_VT_NODIR)) {
                if (mRegTracker.isCallVideoRegistered()) {
                    return true;
                }
            }
        }

        return false;
    }

    @Override
    public void onBinderDied() {
        logi("ImsCallApp :: onBinderDied");

        postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    if (mCallManager != null) {
                        mCallManager.closeAllSessions();
                    }
                }
            });

        initializeImsStates(MtcStateUtils.INIT_ALL);
    }

    public void bindCallApp() {
        synchronized (mLock) {
            if (mInitCompleted) {
                logi("ImsCallApp - already binded; phoneId=" + getPhoneId());

                mRegTracker.refreshCallRegistrationState();
                return;
            }

            // FIXME: P-GII
            mRegTracker.refreshCallRegistrationState();

            mCallContext.init();
            mCallManager.init();

            if (mEcbm != null) {
                mEcbm.init();
            }

            if (mSms != null) {
                mSms.init();
            }

            if (mUt != null) {
                mUt.init();
            }

            mFeatureManager.updateFeaturesOnServiceUpDown(true);

            mInitCompleted = true;
        }
    }

    public void unbindCallApp() {
        synchronized (mLock) {
            if (!mInitCompleted) {
                logi("ImsCallApp - already unbinded; phoneId=" + getPhoneId());

                initializeImsStates(MtcStateUtils.INIT_ALL);
                return;
            }

            if (mEcbm != null) {
                mEcbm.clear();
            }

            if (mSms != null) {
                mSms.clear();
            }

            if (mUt != null) {
                mUt.clear();
            }

            initializeImsStates(MtcStateUtils.INIT_ALL);

            mFeatureManager.updateFeaturesOnServiceUpDown(false);

            mCallManager.clear();
            mCallContext.clear();

            mInitCompleted = false;
        }
    }

    public ImsCallProfile createCallProfile(int serviceType, int callType) {
        ImsCallProfile callProfile = null;

        if (ImsCallUtils.isVoiceCall(callType)) {
            callProfile = createCallProfileForVoiceCall(serviceType, callType);
        } else if (ImsCallUtils.isVideoCall(callType)) {
            callProfile = createCallProfileForVideoCall(serviceType, callType);
        } else {
            callProfile = ImsCallUtils.createCallProfile(serviceType, callType,
                    ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.RTT_MODE_DISABLED);
        }

        return callProfile;
    }

    public ImsCallSessionImpl createCallSession(ImsCallProfile profile) {
        return mCallManager.createSession(profile);
    }

    public boolean takeCallSession(ImsCallSessionImpl callSession) {
        return mCallManager.takeSession(callSession);
    }

    public ImsCallManager getCallManager() {
        return mCallManager;
    }

    public ImsEcbmImpl getEcbmInterface() {
        if (mEcbm == null) {
            mEcbm = new ImsEcbmImpl(mCallContext);
        }

        return mEcbm;
    }

    /**
     * Creates the object of ImsSmsImpl
     * @return Returns the object of ImsSmsImpl
     */
    public ImsSmsImpl getSmsInterface() {
        if (mSms == null) {
            mSms = new ImsSmsImpl(mCallContext);
        }

        return mSms;
    }

    /**
     * Creates the object of ImsMultiEndpointImpl
     * @return Returns the object of MultiEndpoint
     */
    public ImsMultiEndpointImpl getMultiEndpointInterface() {
        if (mMultiEndpoint == null) {
            mMultiEndpoint = new ImsMultiEndpointImpl(mCallContext);
        }

        return mMultiEndpoint;
    }

    public ImsUtImpl getUtInterface() {
        if (mUt == null) {
            mUt = new ImsUtImpl(mCallContext);

            mFeatureManager.setUtInterface(mUt.getUtInterface());
        }

        return mUt;
    }

    public void setTtyMode(int ttyMode) {
        TtyModeTracker tmt = mCallContext.getTtyModeTracker();

        if (tmt != null) {
            tmt.setTtyMode(ttyMode);
        }
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("MmTel:");
        pw.increaseIndent();

        MtcApp mtcApp = mCallContext.getMtcApp();
        if (mtcApp != null) {
            mtcApp.dump(pw);
        }

        if (mSms != null) {
            mSms.dump(pw);
        }

        pw.decreaseIndent();
    }

    private ImsCallProfile createCallProfileForVideoCall(int serviceType, int callType) {
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        int videoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        boolean videoCallCapable = false;

        if (serviceType == ImsCallProfile.SERVICE_TYPE_NONE) {
            videoCallCapable = true;
        } else if (serviceType == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            // FIXME : need to add another conditions? (LTE system info. ?)
            videoCallCapable = true;
        } else {
            if ((mRegTracker.isCallVideoRegistered()
                        || mRegTracker.isCallVoiceAndVideoRegistered())) {
                videoCallCapable = true;
            }
        }

        if (videoCallCapable) {
            if (mCallContext.hasAccessBearerCapabilitiesForHDCall() || isAccessBearerUnknown()) {
                audioQuality = mCallContext.getAudioHDQuality();
                videoQuality = mCallContext.getVideoHDQuality();
            } else {
                audioQuality = mCallContext.getAudioHDQuality();
                videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT;
            }

            callType = getCallTypeByImsState(serviceType, callType);

            audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
            videoDirection = ImsCallMediaUtils.getVideoDirectionFromCallType(callType);
        }

        return ImsCallUtils.createCallProfile(serviceType, callType,
                audioQuality, audioDirection, videoQuality, videoDirection);
    }

    private ImsCallProfile createCallProfileForVoiceCall(int serviceType, int callType) {
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        // Set a default media direction to "sendrecv" forcingly
        // to interwork with MTC enabler properly.
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        boolean volteCallCapable = false;

        if (serviceType == ImsCallProfile.SERVICE_TYPE_NONE) {
            if (mCallContext.hasAccessBearerCapabilitiesForHDCall() || isAccessBearerUnknown()) {
                volteCallCapable = true;
            }
        } else if (serviceType == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            // FIXME : need to add another conditions? (LTE system info. ?)
            volteCallCapable = true;
        } else {
            if ((mCallContext.hasAccessBearerCapabilitiesForHDCall() || isAccessBearerUnknown())
                    && (mRegTracker.isCallVoiceRegistered()
                        || mRegTracker.isCallVoiceAndVideoRegistered())) {
                volteCallCapable = true;
            }
        }

        if (volteCallCapable) {
            audioQuality = mCallContext.getAudioHDQuality();
            audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        }

        callType = getCallTypeByImsState(serviceType, callType);

        return ImsCallUtils.createCallProfile(serviceType, callType,
                audioQuality, audioDirection,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID);
    }

    private int getCallTypeByImsState(int serviceType, int callType) {
        // FIXME: needs to improve for call type
        if (callType == ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO) {
            if (serviceType == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
                callType = ImsCallProfile.CALL_TYPE_VOICE;
            } else if (mRegTracker.isCallVoiceRegistered()
                    && !mRegTracker.isCallVoiceAndVideoRegistered()) {
                callType = ImsCallProfile.CALL_TYPE_VOICE;
            }
        } else if (callType == ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE) {
            if (serviceType == ImsCallProfile.SERVICE_TYPE_NONE) {
                if (hasAccessBearerCapabilitiesForNonHDCall()) {
                    callType = ImsCallProfile.CALL_TYPE_VT;
                }
            } else if (!mRegTracker.isCallVoiceAndVideoRegistered()) {
                callType = ImsCallProfile.CALL_TYPE_VT;
            }
        }

        return callType;
    }

    private boolean hasAccessBearerCapabilitiesForNonHDCall() {
        IDcNetWatcher dcnw = mCallContext.getDcNetWatcher();
        return (dcnw != null) && dcnw.is3G();
    }

    private boolean isAccessBearerUnknown() {
        IDcNetWatcher dcnw = mCallContext.getDcNetWatcher();
        return (dcnw != null)
                && (dcnw.getNetworkType() == TelephonyManager.NETWORK_TYPE_UNKNOWN);
    }

    private void initializeImsStates(int initFlags) {
        int phoneId = MSimUtils.DEFAULT_PHONE_ID;

        if (DeviceConfig.isMultiSimEnabled()) {
            phoneId = getPhoneId();
        }

        MtcStateUtils.initializeImsState(mCallContext.getContext(), phoneId, initFlags);
    }

    private void postAndRunTask(Runnable task) {
        mCallContext.getExecutor().execute(task);
    }
}
