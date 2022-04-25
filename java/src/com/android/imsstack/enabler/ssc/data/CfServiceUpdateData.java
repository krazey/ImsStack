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

package com.android.imsstack.enabler.ssc.data;

import com.android.imsstack.enabler.ssc.ESsType;

public class CfServiceUpdateData extends SscServiceData {
    private String mForwardToNumber = null;
    private int mNoReplyTimer = -1;

    public CfServiceUpdateData(int slotId, ESsType ssType, int eventNum, int transactionId,
            int state, int conidition, String forwardToNumber, int noReplyTimer, int serviceClass) {
        super(slotId, ssType, eventNum, transactionId, state, serviceClass);
        mCondition = conidition;
        mForwardToNumber = forwardToNumber;
        mNoReplyTimer = noReplyTimer;
    }

    public String getForwardToNumber() {
        return mForwardToNumber;
    }

    public int getReplyTimer() {
        return mNoReplyTimer;
    }

    public String toString() {
        return super.toString() + "\nCondition :" + mCondition + "\n" + "ForwardTo :"
                + mForwardToNumber + "\n" + "NoReplyTimer :" + mNoReplyTimer;
    }
}
