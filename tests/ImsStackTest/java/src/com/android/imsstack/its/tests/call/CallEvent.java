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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.function.Predicate;

/**
 * This class represents events that can occur during calls.
 * It stores matchers to validate whether actual parameters align with expected values.
 */
public final class CallEvent<T1, T2, T3> {
    private final @NonNull Type mEventType;
    private final @Nullable Predicate<T1> mParam1Matcher;
    private final @Nullable Predicate<T2> mParam2Matcher;
    private final @Nullable Predicate<T3> mParam3Matcher;

    public CallEvent(@NonNull Type eventType) {
        this(eventType, null, null);
    }

    public CallEvent(
            @NonNull Type eventType,
            @Nullable Predicate<T1> param1Matcher) {
        this(eventType, param1Matcher, null);
    }

    public CallEvent(
            @NonNull Type eventType,
            @Nullable Predicate<T1> param1Matcher,
            @Nullable Predicate<T2> param2Matcher) {
        this(eventType, param1Matcher, param2Matcher, null);
    }

    public CallEvent(
            @NonNull Type eventType,
            @Nullable Predicate<T1> param1Matcher,
            @Nullable Predicate<T2> param2Matcher,
            @Nullable Predicate<T3> param3Matcher) {
        mEventType = eventType;
        mParam1Matcher = param1Matcher;
        mParam2Matcher = param2Matcher;
        mParam3Matcher = param3Matcher;
    }

    @Override
    public String toString() {
        return mEventType.toString();
    }

    /**
     * Checks if this event's type aligns with the specified event type.
     *
     * @return True if this event's type equals the provided type.
     *         Always false if the type is {@link Type.NONE}.
     */
    public boolean is(Type eventType) {
        return mEventType != Type.NONE && mEventType == eventType;
    }

    /** Checks if the parameters match with the expectation. */
    public boolean matchParameters(
            @Nullable T1 param1, @Nullable T2 param2, @Nullable T3 param3) {
        return (mParam1Matcher == null || mParam1Matcher.test(param1))
                && (mParam2Matcher == null || mParam2Matcher.test(param2))
                && (mParam3Matcher == null || mParam3Matcher.test(param3));
    }

    /** Types of the event. */
    public enum Type {
        NONE,
        MMTEL_INCOMING_CALL,
        SESSION_INITIATING_FAILED,
        SESSION_PROGRESSING,
        SESSION_INITIATED,
        SESSION_TERMINATED,
        SESSION_HELD,
        SESSION_HOLD_FAILED,
        SESSION_RESUMED,
        SESSION_RESUME_FAILED,
        SESSION_MERGE_STARTED,
        SESSION_MERGE_COMPLETE,
        SESSION_MERGE_FAILED,
        SESSION_UPDATED,
        SESSION_UPDATE_FAILED,
        SESSION_INVITE_PARTICIPANTS_REQUEST_DELIVERED,
        SESSION_INVITE_PARTICIPANTS_REQUEST_FAILED,
        SESSION_REMOVE_PARTICIPANTS_REQUEST_DELIVERED,
        SESSION_REMOVE_PARTICIPANTS_REQUEST_FAILED,
        SESSION_CONFERENCE_STATE_UPDATED,
        SESSION_USSD_MESSAGE_RECEIVED,
        SESSION_RTT_MESSAGE_RECEIVED,
        SESSION_TRANSFERRED,
        SESSION_TRANSFER_FAILED,
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
            this.param3 = param3;
        }
    }
}
