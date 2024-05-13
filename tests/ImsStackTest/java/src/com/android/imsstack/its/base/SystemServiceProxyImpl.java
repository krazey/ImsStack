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

import android.content.Context;
import android.util.ArrayMap;

import androidx.annotation.NonNull;

import com.android.imsstack.base.SystemServiceProxy;
import com.android.imsstack.base.TelephonyManagerProxy;

import java.util.Map;

/**
 * An implementation class to access the APIs of the various system managers.
 */
public class SystemServiceProxyImpl implements SystemServiceProxy {
    private final Context mContext;
    private final Map<Class<?>, Object> mManagerProxies = new ArrayMap<>();

    SystemServiceProxyImpl(@NonNull Context context) {
        mContext = context;

        mManagerProxies.put(TelephonyManagerProxy.class, new TelephonyManagerProxyImpl(mContext));
        mManagerProxies.put(CarrierConfigManagerProxy.class,
                new CarrierConfigManagerProxyImpl(mContext));
        mManagerProxies.put(SubscriptionManagerProxy.class, new SubscriptionManagerProxyImpl());
        mManagerProxies.put(ConnectivityManagerProxy.class, new ConnectivityManagerProxyImpl());
        mManagerProxies.put(IpSecManagerProxy.class, new IpSecManagerProxyImpl(mContext));
        mManagerProxies.put(LocationManagerProxy.class, new LocationManagerProxyImpl());
        mManagerProxies.put(SensorManagerProxy.class, new SensorManagerProxyImpl(mContext));
        mManagerProxies.put(SmsManagerProxy.class, new SmsManagerProxyImpl());
        mManagerProxies.put(ImsManagerProxy.class, new ImsManagerProxyImpl());
    }

    /**
     * Returns a specific system service corresponding to the given class.
     *
     * @param clazz A requested class name.
     * @return A system service object corresponding to the given class.
     */
    @Override
    @SuppressWarnings("unchecked")
    public <T> T getSystemService(Class<T> clazz) {
        Object proxy = mManagerProxies.get(clazz);
        if (proxy == null) {
            throw new IllegalArgumentException("Unknown system proxy: " + clazz);
        }
        return (T) proxy;
    }
}
