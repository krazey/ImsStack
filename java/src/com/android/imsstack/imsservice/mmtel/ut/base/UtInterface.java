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
import android.os.Bundle;

public interface UtInterface {
    /**
     * Definitions of operational methods.
     */
    public boolean isUtAvailable();
    public void start(Context context);
    public void setListener(UtListener listener);
    public void setServiceStateListener(IUtServiceStateListener listener);

    /**
     * Callback to inform service state changed
     */
    public void onServiceStateChanged();

    /**
     * Implementation of IImsUt.
     */
    public void close();

    public int queryCallBarring(int condition);
    public int queryCallBarringForServiceClass(int condition, int serviceClass);
    public int queryCallForward(int condition, String number);
    public int queryCallWaiting();
    public int queryCLIR();
    public int queryCLIP();
    public int queryCOLR();
    public int queryCOLP();

    public int transact(Bundle ssInfo);

    public int updateCallBarring(int condition, int action, String[] barringList);
    public int updateCallBarringForServiceClass(int cbType, int action, String[] barringList,
            int serviceClass);
    public int updateCallBarringWithPassword(int condition, int action, String[] barringList,
            int serviceClass, String password);
    public int updateCallForward(int action, int condition, String number, int serviceClass,
            int timeSeconds);
    public int updateCallWaiting(boolean enable, int serviceClass);
    public int updateCLIR(int clirMode);
    public int updateCLIP(boolean enable);
    public int updateCOLR(int presentation);
    public int updateCOLP(boolean enable);
}
