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

import android.util.SparseArray;

import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;

/**
 * An implementation class to access the {@link ImsManager}.
 */
public class ImsManagerProxyImpl implements ImsManagerProxy {
    private final SparseArray<ImsMmTelManagerProxy> mImsMmTelManagers = new SparseArray<>();
    private final SparseArray<ProvisioningManagerProxy> mProvisioningManagers = new SparseArray<>();

    @Override
    public @Nullable ImsMmTelManagerProxy getImsMmTelManagerProxy(int subId) {
        return mImsMmTelManagers.get(subId);
    }

    @Override
    public @Nullable ProvisioningManagerProxy getProvisioningManagerProxy(int subId) {
        return mProvisioningManagers.get(subId);
    }

    /**
     * Sets the {@link ImsMmTelManagerProxy} for the specified subscription.
     *
     * @param subId The subscription id.
     * @param proxy The proxy object.
     */
    public void setImsMmTelManagerProxy(int subId, ImsMmTelManagerProxy proxy) {
        mImsMmTelManagers.put(subId, proxy);
    }

    /**
     * Sets the {@link ProvisioningManagerProxy} for the specified subscription.
     *
     * @param subId The subscription id.
     * @param proxy The proxy object.
     */
    public void setProvisioningManagerProxy(int subId, ProvisioningManagerProxy proxy) {
        mProvisioningManagers.put(subId, proxy);
    }
}
