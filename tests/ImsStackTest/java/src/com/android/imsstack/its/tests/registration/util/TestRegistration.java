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
package com.android.imsstack.its.tests.registration.util;

import static org.junit.Assert.fail;

import android.net.Uri;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.util.Log;

import java.util.Arrays;
import java.util.EnumMap;
import java.util.Objects;
import java.util.function.Predicate;

/**
 * Provides utilities and assertions for registration tests that use {@link ImsRegistrationWrapper}.
 */
public class TestRegistration {

    private static final int DEFAULT_TIMEOUT_MS = 10000;

    @NonNull private final ImsRegistrationWrapper mImsRegistration;
    @NonNull private final SingleLatch mLatch = new SingleLatch(
            TestRegistration.class.getSimpleName());
    @NonNull private final EnumMap<RegEvent.Type, RegEvent.EventRecord> mEventRecords =
            new EnumMap<>(RegEvent.Type.class);
    @NonNull private RegEvent mExpectedEvent = new RegEvent(RegEvent.Type.NONE);

    public TestRegistration(@NonNull ImsRegistrationWrapper registration) {
        Objects.requireNonNull(registration, "registration must not be null.");

        mImsRegistration = registration;
        mImsRegistration.setListener(new RegistrationListener());
    }

    /**
     * Requests the IMS stack to perform a graceful IMS deregistration before the radio performs
     * network detachment in the events of SIM removal, refresh, or similar events.
     *
     * @param reason The reason why the deregistration is triggered.
     */
    public void triggerDeregistration(
            @ImsRegistrationImplBase.ImsDeregistrationReason int reason) {
        mImsRegistration.triggerDeregistration(reason);
    }

    /**
     * Initializes an expectation for an event.
     * If no timeout is specified, the default timeout value is used.
     *
     * @return An {@link Expectation} object for defining further expectations.
     */
    @NonNull
    public Expectation expect() {
        return expect(null);
    }

    /**
     * Waits for an expected event to occur within the specified time duration.
     * If the event occurs within this time, the test continues; otherwise, it fails.
     * If no timeout is specified, the default timeout value is used.
     *
     * @param millis The time duration to wait for the event, in milliseconds.
     * @return A {@link Expectation} object for further expectations.
     */
    @NonNull
    public Expectation expect(@Nullable Integer millis) {
        clearEvent();
        return new TimedExpectation(true, millis != null ? millis : DEFAULT_TIMEOUT_MS);
    }

    /**
     * Initializes an expectation for an event that is not expected to occur.
     * If no timeout is specified, the default timeout value is used.
     *
     * @return An {@link Expectation} object for defining further expectations.
     */
    @NonNull
    public Expectation expectNot() {
        return expectNot(null);
    }

    /**
     * Waits for an unexpected event to not occur within the specified time duration.
     * If the event does not occur within this time, the test continues; otherwise, it fails.
     * If no timeout is specified, the default timeout value is used.
     *
     * @param millis The time duration to wait for the absence of the event, in milliseconds.
     * @return A {@link Expectation} object for further expectations.
     */
    @NonNull
    public Expectation expectNot(@Nullable Integer millis) {
        clearEvent();
        return new TimedExpectation(false, millis != null ? millis : DEFAULT_TIMEOUT_MS);
    }

    /**
     * Retrieves an expectation for an event that has already been triggered.
     * The history of expectations is reset by calling {@link TestRegistration#clearEvent}.
     *
     * @return An {@link Expectation} object representing the expected event.
     */
    @NonNull
    public Expectation expectTriggered() {
        return new TriggeredExpectation(true);
    }

    /**
     * Retrieves an expectation for an event that has not been triggered.
     * The history of expectations is reset by calling {@link TestRegistration#clearEvent}.
     *
     * @return An {@link Expectation} object representing the expected event not being triggered.
     */
    @NonNull
    public Expectation expectNotTriggered() {
        return new TriggeredExpectation(false);
    }

    private void clearEvent() {
        mExpectedEvent = new RegEvent(RegEvent.Type.NONE);
        mEventRecords.clear();
    }

    /**
     * Returns a predicate that represents the provided predicate if it is not null,
     * otherwise returns a predicate that always evaluates to true.
     *
     * @param matcher The predicate to be checked for null.
     * @return A predicate that represents the provided predicate if not null,
     *         otherwise a predicate that always evaluates to true.
     */
    private static <T> Predicate<T> nullToTrue(@Nullable Predicate<T> matcher) {
        return matcher != null ? matcher : m -> true;
    }

    private static void logd(String s) {
        Log.d(Log.TAG, "TestRegistration: " + s);
    }

    /**
     * A timed expectation for waiting for a specific event to occur within a defined time period.
     * This expectation is used to verify if a particular event has occurred within the specified
     * timeout during the test execution.
     */
    private class TimedExpectation extends Expectation {

        private final boolean mExpectTriggered;
        private final int mWaitingTimeMillis;

        TimedExpectation(boolean triggered, int waitingTimeMillis) {
            super();
            mExpectTriggered = triggered;
            mWaitingTimeMillis = waitingTimeMillis;
        }

