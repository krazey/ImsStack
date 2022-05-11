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

/**
 * Callback for AcService status changes.
 */
public interface IAcServiceImplCallback {
    /**
     * If override method, then the notification will be transferred when the device receives
     * the provisioning data from service provider server
     * @param data has provisioning.xml
     * @param isDeProvision indicates Provisioning or De-Provisioning
     */
    void onReceivedProvisioning(byte[] data, boolean isDeProvision);

    /**
     * If override method, then the notification will be transferred when the device receives
     * the pre-provisioning data (self-provisioning) from service provider server
     * @param data has provisioning.xml
     */
    void onReceivedPreProvisioning(byte[] data);

    /**
     * If override method, then the notification will be transferred when the device receives
     * the error response regarding request of provisioning
     * @param errorCode HTTP response code
     * @param errorString Text of response code
     */
    void onReceivedError(int errorCode, String errorString);
}
