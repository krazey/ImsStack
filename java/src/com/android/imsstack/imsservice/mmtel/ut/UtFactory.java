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

import com.android.imsstack.enabler.ssc.SscServiceImpl;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

public final class UtFactory {
    @VisibleForTesting
    protected static UtFactory sUtFactory = new UtFactory();
    private final UtInterface[] mUtInterface = new UtInterface[MSimUtils.getMaxSimSlot()];

    @VisibleForTesting
    protected UtFactory() {
        for (int i = 0; i < mUtInterface.length; i++) {
            mUtInterface[i] = null;
        }
    }

    public static UtFactory getInstance() {
        return sUtFactory;
    }

    public UtInterface getUtInterface(int slotId) {
        if (slotId < 0 || slotId >= mUtInterface.length) {
            return null;
        }

        if (mUtInterface[slotId] == null) {
            setUtInterfaceForSlot(slotId, new SscServiceImpl(slotId));
        }

        return mUtInterface[slotId];
    }

    public void releaseUtInterface(int slotId) {
        if (slotId < 0 || slotId >= mUtInterface.length) {
            return;
        }

        if (mUtInterface[slotId] == null) {
            return;
        }

        setUtInterfaceForSlot(slotId, null);
    }

    @VisibleForTesting
    protected void setUtInterfaceForSlot(int slotId, UtInterface sscServiceImpl) {
        if (mUtInterface[slotId] != null) {
            mUtInterface[slotId].close();
        }

        mUtInterface[slotId] = sscServiceImpl;
    }
}
