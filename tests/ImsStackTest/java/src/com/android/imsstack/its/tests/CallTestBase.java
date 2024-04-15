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
package com.android.imsstack.its.tests;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.os.Bundle;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.aidl.IImsCallSessionListener;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.ims.internal.IImsCallSession;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.imsservice.mmtel.call.ImsCallSessionWrapper;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.util.SingleLatch;

public class CallTestBase extends ImsStackTestBase {
    protected final SingleLatch mEventLatch = new SingleLatch(CallTestBase.class.getSimpleName());
    // TODO: Considering to move these SingleLatchs into ImsCallSessionWrapper.
    protected final SingleLatch mInitiatedLatch = new SingleLatch("CallInitiated");
    protected final SingleLatch mTerminatedLatch = new SingleLatch("CallTerminated");
    protected final SingleLatch mIncomingCallLatch = new SingleLatch("OnIncomingCall");
    protected final SingleLatch mUnexpectedEventLatch =
            new SingleLatch("UnexpectedCallTermination");
    protected final CallSessionListener mCallSessionListener = new CallSessionListener();
    protected final IncomingCallListener mIncomingCallListener = new IncomingCallListener();

    protected ImsRegistrationWrapper mImsRegistration = null;
    protected ImsMmTelFeatureWrapper mMmTelFeature = null;
    protected PersistableBundle mConfig = null;
    protected boolean mTestCompleted;
    protected int mExpectedReason;
    protected ImsCallSessionWrapper mIncomingCallSessionWrapper = null;

    public CallTestBase() {
        mTestCompleted = false;
        mExpectedReason = ImsReasonInfo.CODE_UNSPECIFIED;
    }

    protected void performRegistration() {
        startImsStack(SLOT0, mConfig);
        enableAllMmTelCapabilities();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);

        mImsRegistration.waitForRegistered();

        // Just sleep to have a delay between registration and call.
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
    }

    protected @NonNull ImsCallSessionWrapper createAndStartVoiceCall() {
        ImsCallProfile callProfile = mMmTelFeature.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        ImsCallSessionWrapper callSession =
                mMmTelFeature.createCallSession(callProfile, mCallSessionListener);
        callSession.start("1234567890", callProfile);
        return callSession;
    }

    // TODO: Move into CallTestUtilities / CallTestConfigManager.
    protected void turnOffQosAndPrecondition() {
        if (mConfig == null) {
            mConfig = new PersistableBundle();
        }
        mConfig.putInt(
                CarrierConfig.Assets.KEY_POLICY_FOR_ALERT_NOT_USING_PRECONDITION_MECHANISM_INT, 0);
        mConfig.putBoolean(CarrierConfigManager.ImsVoice.KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL,
                false);
    }

    protected class CallSessionListener implements ImsCallSessionWrapper.Listener {
        @Override
        public void callSessionInitiated(ImsCallProfile profile) {
            logd("CallSessionListener.callSessionInitiated");
            mInitiatedLatch.countDownAndInit();
        }

        @Override
        public void callSessionTerminated(ImsReasonInfo reasonInfo) {
            logd("CallSessionListener.callSessionTerminated : " + reasonInfo.mCode);
            if (mTestCompleted) {
                logd("CallSessionListener.callSessionTerminated : ignore after completion");
                return;
            }

            if (reasonInfo.mCode != mExpectedReason) {
                loge("CallSessionListener.callSessionTerminated : unexpected reason");
                mUnexpectedEventLatch.countDown();
            } else {
                logi("CallSessionListener.callSessionTerminated : expected reason");
                mTerminatedLatch.countDownAndInit();
            }
        }
    }

    protected class IncomingCallListener implements ImsMmTelFeatureWrapper.IncomingCallListener {
        @Override
        public @Nullable IImsCallSessionListener onIncomingCall(
                @NonNull IImsCallSession c, @Nullable String callId, @Nullable Bundle extras) {
            logi("IncomingCallListener.onIncomingCall");
            // TODO: use extras.
            ImsCallProfile incomingCallProfile =
                    mImsServiceConnector.getMmTelFeature().createCallProfile(
                            ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
            mIncomingCallSessionWrapper = new ImsCallSessionWrapper(c, mCallSessionListener);

            mIncomingCallLatch.countDownAndInit();
            return mIncomingCallSessionWrapper.getCallSessionListenerProxy();
        }
    }
}
