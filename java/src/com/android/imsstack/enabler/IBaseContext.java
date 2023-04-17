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
package com.android.imsstack.enabler;

import android.os.Handler;
import android.os.Looper;

import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.system.ISystem;

public interface IBaseContext extends IContext {
    /**
     * Utilities for this context.
     */
    public Handler getCallHandler();
    public Looper getCallLooper();

    /**
     * Returns service state tracker to check or monitor VoLte/Vt registration.
     */
    public IServiceStateTracker getServiceStateTracker();

    /**
     * Platform interface's wrappers.
     */

    /**
     * wrapper to get the object of DcApn.
     */
    IDcApn getDcApn();

    /**
     * wrapper to get the object of DcNetWatcher.
     */
    IDcNetWatcher getDcNetWatcher();
    /**
     * Returns the {@link NativeStateInterface} instance.
     */
    NativeStateInterface getNativeStateInterface();
    public ISubscription getSubscription();
    public ISystem getSystem();

    /**
     * Returns the {@link LocationInterface} instance.
     */
    LocationInterface getLocationInterface();

    /** Returns the USAT interface. */
    UsatInterface getUsatInterface();

    public boolean isCommonPackageReady();
    public void addCommonPackageListener(ICommonPackageListener listener);
    public void removeCommonPackageListener(ICommonPackageListener listener);
}
