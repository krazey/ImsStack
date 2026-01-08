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

import androidx.annotation.Nullable;

import java.util.function.Predicate;

/**
 * Represents an expectation for registration events to be triggered or not with specific
 * conditions.
 */
public abstract class Expectation {

    /**
     * Indicates that no specific expectation is set.
     * This method is used when no particular outcome is expected.
     */
    public abstract void nothing();

    /**
     * Expects a registered event with specific attributes.
     *
     * @param matcher The predicate to match the registration attributes.
     *                May be {@code null} if no specific attributes matching is required.
     */
    public abstract void registered(
            @Nullable Predicate<ImsRegistrationAttributes> matcher);

    /**
     * Expects a registering event with specific attributes.
     *
     * @param matcher The predicate to match the registration attributes.
     *                May be {@code null} if no specific attributes matching is required.
     */
    public abstract void registering(
            @Nullable Predicate<ImsRegistrationAttributes> matcher);

    /**
     * Expects a deregistered event with specific reason, action, and technology.
     *
     * @param infoMatcher    The predicate to match the deregistration reason info.
     *                       May be {@code null} if no specific reason info matching is required.
     * @param actionMatcher  The predicate to match the deregistration action.
     *                       May be {@code null} if no specific action matching is required.
     * @param techMatcher    The predicate to match the deregistration technology.
     *                       May be {@code null} if no specific technology matching is required.
     */
    public abstract void deregistered(@Nullable Predicate<ImsReasonInfo> infoMatcher,
            @Nullable Predicate<Integer> actionMatcher, @Nullable Predicate<Integer> techMatcher);

    /**
     * Expects a technology change failure event with specific transport type and reason info.
     *
     * @param transportMatcher The predicate to match the technology change failure transport type.
     *                         May be {@code null} if no specific transport matching is required.
     * @param infoMatcher      The predicate to match the technology change failure reason info.
     *                         May be {@code null} if no specific info matching is required.
     */
    public abstract void technologyChangeFailed(@Nullable Predicate<Integer> transportMatcher,
            @Nullable Predicate<ImsReasonInfo> infoMatcher);

    /**
     * Expects a subscriber associated URI change event with specific URIs.
     *
     * @param matcher    The predicate to match the subscriber associated URIs.
     *                   May be {@code null} if no specific uris matching is required.
     */
    public abstract void subscriberAssociatedUriChanged(@Nullable Predicate<Uri[]> matcher);

    /**
      * Expects a registered event without specific matching.
      */
    public final void registered() {
        registered(null);
    }

    /**
     * Expects a registering event without specific matching.
     */
    public final void registering() {
        registering(null);
    }

    /**
     * Expects a deregistered event without specific matching.
     */
    public final void deregistered() {
        deregistered(null, null, null);
    }

    /**
     * Expects a technology change failure event without specific matching.
     */
    public final void technologyChangeFailed() {
        technologyChangeFailed(null, null);
    }

    /**
     * Expects a subscriber associated URI change event without specific matching.
     */
    public final void subscriberAssociatedUriChanged() {
        subscriberAssociatedUriChanged(null);
    }
}
