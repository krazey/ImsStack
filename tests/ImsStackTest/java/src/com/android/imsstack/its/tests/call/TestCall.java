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
package com.android.imsstack.its.tests.call;

import static org.junit.Assert.fail;

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.aidl.IImsCallSessionListener;
import android.telephony.ims.stub.ImsCallSessionImplBase.State;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.ims.internal.IImsCallSession;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.imsservice.mmtel.call.ImsCallSessionWrapper;
import com.android.imsstack.its.servercontrol.ServerFailureHandler;
import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.util.Log;

import java.util.EnumMap;
import java.util.concurrent.Executors;
import java.util.function.IntPredicate;
import java.util.function.Predicate;

/**
 * Provides utilities and assertions for call tests that use {@link ImsCallSessionWrapper}.
 */
public class TestCall extends ServerFailureHandler {
    private static final int TIMEOUT_DEFAULT_MS = 10000;

    private final @NonNull ImsMmTelFeatureWrapper mMmTelFeature;
    private final @NonNull CallSessionListener mCallSessionListener = new CallSessionListener();
    private final @NonNull IncomingCallListener mIncomingCallListener = new IncomingCallListener();

    private @Nullable ImsCallSessionWrapper mCallSession = null;

    private @NonNull CallEvent<?, ?, ?> mExpectedEvent = new CallEvent<>(CallEvent.Type.NONE);
    private final @NonNull EnumMap<CallEvent.Type, CallEvent.EventRecord> mEventRecords =
            new EnumMap<>(CallEvent.Type.class);
    private @NonNull SingleLatch mLatch = new SingleLatch(TestCall.class.getSimpleName());

    public TestCall(@NonNull ImsMmTelFeatureWrapper mmTelFeature) {
        mMmTelFeature = mmTelFeature;
    }

