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
package com.android.imsstack.its.base;

import android.content.Context;

import com.android.imsstack.base.AppContext;

/**
 * An initializer of {@link AppContext}.
 */
public class AppContextInitializer {
    /**
     * Initializes the {@link AppContext} object and registers the proxy system services.
     *
     * @param context The application context.
     */
    public static void init(Context context) {
        AppContext.init(context);
        AppContext appContext = AppContext.getInstance();
        appContext.setContentProviderProxy(
                new ContentProviderProxyImpl());
        appContext.setBroadcastReceiverProxy(
                new BroadcastReceiverProxyImpl(appContext, appContext.getMainHandler()));
        appContext.setSystemServiceProxy(
                new SystemServiceProxyImpl(appContext));
    }
}
