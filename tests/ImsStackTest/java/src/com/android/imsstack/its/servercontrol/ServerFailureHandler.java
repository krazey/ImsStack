/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.servercontrol;

/**
 * Abstract class to handle server failures. This class is designed to process failure messages
 * received via a socket connection and provide a mechanism for subclasses to implement their own
 * failure handling logic.
 *
 * <p>Subclasses must implement the {@link #handleServerFailure()} method to define how the failure
 * should be handled after the failure detail is set.</p>
 */
public abstract class ServerFailureHandler {
    /**
     * The detailed message describing the failure received from the server.
     */
    protected String mFailureDetail;

    /**
     * Sets the failure detail and invokes the failure handling mechanism.
     *
     * <p>This method should be called when a failure is detected, and it will store the provided
     * failure detail and trigger the {@link #handleServerFailure()} method, which must be
     * implemented by subclasses.</p>
     *
     * @param failureDetail The detailed failure message to set.
     */
    public final void handleFailure(String failureDetail) {
        mFailureDetail = failureDetail;
        handleServerFailure();
    }

    /**
     * Abstract method to be implemented by subclasses to handle the failure.
     *
     * <p>This method is called after the failure detail has been set using
     * {@link #handleFailure(String)}. Subclasses should provide the specific logic for
     * handling the failure.</p>
     */
    protected abstract void handleServerFailure();
}