    /** Triggers {@link ImsCallSessionWrapper#start}. */
    public void start(@NonNull String callee, int serviceType, int callType,
            @Nullable Bundle extras) {
        final ImsCallProfile profile = mMmTelFeature.createCallProfile(serviceType, callType);
        if (extras != null) {
            profile.getCallExtras().putAll(extras);
        }
        mCallSession = mMmTelFeature.createCallSession(profile, mCallSessionListener);
        if (mCallSession == null) {
            fail("Failed to create call session");
            return;
        }

        mCallSession.start(callee, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#start} with parameters for a voice call. */
    public void startVoiceCall() {
        start("123456789", ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE,
                null);
    }

    /** Triggers {@link ImsCallSessionWrapper#startConference}. */
    public void startConference(@NonNull String[] participants, int serviceType, int callType,
            @Nullable Bundle extras) {
        final ImsCallProfile profile = mMmTelFeature.createCallProfile(serviceType, callType);
        if (extras != null) {
            profile.getCallExtras().putAll(extras);
        }
        mCallSession = mMmTelFeature.createCallSession(profile, mCallSessionListener);
        if (mCallSession == null) {
            fail("Failed to create call session");
            return;
        }

        mCallSession.startConference(participants, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#accept}. */
    public void accept(int callType, @Nullable ImsStreamMediaProfile profile) {
        mCallSession.accept(callType, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#accept} with parameters for a voice call. */
    public void acceptAsVoice() {
        accept(ImsCallProfile.CALL_TYPE_VOICE, null);
    }

    /** Triggers {@link ImsCallSessionWrapper#deflect}. */
    public void deflect(@Nullable String deflectNumber) {
        mCallSession.deflect(deflectNumber);
    }

    /** Triggers {@link ImsCallSessionWrapper#reject}. */
    public void reject(int reason) {
        mCallSession.reject(reason);
    }

    /** Triggers {@link ImsCallSessionWrapper#transfer(String, boolean)}. */
    public void transfer(@Nullable String number, boolean isConfirmationRequired) {
        mCallSession.transfer(number, isConfirmationRequired);
    }

    /** Triggers {@link ImsCallSessionWrapper#consultativeTransfer}. */
    public void consultativeTransfer(@Nullable TestCall transferToSession) {
        // TODO: b/333956737
        mCallSession.consultativeTransfer(null);
    }

    /** Triggers {@link ImsCallSessionWrapper#terminate}. */
    public void terminate(int reason) {
        mCallSession.terminate(reason);
    }

    /** Triggers {@link ImsCallSessionWrapper#hold}. */
    public void hold(@Nullable ImsStreamMediaProfile profile) {
        mCallSession.hold(profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#resume}. */
    public void resume(@Nullable ImsStreamMediaProfile profile) {
        mCallSession.resume(profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#merge}. */
    public void merge() {
        mCallSession.merge();
    }

    /** Triggers {@link ImsCallSessionWrapper#update}. */
    public void update(int callType, @Nullable ImsStreamMediaProfile profile) {
        mCallSession.update(callType, profile);
    }

    /** Triggers {@link ImsCallSessionWrapper#extendToConference}. */
    public void extendToConference(String[] participants) {
        mCallSession.extendToConference(participants);
    }

    /** Triggers {@link ImsCallSessionWrapper#inviteParticipants}. */
    public void inviteParticipants(String[] participants) {
        mCallSession.inviteParticipants(participants);
    }

    /** Triggers {@link ImsCallSessionWrapper#removeParticipants}. */
    public void removeParticipants(String[] participants) {
        mCallSession.removeParticipants(participants);
    }

    /** Triggers {@link ImsCallSessionWrapper#sendUssd}. */
    public void sendUssd(String ussdMessage) {
        mCallSession.sendUssd(ussdMessage);
    }

    /** Shortcut of {@link TestCall#expectWithin} with the default timeout. */
    public @NonNull Expectation expect() {
        return expectWithin(TIMEOUT_DEFAULT_MS);
    }

    /** Continues if an expected event occurs before the given time has elapsed. Otherwise fails. */
    public @NonNull Expectation expectWithin(int millis) {
        resetExpectation();
        return new TimedExpectation(millis);
    }

    /**
     * Checks for an event that have already been triggered.
     * {@link TestCall#expectWithin} reset the history.
     */
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

    @Override
    protected void handleServerFailure() {
        mLatch.countDownAndInit();
    }

    /**
     * Stores the call event and stop waiting for the expected event if the given event and
     * parameters match with the expected.
     */
    @SuppressWarnings("unchecked") // It verifies that the event type matches.
    private <T1, T2, T3> void onCallEventReceived(
            @NonNull CallEvent.Type eventType, @NonNull CallEvent.EventRecord record) {
        if (eventType == CallEvent.Type.NONE) {
            Log.e(this, "Invalid event type NONE");
            return;
        }

        mEventRecords.put(eventType, record);

        if (!mExpectedEvent.is(eventType)) {
            Log.i(this, "Expected Event : " + mExpectedEvent + ", actual Event : " + eventType);
            return;
        }

        if (((CallEvent<T1, T2, T3>) mExpectedEvent).matchParameters(
                (T1) record.param1, (T2) record.param2, (T3) record.param3)) {
            mLatch.countDownAndInit();
        }
    }

    private void resetExpectation() {
        this.mExpectedEvent = new CallEvent<>(CallEvent.Type.NONE);
        this.mEventRecords.clear();
    }

    /**
     * Waits for a specified event to occur within a defined time period.
     * The test will fail when the event with the given parameters doesn't occur.
     *
     * When {@link Expectation#not} is set, the test will fail when the event
     * with the given parameters occur.
     */
    private class TimedExpectation extends Expectation {
        private @NonNull final Runnable mWait;
        private final int mWaitingTimeInMillis;

        TimedExpectation(int waitingTimeInMillis) {
            super();

            mWait = () -> {
                if (isExpectToBeTriggered()) {
                    mLatch.await(waitingTimeInMillis);
                } else {
                    mLatch.awaitTimeout(waitingTimeInMillis);
                }

                if (mFailureDetail != null) {
                    fail(mFailureDetail);
                }
            };
            mWaitingTimeInMillis = waitingTimeInMillis;
        }

        @Override
        public void nothing() {
            SingleLatch.delay(mWaitingTimeInMillis);
        }

        @Override
        public void incomingCall(@Nullable Predicate<Bundle> extrasMatcher) {
            mMmTelFeature.setIncomingCallListener(mIncomingCallListener);

            waitFor(new CallEvent<Bundle, Void, Void>(
                    CallEvent.Type.MMTEL_INCOMING_CALL, extrasMatcher));

            mMmTelFeature.setIncomingCallListener(null);
        }

        @Override
        public void initiatingFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_INITIATING_FAILED, reasonMatcher));
        }

        @Override
        public void progressing(@NonNull Predicate<ImsStreamMediaProfile> profileMatcher) {
            waitFor(new CallEvent<ImsStreamMediaProfile, Void, Void>(
                    CallEvent.Type.SESSION_PROGRESSING, profileMatcher));
        }

        @Override
        public void initiated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            waitFor(new CallEvent<ImsCallProfile, Void, Void>(
                    CallEvent.Type.SESSION_INITIATED, profileMatcher));
        }

        @Override
        public void terminated(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_TERMINATED, reasonMatcher));
        }

        @Override
        public void held(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            waitFor(new CallEvent<ImsCallProfile, Void, Void>(
                    CallEvent.Type.SESSION_HELD, profileMatcher));
        }

        @Override
        public void holdFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_HOLD_FAILED, reasonMatcher));
        }

        @Override
        public void resumed(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            waitFor(new CallEvent<ImsCallProfile, Void, Void>(
                    CallEvent.Type.SESSION_RESUMED, profileMatcher));
        }

        @Override
        public void resumeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_RESUME_FAILED, reasonMatcher));
        }

        @Override
        public void mergeStarted(
                @NonNull Predicate<IImsCallSession> sessionMatcher,
                @NonNull Predicate<ImsCallProfile> profileMatcher) {
            waitFor(new CallEvent<IImsCallSession, ImsCallProfile, Void>(
                    CallEvent.Type.SESSION_MERGE_STARTED, sessionMatcher, profileMatcher));
        }

        @Override
        public void mergeComplete(@NonNull Predicate<IImsCallSession> sessionMatcher) {
            waitFor(new CallEvent<IImsCallSession, Void, Void>(
                    CallEvent.Type.SESSION_MERGE_COMPLETE, sessionMatcher));
        }

        @Override
        public void mergeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_MERGE_FAILED, reasonMatcher));
        }

        @Override
        public void updated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            waitFor(new CallEvent<ImsCallProfile, Void, Void>(
                    CallEvent.Type.SESSION_UPDATED, profileMatcher));
        }

        @Override
        public void updateFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_UPDATE_FAILED, reasonMatcher));
        }

        @Override
        public void inviteParticipantsRequestDelivered() {
            waitFor(new CallEvent<>(CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_DELIVERED));
        }

        @Override
        public void inviteParticipantsRequestFailed(
                @NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_FAILED, reasonMatcher));
        }

        @Override
        public void removeParticipantsRequestDelivered() {
            waitFor(new CallEvent<>(CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_DELIVERED));
        }

        @Override
        public void removeParticipantsRequestFailed(
                @NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_FAILED, reasonMatcher));
        }

        @Override
        public void conferenceStateUpdated(@NonNull Predicate<ImsConferenceState> stateMatcher) {
            waitFor(new CallEvent<ImsConferenceState, Void, Void>(
                    CallEvent.Type.SESSION_CONFERENCE_STATE_UPDATED, stateMatcher));
        }

        @Override
        public void ussdMessageReceived(
                @NonNull IntPredicate modeMatcher, @NonNull Predicate<String> ussdMessageMatcher) {
            waitFor(new CallEvent<Integer, String, Void>(
                    CallEvent.Type.SESSION_USSD_MESSAGE_RECEIVED,
                    (obj) -> modeMatcher.test(obj), ussdMessageMatcher));
        }

        @Override
        public void rttMessageReceived(@NonNull Predicate<String> rttMessageMatcher) {
            waitFor(new CallEvent<String, Void, Void>(
                    CallEvent.Type.SESSION_RTT_MESSAGE_RECEIVED, rttMessageMatcher));
        }

        @Override
        public void transferred() {
            waitFor(new CallEvent<>(CallEvent.Type.SESSION_TRANSFERRED));
        }

        @Override
        public void transferFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            waitFor(new CallEvent<ImsReasonInfo, Void, Void>(
                    CallEvent.Type.SESSION_TRANSFER_FAILED, reasonMatcher));
        }

        private void waitFor(@NonNull CallEvent<?, ?, ?> event) {
            mExpectedEvent = event;
            mWait.run();
        }
    }

    /** Checks the event records whether a specific event has been triggered before. */
    private class EventTriggerExpectation extends Expectation {
        @Override
        public void nothing() {}

        @Override
        public void incomingCall(@NonNull Predicate<Bundle> extrasMatcher) {
            assertTriggered(CallEvent.Type.MMTEL_INCOMING_CALL,
                    (record) -> extrasMatcher.test((Bundle) record.param1));
        }

        @Override
        public void initiatingFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_INITIATING_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void progressing(@NonNull Predicate<ImsStreamMediaProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_PROGRESSING,
                    (record) -> profileMatcher.test((ImsStreamMediaProfile) record.param1));
        }

        @Override
        public void initiated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_INITIATED,
                    (record) -> profileMatcher.test((ImsCallProfile) record.param1));
        }

        @Override
        public void terminated(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_TERMINATED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void held(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_HELD,
                    (record) -> profileMatcher.test((ImsCallProfile) record.param1));
        }

        @Override
        public void holdFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_HOLD_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void resumed(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_RESUMED,
                    (record) -> profileMatcher.test((ImsCallProfile) record.param1));
        }

        @Override
        public void resumeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_RESUME_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void mergeStarted(
                @NonNull Predicate<IImsCallSession> sessionMatcher,
                @NonNull Predicate<ImsCallProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_MERGE_STARTED,
                    (record) -> sessionMatcher.test((IImsCallSession) record.param1)
                            && profileMatcher.test((ImsCallProfile) record.param2));
        }

        @Override
        public void mergeComplete(@NonNull Predicate<IImsCallSession> sessionMatcher) {
            assertTriggered(CallEvent.Type.SESSION_MERGE_COMPLETE,
                    (record) -> sessionMatcher.test((IImsCallSession) record.param1));
        }

        @Override
        public void mergeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_MERGE_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void updated(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            assertTriggered(CallEvent.Type.SESSION_UPDATED,
                    (record) -> profileMatcher.test((ImsCallProfile) record.param1));
        }

        @Override
        public void updateFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_UPDATE_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void inviteParticipantsRequestDelivered() {
            assertTriggered(CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_DELIVERED,
                    (record) -> true);
        }

        @Override
        public void inviteParticipantsRequestFailed(
                @NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void removeParticipantsRequestDelivered() {
            assertTriggered(CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_DELIVERED,
                    (record) -> true);
        }

        @Override
        public void removeParticipantsRequestFailed(
                @NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        @Override
        public void conferenceStateUpdated(@NonNull Predicate<ImsConferenceState> stateMatcher) {
            assertTriggered(CallEvent.Type.SESSION_CONFERENCE_STATE_UPDATED,
                    (record) -> stateMatcher.test((ImsConferenceState) record.param1));
        }

        @Override
        public void ussdMessageReceived(
                    @NonNull IntPredicate modeMatcher,
                    @NonNull Predicate<String> ussdMessageMatcher) {
            assertTriggered(CallEvent.Type.SESSION_USSD_MESSAGE_RECEIVED,
                    (record) -> modeMatcher.test((int) record.param1)
                            && ussdMessageMatcher.test((String) record.param2));
        }

        @Override
        public void rttMessageReceived(@NonNull Predicate<String> rttMessageMatcher) {
            assertTriggered(CallEvent.Type.SESSION_RTT_MESSAGE_RECEIVED,
                    (record) -> rttMessageMatcher.test((String) record.param1));
        }

        @Override
        public void transferred() {
            assertTriggered(CallEvent.Type.SESSION_TRANSFERRED, (record) -> true);
        }

        @Override
        public void transferFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            assertTriggered(CallEvent.Type.SESSION_TRANSFER_FAILED,
                    (record) -> reasonMatcher.test((ImsReasonInfo) record.param1));
        }

        /**
         * Asserts if an event was triggered or not, based on {@link #isExpectToBeTriggered()}.
         *
         * If expected, fails if the event is missing or its parameters fail the predicate.
         * If not expected, fails if the event was triggered.
         *
         * @param eventType The event to check.
         * @param parameterPredicate Validates event parameters; only used if the event is expected.
         */
        private void assertTriggered(
                @NonNull CallEvent.Type eventType,
                @NonNull Predicate<CallEvent.EventRecord> parameterPredicate) {
            final @Nullable CallEvent.EventRecord record = mEventRecords.get(eventType);

            if (isExpectToBeTriggered()) {
                if (record == null) {
                    fail("The event " + eventType + " is expected but it hasn't been triggered.");
                } else if (!parameterPredicate.test(record)) {
                    fail("The event has been triggered with unexpected parameters");
                }
            } else {
                if (record != null) {
                    fail("The event " + eventType + " is not expected but has been triggered.");
                }
            }
        }
    }

    /** Receives events from {@link ImsMmTelFeatureWrapper.IncomingCallListener}. */
    private class IncomingCallListener implements ImsMmTelFeatureWrapper.IncomingCallListener {
        @Override
        public @Nullable IImsCallSessionListener onIncomingCall(
                @NonNull IImsCallSession c, @Nullable String callId, @Nullable Bundle extras) {
            mCallSession = new ImsCallSessionWrapper(c, mCallSessionListener);

            Log.d(this, "onIncomingCall");
            onCallEventReceived(
                    CallEvent.Type.MMTEL_INCOMING_CALL, new CallEvent.EventRecord(extras));

            return mCallSessionListener;
        }
    }

    /** Receives events from {@link IImsCallSessionListener}. */
    private class CallSessionListener extends ImsCallSessionWrapper.ImsCallSessionListener {
        @Override
        public void callSessionInitiatingFailed(ImsReasonInfo reason) {
            Log.d(this, "callSessionInitiatingFailed - reason: " + reason);
            onCallEventReceived(CallEvent.Type.SESSION_INITIATING_FAILED,
                    new CallEvent.EventRecord(reason));
        }

        @Override
        public void callSessionProgressing(ImsStreamMediaProfile profile) {
            Log.d(this, "callSessionProgressing - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_PROGRESSING,
                    new CallEvent.EventRecord(profile));
        }

        @Override
        public void callSessionInitiated(ImsCallProfile profile) {
            Log.d(this, "callSessionInitiated - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_INITIATED,
                    new CallEvent.EventRecord(profile));
        }

        @Override
        public void callSessionTerminated(ImsReasonInfo reason) {
            Log.d(this, "callSessionTerminated - reason: " + reason);
            onCallEventReceived(CallEvent.Type.SESSION_TERMINATED,
                    new CallEvent.EventRecord(reason));

            // Simulates the telephony behavior to release the call
            Executors.newSingleThreadExecutor().execute(() -> {
                mCallSession.close();
            });
        }

        @Override
        public void callSessionHeld(ImsCallProfile profile) {
            Log.d(this, "callSessionHeld - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_HELD, new CallEvent.EventRecord(profile));
        }

        @Override
        public void callSessionHoldFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionHoldFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_HOLD_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionResumed(ImsCallProfile profile) {
            Log.d(this, "callSessionResumed - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_RESUMED, new CallEvent.EventRecord(profile));
        }

        @Override
        public void callSessionResumeFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionResumeFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_RESUME_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionMergeStarted(IImsCallSession newSession, ImsCallProfile profile) {
            Log.d(this, "callSessionMergeStarted - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_MERGE_STARTED,
                    new CallEvent.EventRecord(newSession, profile));
        }

        @Override
        public void callSessionMergeComplete(IImsCallSession newSession) {
            Log.d(this, "callSessionMergeComplete");
            onCallEventReceived(CallEvent.Type.SESSION_MERGE_COMPLETE,
                    new CallEvent.EventRecord(newSession));
        }

        @Override
        public void callSessionMergeFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionMergeFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_MERGE_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionUpdated(ImsCallProfile profile) {
            Log.d(this, "callSessionUpdated - profile: " + profile);
            onCallEventReceived(CallEvent.Type.SESSION_UPDATED, new CallEvent.EventRecord(profile));
        }

        @Override
        public void callSessionUpdateFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionUpdateFailed - reason: " + reasonInfo);
            onCallEventReceived(
                    CallEvent.Type.SESSION_UPDATE_FAILED, new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionInviteParticipantsRequestDelivered() {
            Log.d(this, "callSessionInviteParticipantsRequestDelivered");
            onCallEventReceived(CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_DELIVERED,
                    new CallEvent.EventRecord());
        }

        @Override
        public void callSessionInviteParticipantsRequestFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionInviteParticipantsRequestFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_INVITE_PARTICIPANTS_REQUEST_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionRemoveParticipantsRequestDelivered() {
            Log.d(this, "callSessionRemoveParticipantsRequestDelivered");
            onCallEventReceived(CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_DELIVERED,
                    new CallEvent.EventRecord());
        }

        @Override
        public void callSessionRemoveParticipantsRequestFailed(ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionRemoveParticipantsRequestFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_REMOVE_PARTICIPANTS_REQUEST_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }

        @Override
        public void callSessionConferenceStateUpdated(ImsConferenceState state) {
            Log.d(this, "callSessionConferenceStateUpdated - state: " + state);
            onCallEventReceived(CallEvent.Type.SESSION_CONFERENCE_STATE_UPDATED,
                    new CallEvent.EventRecord(state));
        }

        @Override
        public void callSessionUssdMessageReceived(int mode, String ussdMessage) {
            Log.d(this, "callSessionUssdMessageReceived - "
                    + "mode: " + mode + " ussdMessage: " + ussdMessage);
            onCallEventReceived(CallEvent.Type.SESSION_USSD_MESSAGE_RECEIVED,
                    new CallEvent.EventRecord(mode, ussdMessage));
        }

        @Override
        public void callSessionRttMessageReceived(String rttMessage) {
            Log.d(this, "callSessionRttMessageReceived - rttMessage: " + rttMessage);
            onCallEventReceived(CallEvent.Type.SESSION_RTT_MESSAGE_RECEIVED,
                    new CallEvent.EventRecord(rttMessage));
        }

        @Override
        public void callSessionTransferred() {
            Log.d(this, "callSessionTransferred");
            onCallEventReceived(CallEvent.Type.SESSION_TRANSFERRED, new CallEvent.EventRecord());
        }

        @Override
        public void callSessionTransferFailed(@Nullable ImsReasonInfo reasonInfo) {
            Log.d(this, "callSessionTransferFailed - reason: " + reasonInfo);
            onCallEventReceived(CallEvent.Type.SESSION_TRANSFER_FAILED,
                    new CallEvent.EventRecord(reasonInfo));
        }
    }
}
