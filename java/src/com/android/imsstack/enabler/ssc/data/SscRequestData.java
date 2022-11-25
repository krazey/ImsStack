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

/**
 * This class stores request data to process from the framework. It handles transaction id, retry
 * count, and supplementary service configuration data that needs to update via Ut interface.
 */
public class SscRequestData {
    private final int mTransactionId;
    private int mRetryCount = 0;
    private int mPreconditionFailedCount = 0;
    private final ArrayDeque<SscData> mSscDataDeque = new ArrayDeque<>();

    /**
     * The default constructor
     *
     * @param transactionId The unique id to communicate with framework.
     */
    public SscRequestData(int transactionId) {
        this.mTransactionId = transactionId;
    }

    /**
     * Returns transaction ID.
     */
    public int getTransactionId() {
        return mTransactionId;
    }

    /**
     * Increases the number of retry.
     */
    public void increaseRetryCount() {
        mRetryCount++;
    }

    /**
     * Returns the number of retry.
     */
    public int getRetryCount() {
        return mRetryCount;
    }

    /**
     * Increases the number of precondition failure.
     */
    public void increasePreconditionFailedCount() {
        mPreconditionFailedCount++;
    }

    /**
     * Returns the number of precondition failure.
     */
    public int getPreconditionFailedCount() {
        return mPreconditionFailedCount;
    }

    /**
     * Inserts SscData at the front of mSscDataDeque to handle first.
     *
     * @param sscData See {@link SscData}.
     */
    public void offerSscDataFirst(SscData sscData) {
        mSscDataDeque.offerFirst(sscData);
    }

    /**
     * Inserts SscData at the end of mSscDataDeque.
     *
     * @param sscData See {@link SscData}.
     */
    public void offerSscData(SscData sscData) {
        mSscDataDeque.offerLast(sscData);
    }

    /**
     * Retrieves, but does not remove, the last element of mSscDataDeque, or returns null if
     * mSscDataDeque is empty.
     *
     * @return See {@link SscData}.
     */
    public SscData peakSscData() {
        return mSscDataDeque.peekFirst();
    }

    /**
     * Retrieves and removes the first element of mSscDataDeque, or returns null if mSscDataDeque is
     * empty.
     *
     * @return See {@link SscData}.
     */
    public SscData pollSscData() {
        return mSscDataDeque.pollFirst();
    }
}
