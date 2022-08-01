/*
 * Copyright (C) 2022 The Android Open Source Project
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

package com.android.imsstack.core.agents.dcmif;

import android.content.Context;
import android.net.Network;
import android.os.Message;

public interface IApn {
    public static final int IPCAN_CATEGORY_MOBILE = 0;
    public static final int IPCAN_CATEGORY_WLAN = 1;

    public static final int HANDOVER_UNKNOWN = 10;
    public static final int HANDOVER_START = 11;
    public static final int HANDOVER_SUCCESS = 12;
    public static final int HANDOVER_FAILURE = 13;

    void cleanup();

    /**
     * Set reference of DcSettings object in each apn object.
     * If Apn object refer to DcSettings object directly, it cause Circular reference.
     * So, when DcApn set reference of DcSettings to Apn to prevent circular reference.
     */
    void setSettings(IDcSettings settings);

    /**
     * Return DcSettings object.
     */
    IDcSettings getSettings();

    /**
     * Set reference of DcNetWatcher object in each apn object.
     * If Apn object refer to DcSettings object directly, it cause Circular reference.
     * So, when DcApn set reference of DcNetWatcher to Apn to prevent circular reference.
     */
    void setNetWatcher(IDcNetWatcher netWatcher);

    /**
     * Return DcNetWatcher object.
     */
    IDcNetWatcher getNetWatcher();

    /**
     * Add/Remove Listener to receive ip category chaged event
     */
    void addListener(ApnStateListener listener);
    void removeListener(ApnStateListener listener);

    /**
     * request apn connection to use target apn.
     * There are many conditions before requesting to use target apn to connectivity manager
     * This condition can be different based on operator requirement and apn type
     */
    boolean connect();

    /**
     * request apn disconnection to stop use of target apn.
     */
    boolean disconnect();

    /**
     * Return Apn name value of target apn
     */
    String getApn();

    /**
     * Return if target apn is connected state or not
     */
    boolean isConnected();

    /**
     * Return data connection state of target apn
     */
    int getDataState();

    /**
     * Return Ipcan category of target apn
     * In default, return value is "IPCAN_CATEGORY_MOBILE"
     */
    int getIpcanCategory();

    /**
     * Return Ip version setting of target apn.
     */
    int getIpVersion();

    /**
     * Return slot id of target apn.
     */
    int getSlotId();

    /**
     * Return Context object that stored in target apn.
     * Reference of context object delivered to child operator apn classes.
     */
    Context getContext();

    /**
     * Send message object to target Apn.
     * Apn extends Handler class,so each apn can handle those message
     */
    boolean sendMessage(Message msg);

    /**
     * Set to use this apn. This api increase the value of EmployCount.
     */
    void employApn();

    /**
     * Set not to use this apn. This api decreate the value of EmployCount.
     */
    void dismissApn();

    /**
     * Return how many service was registered to use  use this apn.
     * When employ counter is 0 or below, this means there is no service to use this apn.
     * In this case, apn should not trigger 'requestNetwork' to use this apn.
     */
    int getApnEmployCount();

    /**
     * Returns mESMCausePermanentFailure (true/false)
     */
    boolean isESMCausePermanentFailure();

    /**
     * Return cached network of APN
     */
    Network getCachedNetwork();
}
