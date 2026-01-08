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

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;

import androidx.annotation.NonNull;

import com.android.ims.internal.IImsCallSession;

import java.util.function.IntPredicate;
import java.util.function.Predicate;

/** Expects call events are triggered or not with specific conditions. */
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

    /** Expects there's an incoming call with the specific extras. */
    public abstract void incomingCall(@NonNull Predicate<Bundle> extrasMatcher);

    /** Expects there's an incoming call with any extras. */
    public final void incomingCall() {
        incomingCall(extras -> true);
    }

    /** Expects the call initiation is failed with the specific reason. */
    public abstract void initiatingFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call initiation is failed with any reason. */
    public final void initiatingFailed() {
        initiatingFailed(reason -> true);
    }

    /** Expects the call is progressing with the specific stream media profile. */
    public abstract void progressing(@NonNull Predicate<ImsStreamMediaProfile> profileMatcher);

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

    /** Expects the call is held with the specific call profile. */
    public abstract void held(@NonNull Predicate<ImsCallProfile> profileMatcher);

    /** Expects the call is held with any call profile. */
    public final void held() {
        held(profile -> true);
    }

    /** Expects the call hold is failed with the specific reason. */
    public abstract void holdFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call hold is failed with any reason. */
    public final void holdFailed() {
        holdFailed(reason -> true);
    }

    /** Expects the call is resumed with the specific call profile. */
    public abstract void resumed(@NonNull Predicate<ImsCallProfile> profileMatcher);

    /** Expects the call is resumed with any call profile. */
    public final void resumed() {
        resumed(profile -> true);
    }

    /** Expects the call resume is failed with the specific reason. */
    public abstract void resumeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call resume is failed with any reason. */
    public final void resumeFailed() {
        resumeFailed(reason -> true);
    }

    /** Expects the call merge has started. */
    public abstract void mergeStarted(
            @NonNull Predicate<IImsCallSession> sessionMatcher,
            @NonNull Predicate<ImsCallProfile> profileMatcher);

    /** Expects the call merge has started. */
    public final void mergeStarted() {
        mergeStarted((session) -> true, (profile) -> true);
    }

    /** Expects the call merge is complete. */
    public abstract void mergeComplete(@NonNull Predicate<IImsCallSession> sessionMatcher);

    /** Expects the call merge is complete. */
    public final void mergeComplete() {
        mergeComplete(session -> true);
    }

    /** Expects the call merge is failed with the specific reason. */
    public abstract void mergeFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call merge is failed with any reason. */
    public final void mergeFailed() {
        mergeFailed(reason -> true);
    }

    /** Expects the call is updated with the specific call profile. */
    public abstract void updated(@NonNull Predicate<ImsCallProfile> profileMatcher);

    /** Expects the call is updated with any call profile. */
    public final void updated() {
        updated(profile -> true);
    }

    /** Expects the call update is failed with the specific reason. */
    public abstract void updateFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call update is failed with any reason. */
    public final void updateFailed() {
        updateFailed(reason -> true);
    }

    /** Expects the request to invite participants is delivered. */
    public abstract void inviteParticipantsRequestDelivered();

    /** Expects the request to invite participants is failed with the specific reason. */
    public abstract void inviteParticipantsRequestFailed(
            @NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the request to invite participants is failed with any reason. */
    public final void inviteParticipantsRequestFailed() {
        inviteParticipantsRequestFailed(reason -> true);
    }

    /** Expects the request to remove participants is delivered. */
    public abstract void removeParticipantsRequestDelivered();

    /** Expects the request to remove participants is failed with the specific reason. */
    public abstract void removeParticipantsRequestFailed(
            @NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the request to remove participants is failed with any reason. */
    public final void removeParticipantsRequestFailed() {
        removeParticipantsRequestFailed(reason -> true);
    }

    /** Expects the conference state is updated. */
    public abstract void conferenceStateUpdated(
            @NonNull Predicate<ImsConferenceState> stateMatcher);

    /** Expects the conference state is updated. */
    public final void conferenceStateUpdated() {
        conferenceStateUpdated(state -> true);
    }

    /** Expects a USSD message is received. */
    public abstract void ussdMessageReceived(
            @NonNull IntPredicate modeMatcher, @NonNull Predicate<String> ussdMessageMatcher);

    /** Expects an RTT message is received. */
    public abstract void rttMessageReceived(@NonNull Predicate<String> rttMessageMatcher);

    /** Expects an RTT message is received. */
    public final void rttMessageReceived() {
        rttMessageReceived(message -> true);
    }

    /** Expects the call is transferred. */
    public abstract void transferred();

    /** Expects the call transfer is failed with the specific reason. */
    public abstract void transferFailed(@NonNull Predicate<ImsReasonInfo> reasonMatcher);

    /** Expects the call transfer is failed with any reason. */
    public final void transferFailed() {
        transferFailed(reason -> true);
    }
}
