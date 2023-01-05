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
package com.android.imsstack.jni;

import android.os.Parcel;

/**
 * This is a listener interface to receive the event from the native layer.
 */
public interface JniImsListener {
    /**
     * Invoked when the event is received from the native layer.
     *
     * @param parcel Parcel object that contains the event and its parameters.
     */
    void onMessage(Parcel parcel);
}
