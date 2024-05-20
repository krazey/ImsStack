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

import android.net.Uri;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.RegistrationManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.function.Predicate;

/**
 * This class represents events that can occur during registration.
 * It includes matchers to validate whether actual parameters align with expected values.
 */
public class RegEvent {

    /**
     * Types of the event.
     */
    public enum Type {
        NONE,
        REGISTERED,
        REGISTERING,
        DEREGISTERED,
        TECHNOLOGY_CHANGE_FAILED,
        SUBSCRIBER_ASSOCIATED_URI_CHANGED,
        TRIGGER_DEREGISTRATION
    }

    @NonNull
    private final Type mType;

    public RegEvent(@NonNull Type type) {
        mType = type;
    }

    /**
     * Checks if this event's type aligns with the specified event type.
     *
     * @param type The event type to compare.
     * @return True if this event's type equals the provided type.
     *         Always false if the type is {@link Type#NONE}.
     */
    public final boolean is(Type type) {
        return mType != Type.NONE && mType == type;
    }

    /**
     * Represents an EventRecord to store event parameters.
     * This class allows for the flexible construction of records with up to three parameters.
     * <p>
     * Instances of this class are immutable and can store up to three optional parameters
     * of any type. These parameters can be accessed using the {@code getParam} method.
     * If a parameter is not provided, its corresponding value will be {@code null}.
     * </p>
     */
    public static class EventRecord {
        @Nullable
        private final Object mParam1;
        @Nullable
        private final Object mParam2;
        @Nullable
        private final Object mParam3;

        /**
         * Constructs a new EventRecord with no parameters.
         */
        public EventRecord() {
            this(null, null, null);
        }

        /**
         * Constructs a new EventRecord with the given first parameter.
         *
         * @param param1 The first parameter.
         */
        public EventRecord(@Nullable Object param1) {
            this(param1, null, null);
        }

        /**
         * Constructs a new EventRecord with the given first and second parameters.
         *
         * @param param1 The first parameter.
         * @param param2 The second parameter.
         */
        public EventRecord(@Nullable Object param1, @Nullable Object param2) {
            this(param1, param2, null);
        }

        /**
         * Constructs a new EventRecord with the given first, second, and third parameters.
         *
         * @param param1 The first parameter.
         * @param param2 The second parameter.
         * @param param3 The third parameter.
         */
        public EventRecord(@Nullable Object param1, @Nullable Object param2,
                @Nullable Object param3) {
            mParam1 = param1;
            mParam2 = param2;
            mParam3 = param3;
        }

        /**
         * Returns the parameter at the specified index.
         *
         * @param index The index of the parameter to retrieve (1-based).
         * @return The parameter at the specified index, or null if the index is out of bounds or
         * the parameter is not set.
         */
        @Nullable
        public Object getParam(int index) {
            return switch (index) {
                case 1 -> mParam1;
                case 2 -> mParam2;
                case 3 -> mParam3;
                default -> null;
            };
        }
    }

    /**
     * Represents an event indicating a registration, with specific criteria
     * to match the registration attributes.
     */
    public static class RegisteredEvent extends RegEvent {
        @NonNull Predicate<ImsRegistrationAttributes> mMatcher;

        /**
         * Constructs a {@code RegisteredEvent} with the specified matcher.
         *
         * @param matcher The predicate to match the registration attributes.
         */
        public RegisteredEvent(@NonNull Predicate<ImsRegistrationAttributes> matcher) {
            super(Type.REGISTERED);
            mMatcher = matcher;
        }

        /**
         * Checks if the parameters match with the expectation.
         *
         * @param attributes The registration attributes to be tested.
         * @return {@code true} if the parameters match the expectation, {@code false} otherwise.
         */
        public boolean matchParameter(@NonNull ImsRegistrationAttributes attributes) {
            return mMatcher.test(attributes);
        }
    }

    /**
     * Represents an event indicating a registering process, with specific criteria
     * to match the registration attributes.
     */
    public static class RegisteringEvent extends RegEvent {
        @NonNull Predicate<ImsRegistrationAttributes> mMatcher;

        /**
         * Constructs a {@code RegisteringEvent} with the specified matcher.
         *
         * @param matcher The predicate to match the registration attributes.
         */
        public RegisteringEvent(@NonNull Predicate<ImsRegistrationAttributes> matcher) {
            super(Type.REGISTERING);
            mMatcher = matcher;
        }