        private void waitForEvent() {
            if (mExpectTriggered) {
                mLatch.await(mWaitingTimeMillis);
            } else {
                mLatch.awaitTimeout(mWaitingTimeMillis);
            }
        }

        @Override
        public void nothing() {
            mLatch.sleep(mWaitingTimeMillis);
        }

        @Override
        public void registered(@Nullable Predicate<ImsRegistrationAttributes> matcher) {
            mExpectedEvent = new RegEvent.RegisteredEvent(nullToTrue(matcher));
            waitForEvent();
        }

        @Override
        public void registering(@Nullable Predicate<ImsRegistrationAttributes> matcher) {
            mExpectedEvent = new RegEvent.RegisteringEvent(nullToTrue(matcher));
            waitForEvent();
        }

        @Override
        public void deregistered(@Nullable Predicate<ImsReasonInfo> infoMatcher,
                @Nullable Predicate<Integer> actionMatcher,
                @Nullable Predicate<Integer> techMatcher) {

            mExpectedEvent = new RegEvent.DeregisteredEvent(nullToTrue(infoMatcher),
                    nullToTrue(actionMatcher),
                    nullToTrue(techMatcher));
            waitForEvent();
        }

        @Override
        public void technologyChangeFailed(@Nullable Predicate<Integer> transportMatcher,
                @Nullable Predicate<ImsReasonInfo> infoMatcher) {
            mExpectedEvent = new RegEvent.TechnologyChangeFailedEvent(nullToTrue(transportMatcher),
                    nullToTrue(infoMatcher));
            waitForEvent();
        }

        @Override
        public void subscriberAssociatedUriChanged(@Nullable Predicate<Uri[]> matcher) {
            mExpectedEvent = new RegEvent.SubscriberAssociatedUriChangedEvent(nullToTrue(matcher));
            waitForEvent();
        }
    }

    /**
     * An expectation for checking whether a specific event has been triggered before.
     * This expectation is used to verify if a particular event has occurred during the test
     * execution.
     */
    private class TriggeredExpectation extends Expectation {

        private final boolean mExpectTriggered;

        TriggeredExpectation(boolean triggered) {
            super();
            mExpectTriggered = triggered;
        }

        private boolean checkRecordNull(@Nullable RegEvent.EventRecord record) {
            if (mExpectTriggered) {
                if (record == null) {
                    fail("Expected event has not been triggered.");
                }
                return false;    // Pass, continue to check parameters.
            } else {
                if (record != null) {
                    fail("Unexpected event has been triggered.");
                }
                return true;    // Pass, no need to check parameters
            }
        }

        private void failOnUnexpectedParameters() {
            fail("The event has been triggered with unexpected parameters");
        }

        @Override
        public void nothing() {}

        @Override
        public void registered(@Nullable Predicate<ImsRegistrationAttributes> matcher) {
            final RegEvent.EventRecord record = mEventRecords.get(RegEvent.Type.REGISTERED);

            if (checkRecordNull(record)) {
                return;
            }

            Object param = record.getParam(1);
            if (param != null && !nullToTrue(matcher).test((ImsRegistrationAttributes) param)) {
                failOnUnexpectedParameters();
            }
        }

        @Override
        public void registering(@Nullable Predicate<ImsRegistrationAttributes> matcher) {
            final RegEvent.EventRecord record = mEventRecords.get(RegEvent.Type.REGISTERING);

            if (checkRecordNull(record)) {
                return;
            }

            Object param = record.getParam(1);
            if (param != null && !nullToTrue(matcher).test((ImsRegistrationAttributes) param)) {
                failOnUnexpectedParameters();
            }
        }

        @Override
        public void deregistered(@Nullable Predicate<ImsReasonInfo> infoMatcher,
                @Nullable Predicate<Integer> actionMatcher,
                @Nullable Predicate<Integer> techMatcher) {
            final RegEvent.EventRecord record = mEventRecords.get(RegEvent.Type.DEREGISTERED);

            if (checkRecordNull(record)) {
                return;
            }

            Object param1 = record.getParam(1);
            Object param2 = record.getParam(2);
            Object param3 = record.getParam(3);

            if ((param1 != null && !nullToTrue(infoMatcher).test((ImsReasonInfo) param1))
                    || (param2 != null && !nullToTrue(actionMatcher).test((Integer) param2))
                    || (param3 != null && !nullToTrue(techMatcher).test((Integer) param3))) {
                failOnUnexpectedParameters();
            }
        }

        @Override
        public void technologyChangeFailed(@Nullable Predicate<Integer> transportMatcher,
                @Nullable Predicate<ImsReasonInfo> infoMatcher) {
            final RegEvent.EventRecord record =
                    mEventRecords.get(RegEvent.Type.TECHNOLOGY_CHANGE_FAILED);

            if (checkRecordNull(record)) {
                return;
            }

            Object param1 = record.getParam(1);
            Object param2 = record.getParam(2);

            if ((param1 != null && !nullToTrue(transportMatcher).test((Integer) param1))
                    || (param2 != null && !nullToTrue(infoMatcher).test((ImsReasonInfo) param2))) {
                failOnUnexpectedParameters();
            }
        }

