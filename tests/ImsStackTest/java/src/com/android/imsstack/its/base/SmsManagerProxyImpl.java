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

import android.net.Uri;
import android.telephony.SmsManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;

/**
 * An implementation class to access the {@link SmsManager}.
 */
public class SmsManagerProxyImpl implements SmsManagerProxy {
    private String mSmscAddress;
    private Uri mSmscIdentity;

    SmsManagerProxyImpl() {
        setDefaultValues();
    }

    @Override
    public @NonNull SmsManagerProxy createForSubscriptionId(int subId) {
        return this;
    }

    @Override
    public @Nullable String getSmscAddress() {
        return mSmscAddress;
    }

    @Override
    public @NonNull Uri getSmscIdentity() {
        return mSmscIdentity;
    }

    /**
     * Sets the SMS center address.
     *
     * @param smscAddress The SMS center address.
     */
    public void setSmscAddress(String smscAddress) {
        mSmscAddress = smscAddress;
    }

    /**
     * Sets the SMS center's identity (Public Service Identity).
     *
     * @param smscIdentity The SMS center's identity.
     */
    public void setSmscIdentity(@NonNull Uri smscIdentity) {
        mSmscIdentity = smscIdentity;
    }

    /**
     * Sets the default values for this object.
     */
    public void setDefaultValues() {
        setSmscAddress("1123456789");
        setSmscIdentity(Uri.parse("tel:" + mSmscAddress));
    }
}
