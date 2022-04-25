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

public class CbServiceUpdateData extends SscServiceData  {
    private String[] mBarrList = null;
    private String mPassword = null;

    public CbServiceUpdateData(int slotId, ESsType ssType, int evnetNum, int transactionId,
            int state, int condition, String[] barrList, int serviceClass, String password) {
        super(slotId, ssType, evnetNum, transactionId, state, serviceClass);
        mCondition = condition;
        mBarrList = barrList;
        mPassword = password;
    }

    public String[] getBarrList() {
        return mBarrList;
    }

    public String getPassword() {
        return mPassword;
    }

    public String toString() {
        return super.toString() + ", mCondition : " + mCondition + ", Barr List : " + mBarrList;
    }
}
