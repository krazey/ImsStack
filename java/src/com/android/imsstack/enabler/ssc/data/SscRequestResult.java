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

public class SscRequestResult {
    private int mSlotId = -1;
    private int mTransactionId = -1;
    private int mResultState = -1;
    private int mCode = -1;
    private int mExtraCode = -1;

    private SscServiceData mSscServiceData = null;

    public SscRequestResult(int slotId, int transactionId, int resultState, int code,
            int extraCode) {
        mSlotId = slotId;
        mTransactionId = transactionId;
        mResultState = resultState;
        mCode = code;
        mExtraCode = extraCode;
    }

    public int getSlotId() {
        return mSlotId;
    }

    public int getTransactionId() {
        return mTransactionId;
    }

    public int getResultState() {
        return mResultState;
    }

    public int getCode() {
        return mCode;
    }

    public int getExtraCode() {
        return mExtraCode;
    }

    public void setSscServiceData(SscServiceData data) {
        mSscServiceData = data;
    }

    public SscServiceData getSscServiceData() {
        return mSscServiceData;
    }

    public String toString() {
        return "TransactionId : " + mTransactionId + "\n" + "ResultState : " + mResultState + "\n"
                + "Code : " + mCode + "\n" + "ExtraCode : " + mExtraCode + "\n"
                + (getSscServiceData() != null ?  "\n" + getSscServiceData().toString() : "\n");
    }
}
