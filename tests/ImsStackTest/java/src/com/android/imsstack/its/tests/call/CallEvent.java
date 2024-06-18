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

import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.function.IntPredicate;
import java.util.function.Predicate;

/**
 * This class represents events that can occur during calls.
 * It includes matchers to validate whether actual parameters align with expected values.
 */
public class CallEvent {
    private final @NonNull Type mEventType;

    public CallEvent(@NonNull Type eventType) {
        mEventType = eventType;
    }

    /**
     * Checks if this event's type aligns with the specified event type.
     *
     * @return True if this event's type equals the provided type.
     *         Always false if the type is {@link Type.NONE}.
     */
    public final boolean is(Type eventType) {
        return mEventType != Type.NONE && mEventType == eventType;
    }

    /** Types of the event. */
    public enum Type {
        NONE,
        MMTEL_INCOMING_CALL,
        SESSION_INITIATED,
        SESSION_TERMINATED,
        SESSION_USSD_MESSAGE_RECEIVED,
    }

    /** Can be used to store event parameters. */
    public static class EventRecord {
        public @Nullable Object param1;
        public @Nullable Object param2;
        public @Nullable Object param3;

        public EventRecord() {
            this(null);
        }

        public EventRecord(Object param1) {
            this(param1, null);
        }

        public EventRecord(Object param1, Object param2) {
            this(param1, param2, null);
        }

        public EventRecord(Object param1, Object param2, Object param3) {
            this.param1 = param1;
            this.param2 = param2;
            this.param2 = param3;
        }
    }

    /** Event from {@link ImsMmTelFeatureWrapper.IncomingCallListener#onIncomingCall}. */
    public static class MmTelIncomingCallEvent extends CallEvent {
        public MmTelIncomingCallEvent() {
            super(Type.MMTEL_INCOMING_CALL);
        }
    }

    /** Event from {@link IImsCallSessionListener.Stub#callSessionInitiated}. */
    public static class SessionInitiatedEvent extends CallEvent {
        private @NonNull Predicate<ImsCallProfile> mProfileMatcher;

        public SessionInitiatedEvent(@NonNull Predicate<ImsCallProfile> profileMatcher) {
            super(Type.SESSION_INITIATED);
            mProfileMatcher = profileMatcher;
        }

        /** Checks if the parameters match with the expectation. */
        public boolean matchParameter(@Nullable ImsCallProfile profile) {
            return mProfileMatcher.test(profile);
        }
    }

    /** Event from {@link IImsCallSessionListener.Stub#callSessionTerminated}. */
    public static class SessionTerminatedEvent extends CallEvent {
        private @NonNull Predicate<ImsReasonInfo> mReasonMatcher;

        public SessionTerminatedEvent(@NonNull Predicate<ImsReasonInfo> reasonMatcher) {
            super(Type.SESSION_TERMINATED);
            mReasonMatcher = reasonMatcher;
        }

        /** Checks if the parameters match with the expectation. */
        public boolean matchParameter(@Nullable ImsReasonInfo reason) {
            return mReasonMatcher.test(reason);
        }
    }

    /** Event from {@link IImsCallSessionListener.Stub#callSessionUssdMessageReceived}. */
    public static class SessionUssdMessageReceivedEvent extends CallEvent {
        private @NonNull IntPredicate mModeMatcher;
        private @NonNull Predicate<String> mUssdMessageMatcher;

        public SessionUssdMessageReceivedEvent(
                @NonNull IntPredicate modeMatcher, @NonNull Predicate<String> ussdMessageMatcher) {
            super(Type.SESSION_USSD_MESSAGE_RECEIVED);
            mModeMatcher = modeMatcher;
            mUssdMessageMatcher = ussdMessageMatcher;
        }

        /** Checks if the parameters match with the expectation. */
        public boolean matchParameter(int mode, @Nullable String ussdMessage) {
            return mModeMatcher.test(mode) && mUssdMessageMatcher.test(ussdMessage);
        }
    }
}