        /**
         * Checks if the parameters match with the expectation.
         *
         * @param attributes The registration attributes to be tested.
         * @return {@code true} if the parameters match the expectation, {@code false} otherwise.
         */
        public boolean matchParameter(@NonNull ImsRegistrationAttributes attributes) {
            return mMatcher.test(attributes);
        }
    }

    /**
     * Represents an event indicating a deregistration, with specific criteria
     * to match the deregistration reason, suggested action, and IMS radio technology.
     */
    public static class DeregisteredEvent extends RegEvent {
        @NonNull Predicate<ImsReasonInfo> mInfoMatcher;
        @NonNull Predicate<Integer> mActionMatcher;
        @NonNull Predicate<Integer> mTechMatcher;

        /**
         * Constructs a {@code DeregisteredEvent} with the specified matchers.
         *
         * @param infoMatcher    The predicate to match the deregistration reason information.
         * @param actionMatcher  The predicate to match the suggested action after deregistration.
         * @param techMatcher    The predicate to match the IMS radio technology during
         */
        public DeregisteredEvent(@NonNull Predicate<ImsReasonInfo> infoMatcher,
                @NonNull Predicate<Integer> actionMatcher,
                @NonNull Predicate<Integer> techMatcher) {
            super(Type.DEREGISTERED);
            mInfoMatcher = infoMatcher;
            mActionMatcher = actionMatcher;
            mTechMatcher = techMatcher;
        }

        /**
         * Checks if the parameters match with the expectation.
         *
         * @param info              The deregistration reason information to be tested.
         * @param suggestedAction   The suggested action after deregistration to be tested.
         * @param imsRadioTech      The IMS radio technology during deregistration to be tested.
         * @return {@code true} if the parameters match the expectation, {@code false} otherwise.
         */
        public boolean matchParameter(@NonNull ImsReasonInfo info,
                @RegistrationManager.SuggestedAction int suggestedAction,
                @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
            return mInfoMatcher.test(info) && mActionMatcher.test(suggestedAction)
                    && mTechMatcher.test(imsRadioTech);
        }
    }

    /**
     * Represents an event indicating a technology change failure, which includes
     * specific criteria to match the transport type and reason information.
     */
    public static class TechnologyChangeFailedEvent extends RegEvent {
        @NonNull private final Predicate<Integer> mTransportMatcher;
        @NonNull private final Predicate<ImsReasonInfo> mInfoMatcher;

        /**
         * Constructs a {@code TechnologyChangeFailedEvent} with the specified matchers.
         *
         * @param transportMatcher The predicate to match the technology change failure transport
         *                         type.
         * @param infoMatcher      The predicate to match the technology change failure reason
         *                         information.
         */
        public TechnologyChangeFailedEvent(@NonNull Predicate<Integer> transportMatcher,
                @NonNull Predicate<ImsReasonInfo> infoMatcher) {
            super(Type.TECHNOLOGY_CHANGE_FAILED);
            mTransportMatcher = transportMatcher;
            mInfoMatcher = infoMatcher;
        }

        /**
         * Checks if the parameters match with the expectation.
         *
         * @param transport The transport to be tested.
         * @param info      The reason info to be tested.
         * @return {@code true} if the parameters match the expectation, {@code false} otherwise.
         */
        public boolean matchParameter(int transport, @NonNull ImsReasonInfo info) {
            return mTransportMatcher.test(transport) && mInfoMatcher.test(info);
        }
    }

    /**
     * Represents an event indicating a change in subscriber associated URIs, with specific criteria
     * to match the associated URIs.
     */
    public static class SubscriberAssociatedUriChangedEvent extends RegEvent {
        @NonNull private final Predicate<Uri[]> mUrisMatcher;

        /**
         * Constructs a {@code SubscriberAssociatedUriChangedEvent} with the specified matcher.
         *
         * @param urisMatcher The predicate to match the subscriber associated URIs.
         */
        public SubscriberAssociatedUriChangedEvent(@NonNull Predicate<Uri[]> urisMatcher) {
            super(Type.SUBSCRIBER_ASSOCIATED_URI_CHANGED);
            mUrisMatcher = urisMatcher;
        }

        /**
         * Checks if the parameters match with the expectation.
         *
         * @param uris The URIs to be tested.
         * @return {@code true} if the parameters match the expectation, {@code false} otherwise.
         */
        public boolean matchParameter(@Nullable Uri[] uris) {
            return mUrisMatcher.test(uris);
        }
    }
}
