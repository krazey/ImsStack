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
package com.android.imsstack.its.base;

import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.PhoneNumberSource;
import android.util.SparseArray;

import androidx.annotation.NonNull;

import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;

/**
 * An implementation class to access the {@link SubscriptionManager}.
 */
public class SubscriptionManagerProxyImpl implements SubscriptionManagerProxy {
    private final SparseArray<Integer> mSubIdPerSlot = new SparseArray<>();
    private final SparseArray<String> mPhoneNumberPerSubscription = new SparseArray<>();

    SubscriptionManagerProxyImpl() {
        setDefaultValues();
    }

    @Override
    public boolean isUsableSubscriptionId(int subId) {
        return SubscriptionManager.isUsableSubscriptionId(subId);
    }

    @Override
    public boolean isValidSubscriptionId(int subId) {
        return SubscriptionManager.isValidSubscriptionId(subId);
    }

    @Override
    public int getDefaultDataSubscriptionId() {
        return TestConstants.SUB_ID_1;
    }

    @Override
    public int getSlotIndex(int subId) {
        int index = mSubIdPerSlot.indexOfValue(subId);
        return index >= 0 ? mSubIdPerSlot.keyAt(index) : TestConstants.INVALID_SLOT_ID;
    }

    @Override
    public int getSubscriptionId(int slotIndex) {
        return mSubIdPerSlot.get(slotIndex, SubscriptionManager.INVALID_SUBSCRIPTION_ID);
    }

    @Override
    public @NonNull String getPhoneNumber(int subId, @PhoneNumberSource int source) {
        return mPhoneNumberPerSubscription.get(subId, "");
    }

    /**
     * Sets the subscription identifier for the given slot.
     *
     * @param slotIndex The slot index.
     * @param subId The subscription identifier to be configured.
     */
    public void setSubscriptionId(int slotIndex, int subId) {
        mSubIdPerSlot.put(slotIndex, subId);
    }

    /**
     * Sets the phone number for the given subscription.
     *
     * @param subId The subscription identifier.
     * @param phoneNumber The phone number to be configured.
     */
    public void setPhoneNumber(int subId, @NonNull String phoneNumber) {
        mPhoneNumberPerSubscription.put(subId, phoneNumber);
    }

    /**
     * Sets the default values for this object.
     */
    public void setDefaultValues() {
        // Set the default subscription ids.
        setSubscriptionId(TestConstants.SLOT0, TestConstants.SUB_ID_1);
        setSubscriptionId(TestConstants.SLOT1, TestConstants.SUB_ID_2);

        // Set the default phone numbers.
        setPhoneNumber(TestConstants.SUB_ID_1, TestConstants.PHONE_NUMBER_1);
        setPhoneNumber(TestConstants.SUB_ID_2, TestConstants.PHONE_NUMBER_2);
    }
}
