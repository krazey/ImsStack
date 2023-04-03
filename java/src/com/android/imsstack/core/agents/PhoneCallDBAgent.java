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

import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPICallInfo;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

public class PhoneCallDBAgent implements IAgent, ISystemAPICallInfo {
    // Constants--------------------------------------------------

    // Variables--------------------------------------------------
    private Context mContext;
    private final int mSlotId;

    // Public methods --------------------------------------------
    public PhoneCallDBAgent(int slotId) {
        mSlotId = slotId;
    }

    // Interface implementation methods --------------------------
    @Override
    public void init(Context context) {
        mContext = context;

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPICallInfo(this);
        }
    }

    @Override
    public void cleanup() {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPICallInfo(null);
        }
    }

    @Override
    public int isEmergencyNumber(String number) {
        String formatted = android.telephony.PhoneNumberUtils.stripSeparators(number);

        // normal number is false
        boolean bEmergencyNumber =
                AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId)).
                        isEmergencyNumber(formatted);

        ImsLog.d(mSlotId, "number = " + number + " , formatted string: format[" + formatted + "]"
            + "Is[" + bEmergencyNumber + "]");

        if (bEmergencyNumber) {
            return 1;
        } else {
            return 0;
        }
    }

    @Override
    public int getCallStateInOtherSlot() {
        // TODO: add CS call state in other slot using AOSP API
        return TelephonyManager.CALL_STATE_IDLE;
    }
}
