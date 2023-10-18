/*
 * Copyright (C) 2021 The Android Open Source Project
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
package com.android.imsstack.imsservice.sipcontroller.sipdelegate;

import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateStateCallback;
import android.annotation.NonNull;
import java.util.Set;

/**
 * This class holds the data related sip delegate request done by the framework/app.
 */
public class SipDelegateRequestData {
    /**
     * The subscription id for which sip delegate request is done.
     */
    int mSubscriptionId;
    /**
     * The list of feature tags requested by an application.
     */
    private final Set<String> mRequestedFeatureTags;
    /**
     * The call back object to update delegate state to app/framework.
     */
    @NonNull
    DelegateStateCallback mDelegateStateCallback;
    /**
     * The call back object to update message state to app/framework.
     */
    @NonNull
    DelegateMessageCallback mDelegateMessageCallback;
    /**
     * The default constructor for creating sip delegate request.
     * @param subscriptionId The subscription id for which sip delegate request is done.
     * @param featureTags The list of feature tags requested by an application.
     * @param delegateStateCallback The call back object to update delegate state to app/framework.
     * @param delegateMessageCallback The call back object to update message state to app/framework.
     */
    public SipDelegateRequestData(int subscriptionId, Set<String> featureTags,
            @NonNull DelegateStateCallback delegateStateCallback,
            @NonNull DelegateMessageCallback delegateMessageCallback) {
        mSubscriptionId = subscriptionId;
        mRequestedFeatureTags = featureTags;
        mDelegateStateCallback = delegateStateCallback;
        mDelegateMessageCallback = delegateMessageCallback;
    }
    /**
     *  Method to get subscription id for which sip delegate request is done.
     * @return the subscription id associated with request.
     */
    public int getSubscriptionId() {
        return mSubscriptionId;
    }
    /**
     * Method get the delegate state call back associated with delegate request.
     * @return the state callback.
     */
    @NonNull
    public DelegateStateCallback getDelegateStateCallback() {
        return mDelegateStateCallback;
    }
    /**
     * Method to get the message call back associated with delegate request.
     * @return the message call back.
     */
    @NonNull
    public DelegateMessageCallback getDelegateMessageCallback() {
        return mDelegateMessageCallback;
    }
    /**
     * Get the requested feature tag list
     * @return feature tag requested list
     */
    public Set<String> getRequestedFeatureTags() {
        return mRequestedFeatureTags;
    }
}
