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

import java.util.ArrayDeque;

public class SscRequestData {
    private int mTransactionId = 0;
    private int mRetryCount = 0;
    private int mPreconditionFailedCount = 0;
    private ArrayDeque<SscData> mSscDatas = new ArrayDeque<>();

    public SscRequestData(int transactionId) {
        this.mTransactionId = transactionId;
    }

    public int getTransactionId() {
        return mTransactionId;
    }

    public void increaseRetryCount() {
        mRetryCount++;
    }

    public int getRetryCount() {
        return mRetryCount;
    }

    public void increasePreconditionFailedCount() {
        mPreconditionFailedCount++;
    }

    public int getPreconditionFailedCount() {
        return mPreconditionFailedCount;
    }

    public void offerSscDataFirst(SscData SscData) {
        mSscDatas.offerFirst(SscData);
    }

    public void offerSscData(SscData SscData) {
        mSscDatas.offerLast(SscData);
    }

    public SscData peakSscData() {
        return mSscDatas.peekFirst();
    }

    public SscData pollSscData() {
        return mSscDatas.pollFirst();
    }
}
