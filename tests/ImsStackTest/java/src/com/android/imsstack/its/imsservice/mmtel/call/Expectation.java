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

import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;

import androidx.annotation.NonNull;

import java.util.function.IntPredicate;
import java.util.function.Predicate;

/** Expects events are triggered or not with specific conditions. */
public abstract class Expectation {
    private boolean mExpectToBeTriggered = true;

    /** Set to expect a negative result. */
    public final @NonNull Expectation not() {
        mExpectToBeTriggered = !mExpectToBeTriggered;
        return this;
    }

    protected boolean isExpectToBeTriggered() {
        return mExpectToBeTriggered;
    }

    /** Expects nothing. */
    public abstract void nothing();

    /** Expects there's an incoming call. */
    public abstract void incomingCall();

    /** Expects the call is initiated with the specific call profile. */
    public abstract void initiated(@NonNull Predicate<ImsCallProfile> profileMatcher);

    /** Expects the call is initiated with any call profile. */
    public final void initiated() {
        initiated(profile -> true);
    }

    /** Expects the call is terminated with the specific reason. */
    public abstract void terminated(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call is terminated with any reason. */
    public final void terminated() {
        terminated(reason -> true);
    }

    /** Expects a USSD message is received. */
    public abstract void ussdMessageReceived(
            @NonNull IntPredicate modeMatcher, @NonNull Predicate<String> ussdMessageMatcher);
}
