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

package com.android.imsstack.core.agents;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SimUtils;

/**
 * This class provides all the SIM states and its related operations.
 */
public class SimAgent implements SimInterface {
    private final int mSlotId;
    private final UsatAgent mUsatAgent;

    public SimAgent(int slotId) {
        mSlotId = slotId;
        mUsatAgent = new UsatAgent(this);
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public int getSlotId() {
        return mSlotId;
    }

    @Override
    public UsatInterface getUsatInterface() {
        return mUsatAgent;
    }

    @Override
    public byte[] getUsimServiceTable() {
        // TODO: EF_UST will be managed by this component.
        int subId = MSimUtils.getSubId(getSlotId());
        TelephonyManager tm = AppContext.getTelephonyManager(subId);
        String serviceTable = null; /*(tm != null)
                ? tm.getSimServiceTable(TelephonyManager.APPTYPE_USIM)
                : null;*/
        return SimUtils.hexStringToBytes(serviceTable);
    }
}
