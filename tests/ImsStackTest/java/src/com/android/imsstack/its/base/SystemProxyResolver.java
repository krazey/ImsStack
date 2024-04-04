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

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.IpSecManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.LocationManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SensorManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;

/**
 * A helper class to get the system service proxies from the {@link AppContext}.
 */
public class SystemProxyResolver {
    /** Returns the {@link BroadcastReceiverProxyImpl} instance. */
    public static BroadcastReceiverProxyImpl getBroadcastReceiverProxy() {
        return (BroadcastReceiverProxyImpl) AppContext.getInstance().getBroadcastReceiverProxy();
    }

    /** Returns the {@link ContentProviderProxyImpl} instance. */
    public static ContentProviderProxyImpl getContentProviderProxy() {
        return (ContentProviderProxyImpl) AppContext.getInstance().getContentProviderProxy();
    }

    /** Returns the {@link CarrierConfigManagerProxyImpl} instance. */
    public static CarrierConfigManagerProxyImpl getCarrierConfigManagerProxy() {
        return (CarrierConfigManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(CarrierConfigManagerProxy.class);
    }

    /** Returns the {@link ConnectivityManagerProxyImpl} instance. */
    public static ConnectivityManagerProxyImpl getConnectivityManagerProxy() {
        return (ConnectivityManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(ConnectivityManagerProxy.class);
    }

    /** Returns the {@link ImsManagerProxyImpl} instance. */
    public static ImsManagerProxyImpl getImsManagerProxy() {
        return (ImsManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(ImsManagerProxy.class);
    }

    /** Returns the {@link IpSecManagerProxyImpl} instance. */
    public static IpSecManagerProxyImpl getIpSecManagerProxy() {
        return (IpSecManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(IpSecManagerProxy.class);
    }

    /** Returns the {@link LocationManagerProxyImpl} instance. */
    public static LocationManagerProxyImpl getLocationManagerProxy() {
        return (LocationManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(LocationManagerProxy.class);
    }

    /** Returns the {@link SensorManagerProxyImpl} instance. */
    public static SensorManagerProxyImpl getSensorManagerProxy() {
        return (SensorManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(SensorManagerProxy.class);
    }

    /** Returns the {@link SmsManagerProxyImpl} instance. */
    public static SmsManagerProxyImpl getSmsManagerProxy() {
        return (SmsManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(SmsManagerProxy.class);
    }

    /** Returns the {@link SmsManagerProxyImpl} instance of the specified subscription. */
    public static SmsManagerProxyImpl getSmsManagerProxy(int subId) {
        return (SmsManagerProxyImpl) AppContext.getSmsManagerProxy(subId);
    }

    /** Returns the {@link SubscriptionManagerProxyImpl} instance. */
    public static SubscriptionManagerProxyImpl getSubscriptionManagerProxy() {
        return (SubscriptionManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(SubscriptionManagerProxy.class);
    }

    /** Returns the {@link TelephonyManagerProxyImpl} instance. */
    public static TelephonyManagerProxyImpl getTelephonyManagerProxy() {
        return (TelephonyManagerProxyImpl) AppContext.getInstance()
                .getSystemServiceProxy(TelephonyManagerProxy.class);
    }

    /** Returns the {@link TelephonyManagerProxyImpl} instance of the specified subscription. */
    public static TelephonyManagerProxyImpl getTelephonyManagerProxy(int subId) {
        return (TelephonyManagerProxyImpl) AppContext.getTelephonyManagerProxy(subId);
    }

    /** Returns the {@link ImsMmTelManagerProxyImpl} instance of the specified subscription. */
    public static ImsMmTelManagerProxyImpl getImsMmTelManagerProxy(int subId) {
        return (ImsMmTelManagerProxyImpl) getImsManagerProxy().getImsMmTelManagerProxy(subId);
    }

    /** Returns the {@link ProvisioningManagerProxyImpl} instance of the specified subscription. */
    public static ProvisioningManagerProxyImpl getProvisioningManagerProxy(int subId) {
        return (ProvisioningManagerProxyImpl) getImsManagerProxy()
                .getProvisioningManagerProxy(subId);
    }
}
