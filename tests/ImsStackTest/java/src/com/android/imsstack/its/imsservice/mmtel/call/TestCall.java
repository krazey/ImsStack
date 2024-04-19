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

import static org.junit.Assert.fail;

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.aidl.IImsCallSessionListener;
import android.telephony.ims.stub.ImsCallSessionImplBase.State;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.ims.internal.IImsCallSession;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.util.SingleLatch;

import java.util.EnumMap;
import java.util.function.IntPredicate;
import java.util.function.Predicate;

/**
 * Provides utilities and assertions for call tests that use {@link ImsCallSessionWrapper}.
 */
public class TestCall {
    private static final int TIMEOUT_DEFAULT_MS = 10000;

    private final @NonNull ImsMmTelFeatureWrapper mMmTelFeature;
    private final @NonNull CallSessionListener mCallSessionListener = new CallSessionListener();
    private final @NonNull IncomingCallListener mIncomingCallListener = new IncomingCallListener();

    private @Nullable ImsCallSessionWrapper mCallSession = null;

    private @NonNull CallEvent mExpectedEvent = new CallEvent(CallEvent.Type.NONE);
    private final @NonNull EnumMap<CallEvent.Type, CallEvent.EventRecord> mEventRecords =
            new EnumMap<>(CallEvent.Type.class);
    private @NonNull SingleLatch mLatch = new SingleLatch(TestCall.class.getSimpleName());

    public TestCall(@NonNull ImsMmTelFeatureWrapper mmTelFeature) {
        mMmTelFeature = mmTelFeature;
    }

