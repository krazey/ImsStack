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

package com.android.imsstack.imsservice.mmtel.ut.base;

import android.content.Context;

/**
 * Provides the Ut interface interworking to get/set the supplementary service configuration.
 * {link @IImsUt}
 */
public interface IUtInterface {
    /**
     * Provide current Ut service availability
     */
    boolean isUtAvailable();

    /**
     * Changes Ut capability
     */
    void changeCapability(boolean enable);

    /**
     * Initialize Ut service
     */
    void start(Context context);

    /**
     * Set listener for Ut operation result
     */
    void setListener(IUtListener listener);

    /**
     * Set listener to listen to Ut service state change
     */
    void setServiceStateListener(IUtServiceStateListener listener);

    /**
     * Inform service state change
     */
    void onServiceStateChanged();

    /**
     * Implementation of IImsUt.
     */
    void close();

    /**
     * Implementation of IImsUt.
     */
    void queryCallBarring(int tId, int condition);

    /**
     * Implementation of IImsUt.
     */
    void queryCallBarringForServiceClass(int tId, int condition, int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void queryCallForward(int tId, int condition, String number);

    /**
     * Implementation of IImsUt.
     */
    void queryCallWaiting(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCLIR(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCLIP(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCOLR(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCOLP(int tId);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarring(int tId, int condition, int action, String[] barringList);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarringForServiceClass(int tId, int cbType, int action, String[] barringList,
            int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarringWithPassword(int tId, int condition, int action, String[] barringList,
            int serviceClass, String password);

    /**
     * Implementation of IImsUt.
     */
    void updateCallForward(int tId, int action, int condition, String number, int serviceClass,
            int timeSeconds);

    /**
     * Implementation of IImsUt.
     */
    void updateCallWaiting(int tId, boolean enable, int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void updateCLIR(int tId, int clirMode);

    /**
     * Implementation of IImsUt.
     */
    void updateCLIP(int tId, boolean enable);

    /**
     * Implementation of IImsUt.
     */
    void updateCOLR(int tId, int presentation);

    /**
     * Implementation of IImsUt.
     */
    void updateCOLP(int tId, boolean enable);
}
