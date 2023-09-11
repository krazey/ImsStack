/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.imsservice.mmtel;

import android.telephony.imscallext.ImsCallExtManager;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

/**
 * The ImsService needs to notify E2EE module with related call information for RTP encryption.
 * This class will notify the E2EE module of call information.
 */
public final class ImsE2eeNotifier {
    private final int mSlotId;
    private final String mImsCallSessionId;
    private final byte[] mLocalSessionId;
    private final byte[] mRemoteSessionId;
    private final String mRemotePhoneNumber;

    /**
     * The Builder of the {@link ImsE2eeNotifier}.
     */
    public static class Builder {
        private final int mSlotId;
        private String mImsCallSessionId;
        private byte[] mLocalSessionId;
        private byte[] mRemoteSessionId;
        private String mRemotePhoneNumber;

        public Builder(int slotId) {
            mSlotId = slotId;
        }

        /**
         * Sets a Call session ID of the {@link ImsCallSessionImpl}.
         */
        public Builder setImsCallSessionId(String imsCallSessionId) {
            mImsCallSessionId = imsCallSessionId;
            return this;
        }

        /**
         * Sets a local Session-ID for this call.
         */
        public Builder setLocalSessionId(byte[] localSessionId) {
            mLocalSessionId = localSessionId;
            return this;
        }

        /**
         * Sets a remote Session-ID for this call
         */
        public Builder setRemoteSessionId(byte[] remoteSessionId) {
            mRemoteSessionId = remoteSessionId;
            return this;
        }

        /**
         * Sets a phone number of remote peer.
         */
        public Builder setRemotePhoneNumber(String remotePhoneNumber) {
            mRemotePhoneNumber = remotePhoneNumber;
            return this;
        }

        /**
         * Builds an instance of the {@link ImsE2eeNotifier}.
         */
        public ImsE2eeNotifier build() {
            return new ImsE2eeNotifier(this);
        }
    }

    private ImsE2eeNotifier(Builder builder) {
        mSlotId = builder.mSlotId;
        mImsCallSessionId = builder.mImsCallSessionId;
        mLocalSessionId = builder.mLocalSessionId;
        mRemoteSessionId = builder.mRemoteSessionId;
        mRemotePhoneNumber = builder.mRemotePhoneNumber;
    }

    /**
     * Notifies the E2EE module with related call information.
     */
    public void notifyE2eeCallInfo() {
        try {
            ImsLog.i("notifyE2eeCallInfo");
            ImsCallExtManager.getInstance(AppContext.getInstance()).reportCallInfo(mSlotId,
                    mImsCallSessionId, mLocalSessionId, mRemoteSessionId, mRemotePhoneNumber);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