        @Override
        public void subscriberAssociatedUriChanged(@Nullable Predicate<Uri[]> matcher) {
            final RegEvent.EventRecord record =
                    mEventRecords.get(RegEvent.Type.SUBSCRIBER_ASSOCIATED_URI_CHANGED);

            if (checkRecordNull(record)) {
                return;
            }

            Object param = record.getParam(1);

            if (param != null && !nullToTrue(matcher).test((Uri[]) param)) {
                failOnUnexpectedParameters();
            }
        }
    }

    private class RegistrationListener implements ImsRegistrationWrapper.RegistrationListener {
        /**
         * Notifies when the IMS Provider is registered to the IMS network
         * with corresponding attributes.
         *
         * @param attributes The attributes associated with this IMS registration.
         */
        @Override
        public void onRegistered(@NonNull ImsRegistrationAttributes attributes) {
            logd("onRegistered::attributes:" + attributes.toString());

            mEventRecords.put(RegEvent.Type.REGISTERED, new RegEvent.EventRecord(attributes));

            if (mExpectedEvent.is(RegEvent.Type.REGISTERED)
                    && ((RegEvent.RegisteredEvent) mExpectedEvent).matchParameter(attributes)) {
                mLatch.countDownAndInit();
            }
        }

        /**
         * Notifies when the IMS Provider is trying to register the IMS network.
         *
         * @param attributes The attributes associated with this IMS registration.
         */
        @Override
        public void onRegistering(@NonNull ImsRegistrationAttributes attributes) {
            logd("onRegistering::attributes:" + attributes.toString());

            mEventRecords.put(RegEvent.Type.REGISTERING, new RegEvent.EventRecord(attributes));

            if (mExpectedEvent.is(RegEvent.Type.REGISTERING)
                    && ((RegEvent.RegisteringEvent) mExpectedEvent).matchParameter(attributes)) {
                mLatch.countDownAndInit();
            }
        }

        /**
         * Notifies when the IMS Provider is unregistered from the IMS network.
         *
         * @param info            the {@link ImsReasonInfo} associated with why registration was
         *                        disconnected.
         * @param suggestedAction the expected behavior of radio protocol stack.
         * @param imsRadioTech    the network type on which IMS registration has failed.
         */
        @Override
        public void onDeregistered(@NonNull ImsReasonInfo info, int suggestedAction,
                int imsRadioTech) {
            logd("onDeregistered::info:" + info.toString() + ", suggestedAction:"
                    + suggestedAction + ", imsRadioTech:" + imsRadioTech);

            mEventRecords.put(RegEvent.Type.DEREGISTERED,
                    new RegEvent.EventRecord(info, suggestedAction, imsRadioTech));

            if (mExpectedEvent.is(RegEvent.Type.DEREGISTERED)
                    && ((RegEvent.DeregisteredEvent) mExpectedEvent).matchParameter(info,
                        suggestedAction, imsRadioTech)) {
                mLatch.countDownAndInit();
            }
        }

        /**
         * A failure has occurred when trying to handover registration to another technology type.
         *
         * @param imsTransportType The transport type that has failed to handover registration to.
         * @param info             A {@link ImsReasonInfo} that identifies the reason for failure.
         */
        @Override
        public void onTechnologyChangeFailed(int imsTransportType, @NonNull ImsReasonInfo info) {
            logd("onTechnologyChangeFailed::imsTransportType:" + imsTransportType
                    + ", info:" + info.toString());

            mEventRecords.put(RegEvent.Type.TECHNOLOGY_CHANGE_FAILED,
                    new RegEvent.EventRecord(imsTransportType, info));

            if (mExpectedEvent.is(RegEvent.Type.TECHNOLOGY_CHANGE_FAILED)
                    && ((RegEvent.TechnologyChangeFailedEvent) mExpectedEvent).matchParameter(
                            imsTransportType, info)) {
                mLatch.countDownAndInit();
            }
        }

        /**
         * Returns a list of subscriber {@link Uri}s associated with this IMS subscription when
         * it changes. Per RFC3455, an associated URI is a URI that the service provider has
         * allocated to a user for their own usage. A user's phone number is typically one of the
         * associated URIs.
         *
         * @param uris new array of subscriber {@link Uri}s that are associated with this IMS
         *             subscription.
         */
        @Override
        public void onSubscriberAssociatedUriChanged(@Nullable Uri[] uris) {
            logd("onSubscriberAssociatedUriChanged::uris:" + Arrays.toString(uris));

            mEventRecords.put(RegEvent.Type.SUBSCRIBER_ASSOCIATED_URI_CHANGED,
                    new RegEvent.EventRecord(uris));

            if (mExpectedEvent.is(RegEvent.Type.SUBSCRIBER_ASSOCIATED_URI_CHANGED)
                    && ((RegEvent.SubscriberAssociatedUriChangedEvent) mExpectedEvent)
                        .matchParameter(uris)) {
                mLatch.countDownAndInit();
            }
        }
    }
}
