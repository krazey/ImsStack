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

package com.android.imsstack.imsservice.mmtel.ut;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.enabler.ssc.SscServiceImpl;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.internal.annotations.VisibleForTesting;

public final class UtFactory {
    private static final UtFactory sUtFactory = new UtFactory();
    private final IUtInterface[] mUtInterfaces =
            new IUtInterface[DeviceConfig.getSupportedSimCount()];

    private UtFactory() {
        java.util.Arrays.fill(mUtInterfaces, null);
    }

    public static UtFactory getInstance() {
        return sUtFactory;
    }

    /**
     * Object of IUtInterface is fetched.
     */
    public IUtInterface getUtInterface(int slotId) {
        if (slotId < 0 || slotId >= mUtInterfaces.length) {
            return null;
        }

        if (mUtInterfaces[slotId] == null) {
            setUtInterfaceForSlot(slotId, new SscServiceImpl(slotId));
        }

        return mUtInterfaces[slotId];
    }

    public void releaseUtInterface(int slotId) {
        if (slotId < 0 || slotId >= mUtInterfaces.length) {
            return;
        }

        setUtInterfaceForSlot(slotId, null);
    }

    @VisibleForTesting
    public void setUtInterfaceForSlot(int slotId, IUtInterface sscServiceImpl) {
        if (mUtInterfaces[slotId] != null) {
            mUtInterfaces[slotId].close();
        }

        mUtInterfaces[slotId] = sscServiceImpl;
    }
}
