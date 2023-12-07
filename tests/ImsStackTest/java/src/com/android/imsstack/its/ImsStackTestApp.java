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
package com.android.imsstack.its;

import android.app.Application;
import android.util.Log;

import com.android.imsstack.ImsStackMain;

/**
 * A application entry point of ImsStack test process.
 */
public class ImsStackTestApp extends Application {
    private static final String TAG = ImsStackTestApp.class.getSimpleName();

    private final ImsStackMain mMain = new ImsStackMain();

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "onCreate");
        mMain.start(this);
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
        Log.i(TAG, "onTerminate");
        mMain.stop();
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        Log.i(TAG, "onTrimMemory: level=" + level);
        Runtime.getRuntime().gc();
    }
}
