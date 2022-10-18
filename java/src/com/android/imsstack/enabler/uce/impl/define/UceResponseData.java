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

package com.android.imsstack.enabler.uce.impl.define;

public class UceResponseData {
    private int mResponseCode;
    private String mReason;
    private int mReasonHeaderCause;
    private String mReasonHeaderText;
    private String mEtag;
    private long mCapability;
    private int mNeedToRetry;

    public UceResponseData(int responseCode, String reason) {
        mResponseCode = responseCode;
        mReason = reason;
        mReasonHeaderCause = 0;
        mReasonHeaderText = "";
        mEtag = "";
        mCapability = 0;
        mNeedToRetry = 0;
    }

    public void setReasonHeaderCause(int reasonHeaderCause) {
        mReasonHeaderCause = reasonHeaderCause;
    }

    public void setReasonHeaderText(String reasonHeaderText) {
        mReasonHeaderText = reasonHeaderText;
    }

    public void setEtag(String etag) {
        mEtag = etag;
    }

    public void setCapability(long capability) {
        mCapability = capability;
    }

    public void setNeedToRetry(int needToRetry) {
        mNeedToRetry = needToRetry;
    }

    public int getResponseCode() {
        return mResponseCode;
    }

    public String getReason() {
        return mReason;
    }

    public int getReasonHeaderCause() {
        return mReasonHeaderCause;
    }

    public String getReasonHeaderText() {
        return mReasonHeaderText;
    }

    public String getEtag() {
        return mEtag;
    }

    public long getCapability() {
        return mCapability;
    }

    public int getNeedToRetry() {
        return mNeedToRetry;
    }
}