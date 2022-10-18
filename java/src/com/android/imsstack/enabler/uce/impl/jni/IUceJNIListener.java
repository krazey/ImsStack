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

package com.android.imsstack.enabler.uce.impl.jni;

import android.os.Parcel;

public interface IUceJNIListener {
    default void onPublishResponseMessage(Parcel parcel) {}
    default void onSubscribeResponseMessage(Parcel parcel) {}
    default void onOptionsResponseMessage(Parcel parcel) {}
    default void onServiceStatusMessage(Parcel parcel) {}
    default void onNetworkStatusMessage(Parcel parcel) {}
    default void onReceivedRemoteOptionsMessage(Parcel parcel) {}
    default void onPublishStatusMessage(Parcel parcel) {}
}
