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

package com.android.imsstack.enabler.ssc;

import android.content.Context;

import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.util.ImsLog;

public class SscNetConnectionWifi extends SscNetConnection {
    public SscNetConnectionWifi(int slotId) {
        super(slotId);
        ImsLog.w("");
    }

    @Override
    public void init(Context context, EApnType apnType) {
        super.init(context, apnType);
    }

    @Override
    public boolean isConnected() {
        ImsLog.d("");
        return true;
    }

    @Override
    public boolean connect() {
        ImsLog.d("");
        return true;
    }

    @Override
    public void disconnect() {
        ImsLog.d("");
        return;
    }
}
