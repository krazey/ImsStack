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

import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.imsservice.mmtel.ut.base.UtListener;

public class UtInterfaceBase implements UtInterface {
    protected UtListener mUtListener = null;
    private IUtServiceStateListener mUtServiceStateListener = null;

    @Override
    public boolean isUtAvailable() {
        return false;
    }

    @Override
    public void start(Context context) {
        // no-op
    }

    @Override
    public void setListener(UtListener listener) {
        mUtListener = listener;
    }

    @Override
    public void setServiceStateListener(IUtServiceStateListener listener) {
        mUtServiceStateListener = listener;
    }

    @Override
    public void onServiceStateChanged() {
        if (mUtServiceStateListener != null) {
            mUtServiceStateListener.onServiceStateChanged();
        }
    }

    @Override
    public void close() {
        // no-op
    }

    @Override
    public void queryCallBarring(int tId, int condition) {
        // no-op
    }

    @Override
    public void queryCallBarringForServiceClass(int tId, int condition, int serviceClass) {
        // no-op
    }

    @Override
    public void queryCallForward(int tId, int condition, String number) {
        // no-op
    }

    @Override
    public void queryCallWaiting(int tId) {
        // no-op
    }

    @Override
    public void queryCLIR(int tId) {
        // no-op
    }

    @Override
    public void queryCLIP(int tId) {
        // no-op
    }

    @Override
    public void queryCOLR(int tId) {
        // no-op
    }

    @Override
    public void queryCOLP(int tId) {
        // no-op
    }

    @Override
    public void transact(int tId, Bundle ssInfo) {
        // no-op
    }

    @Override
    public void updateCallBarring(int tId, int condition, int action, String[] barringList) {
        // no-op
    }

    @Override
    public void updateCallBarringForServiceClass(int tId, int cbType, int action,
            String[] barringList, int serviceClass) {
        // no-op
    }

    @Override
    public void updateCallBarringWithPassword(int tId, int condition, int action,
            String[] barringList, int serviceClass, String password) {
        // no-op
    }

    @Override
    public void updateCallForward(int tId, int action, int condition, String number,
            int serviceClass, int timeSeconds) {
        // no-op
    }

    @Override
    public void updateCallWaiting(int tId, boolean enable, int serviceClass) {
        // no-op
    }

    @Override
    public void updateCLIR(int tId, int clirMode) {
        // no-op
    }

    @Override
    public void updateCLIP(int tId, boolean enable) {
        // no-op
    }

    @Override
    public void updateCOLR(int tId, int presentation) {
        // no-op
    }

    @Override
    public void updateCOLP(int tId, boolean enable) {
        // no-op
    }
}