    /** Triggers {@link ImsCallSessionWrapper#start}. */
    public void start(@NonNull String callee, int serviceType, int callType) {
        final ImsCallProfile profile = mMmTelFeature.createCallProfile(serviceType, callType);
        mCallSession = mMmTelFeature.createCallSession(profile, mCallSessionListener);
        if (mCallSession == null) {
            return;
        }

        mCallSession.start(callee, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#start} with parameters for a voice call. */
    public void startVoiceCall() {
        start("123456789", ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
    }

    /** Triggers {@link ImsCallSessionWrapper#accept}. */
    public void accept(int callType, @Nullable ImsStreamMediaProfile profile) {
        mCallSession.accept(callType, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#accept} with parameters for a voice call. */
    public void acceptAsVoice() {
        // TODO: need to set profile?
        accept(ImsCallProfile.CALL_TYPE_VOICE, null);
    }

    /** Triggers {@link ImsCallSessionWrapper#reject}. */
    public void reject(int reason) {
        mCallSession.reject(reason);
    }

    /** Triggers {@link ImsCallSessionWrapper#terminate}. */
    public void terminate(int reason) {
        mCallSession.terminate(reason);
    }

    /** Waits an event for a while. Just continues when a timeout occurs. */
    public @NonNull Expectation await() {
        return await(TIMEOUT_DEFAULT_MS);
    }

    /** Waits an event for a given time. Just continues when a timeout occurs. */
    public @NonNull Expectation await(int millis) {
        return new TimedExpectation(false, millis);
    }

    /** Shortcut of {@link TestCall#expectWithin} with the default timeout. */
    public @NonNull Expectation expect() {
        return expectWithin(TIMEOUT_DEFAULT_MS);
    }

    /** Continues if an expected event occurs before the given time has elapsed. Otherwise fails. */
    public @NonNull Expectation expectWithin(int millis) {
        resetExpectation();
        return new TimedExpectation(true, millis);
    }

    /** Checks for an event that have already been triggered. */
    public @NonNull Expectation expectToHaveBeen() {
        return new EventTriggerExpectation();
    }

    /** Returns null if not started or no incoming call. */
    public @Nullable ImsCallSessionWrapper getCallSession() {
        return this.mCallSession;
    }

    /** Triggers {@link ImsCallSessionWrapper#getState}. */
    public int getState() {
        if (this.mCallSession == null) {
            return State.INVALID;
        }
        return this.mCallSession.getState();
    }

    private void resetExpectation() {
        this.mExpectedEvent = new CallEvent(CallEvent.Type.NONE);
        this.mEventRecords.clear();
    }

    /** Waits for a specified event to occur within a defined time period. */
    private class TimedExpectation extends Expectation {
        private @NonNull final Runnable mWait;

        /**
         * @param needAssert If true, the test fails when the event doesn't occur.
         *                   ({@link Expectation#not} can be used to invert the result.)
         *                   If false, the test continues without fail.
         */
        TimedExpectation(boolean needAssert, int waitingTimeInMillis) {
            super();

            mWait = () -> {
                if (!needAssert) {
                    mLatch.sleep(waitingTimeInMillis);
                    return;
                }

                if (isExpectToBeTriggered()) {
                    mLatch.await(waitingTimeInMillis);
                } else {
                    mLatch.awaitTimeout(waitingTimeInMillis);
                }
            };
        }

        @Override
        public void incomingCall() {
            mMmTelFeature.setIncomingCallListener(mIncomingCallListener);

            mExpectedEvent = new CallEvent.MmTelIncomingCallEvent();
            mWait.run();

            mMmTelFeature.setIncomingCallListener(null);
        }

        @Override
        public void initiated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            mExpectedEvent = new CallEvent.SessionInitiatedEvent(profileMatcher);
            mWait.run();
        }

        @Override
        public void terminated(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            mExpectedEvent = new CallEvent.SessionTerminatedEvent(reasonMatcher);
            mWait.run();
        }

        @Override
        public void ussdMessageReceived(
                @NonNull IntPredicate modeMatcher, @NonNull Predicate<String> ussdMessageMatcher) {
            mExpectedEvent = new CallEvent.SessionUssdMessageReceivedEvent(
                    modeMatcher, ussdMessageMatcher);
            mWait.run();
        }
    }

    /** Checks the event records whether a specific event has been triggered before. */
    private class EventTriggerExpectation extends Expectation {
        @Override
        public void incomingCall() {
            final @Nullable CallEvent.EventRecord record =
                    mEventRecords.get(CallEvent.Type.MMTEL_INCOMING_CALL);

            assertTriggered(record);
        }

        @Override
        public void initiated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            final @Nullable CallEvent.EventRecord record =
                    mEventRecords.get(CallEvent.Type.SESSION_INITIATED);

            if (!assertTriggered(record)) {
                return;
            }
            if (!profileMatcher.test((ImsCallProfile) record.param1)) {
                failByUnexpectedParameters();
            }
        }

        @Override
        public void terminated(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            final @Nullable CallEvent.EventRecord record =
                    mEventRecords.get(CallEvent.Type.SESSION_TERMINATED);

            if (!assertTriggered(record)) {
                return;
            }
            if (!reasonMatcher.test((ImsReasonInfo) record.param1)) {
                failByUnexpectedParameters();
            }
        }

        @Override
        public void ussdMessageReceived(
                    @NonNull IntPredicate modeMatcher,
                    @NonNull Predicate<String> ussdMessageMatcher) {
            final @Nullable CallEvent.EventRecord record =
                    mEventRecords.get(CallEvent.Type.SESSION_USSD_MESSAGE_RECEIVED);

            if (!assertTriggered(record)) {
                return;
            }
            if (!modeMatcher.test((int) record.param1)
                    || !ussdMessageMatcher.test((String) record.param2)) {
                failByUnexpectedParameters();
            }
        }

        /**
         * @return True if the record exists and continue to check parameters.
         *         False if the record doesn't exist so no need to check parameters.
         *         The test fails and returns nothing if the assertion fails.
         */
        private boolean assertTriggered(@Nullable CallEvent.EventRecord record) {
            if (isExpectToBeTriggered()) {
                if (record == null) {
                    fail("The event with the given parameter is expected but not triggered.");
                }
                return true;    // Pass, continue to check parameters.
            } else {
                if (record != null) {
                    fail("The event with the given parameter is not expected but triggered.");
                }
                return false;   // Pass, no need to check parameters.
            }
        }

        private void failByUnexpectedParameters() {
            fail("The event is triggered with unexpected parameters.");
        }
    }

    private class IncomingCallListener implements ImsMmTelFeatureWrapper.IncomingCallListener {
        @Override
        public @Nullable IImsCallSessionListener onIncomingCall(
                @NonNull IImsCallSession c, @Nullable String callId, @Nullable Bundle extras) {
            mEventRecords.put(CallEvent.Type.MMTEL_INCOMING_CALL, new CallEvent.EventRecord());

            mCallSession = new ImsCallSessionWrapper(c, mCallSessionListener);

            if (mExpectedEvent.is(CallEvent.Type.MMTEL_INCOMING_CALL)) {
                mLatch.countDownAndInit();
            }

            return mCallSessionListener;
        }
    }

    private class CallSessionListener extends ImsCallSessionWrapper.ImsCallSessionListener {
        @Override
        public void callSessionInitiated(ImsCallProfile profile) {
            mEventRecords.put(CallEvent.Type.SESSION_INITIATED, new CallEvent.EventRecord(profile));

            if (mExpectedEvent.is(CallEvent.Type.SESSION_INITIATED)
                    && ((CallEvent.SessionInitiatedEvent) mExpectedEvent)
                        .matchParameter(profile)) {
                mLatch.countDownAndInit();
            }
        }

        @Override
        public void callSessionTerminated(ImsReasonInfo reason) {
            mEventRecords.put(CallEvent.Type.SESSION_TERMINATED, new CallEvent.EventRecord(reason));

            if (mExpectedEvent.is(CallEvent.Type.SESSION_TERMINATED)
                    && ((CallEvent.SessionTerminatedEvent) mExpectedEvent)
                        .matchParameter(reason)) {
                mLatch.countDownAndInit();
            }
        }

        @Override
        public void callSessionUssdMessageReceived(int mode, String ussdMessage) {
            mEventRecords.put(CallEvent.Type.SESSION_USSD_MESSAGE_RECEIVED,
                    new CallEvent.EventRecord(mode, ussdMessage));

            if (mExpectedEvent.is(CallEvent.Type.SESSION_TERMINATED)
                    && ((CallEvent.SessionUssdMessageReceivedEvent) mExpectedEvent)
                        .matchParameter(mode, ussdMessage)) {
                mLatch.countDownAndInit();
            }
        }
    }
}
