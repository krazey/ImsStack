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

package com.android.imsstack.enabler.acs;

import android.annotation.NonNull;
import android.text.TextUtils;

/**
 * The container of RCS application related configs.
 */
public class AcServiceClientInfo {
    private String mRcsVersion;
    private String mRcsProfile;
    private String mClientVendor;
    private String mClientVersion;
    private boolean mRcsEnabledByUser;

    /**
     * Create a AcServiceClientInfo object.
     * @param rcsVersion The parameter identifies the RCS version supported
     * by the client. Refer to GSMA RCC.07 "rcs_version" parameter.
     * @param rcsProfile Identifies a fixed set of RCS services that are
     * supported by the client.
     * @param clientVendor Identifies the vendor providing the RCS client.
     * @param clientVersion Identifies the RCS client version. Refer to GSMA
     * RCC.07 "client_version" parameter.
     * Example:client_version=RCSAndrd-1.0
     * @param isRcsEnabledByUser The current user setting for weather or not the user has
     * enabled or disabled RCS.
     */
    public AcServiceClientInfo(@NonNull String rcsVersion,
            @NonNull String rcsProfile,
            @NonNull String clientVendor, @NonNull String clientVersion,
            boolean isRcsEnabledByUser) {
        mRcsVersion = rcsVersion;
        mRcsProfile = rcsProfile;
        mClientVendor = clientVendor;
        mClientVersion = clientVersion;
        mRcsEnabledByUser = isRcsEnabledByUser;
    }

    /**
     * Create instance based source object
     * @param acServiceClientInfo source AcServiceClientInfo
     */
    public AcServiceClientInfo(@NonNull AcServiceClientInfo acServiceClientInfo) {
        mRcsVersion = acServiceClientInfo.getRcsVersion();
        mRcsProfile = acServiceClientInfo.getRcsProfile();
        mClientVendor = acServiceClientInfo.getClientVendor();
        mClientVersion = acServiceClientInfo.getClientVersion();
        mRcsEnabledByUser = acServiceClientInfo.isRcsEnabledByUser();
    }

    /**
     * Returns RCS version supported.
     */
    public @NonNull String getRcsVersion() {
        return mRcsVersion;
    }

    /**
     * Returns RCS profile supported.
     */
    public @NonNull String getRcsProfile() {
        return mRcsProfile;
    }

    /**
     * Returns the name of the vendor providing the RCS client.
     */
    public @NonNull String getClientVendor() {
        return mClientVendor;
    }

    /**
     * Returns the RCS client version.
     */
    public @NonNull String getClientVersion() {
        return mClientVersion;
    }

    /**
     * The current user setting provided by the RCS messaging application that determines
     * weather or not the user has enabled RCS.
     * @return true if RCS is enabled by the user, false if RCS is disabled by the user.
     */
    public boolean isRcsEnabledByUser() {
        return mRcsEnabledByUser;
    }

    /**
     * To check all attributes are initialized.
     * @return true if all attributes have valid value, false if one of them is not initialized.
     */
    public boolean isValid() {
        return (!TextUtils.isEmpty(mRcsVersion) && !TextUtils.isEmpty(mRcsProfile)
                && !TextUtils.isEmpty(mClientVendor) && !TextUtils.isEmpty(mClientVersion));
    }

    /**
     * Return the String object includes all internal data
     */
    public @NonNull String toString() {
        return new String("ver : " + mRcsVersion
                + " profile : " + mRcsProfile
                + " client vendor : " + mClientVendor
                + " client version : " + mClientVersion
                + " enable by user : " + mRcsEnabledByUser);
    }
}
