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

public class SscServiceQueryData extends SscData {
    protected int mCondition = -1;

    public SscServiceQueryData(int slotId, ESsType ssType, int eventNum, int transactionId,
            int serviceClass) {
        super(slotId, ssType, eventNum, transactionId, serviceClass);
    }

    public int getCondition() {
        return mCondition;
    }

    public void setCondition(int condition) {
        mCondition = condition;
    }

    public String toString() {
        return ("SlotId : " + mSlotId + ", SStype : " + mSsType
                + ", EventNum : " + mEventNum + ", TransactionId : " + mTransactionId);
    }
}
