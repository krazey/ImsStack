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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.util.SparseArray;

import com.android.imsstack.system.IpSecSaParameter;
import com.android.imsstack.system.SystemCallInterface;
import com.android.imsstack.util.ImsLog;

import java.io.FileDescriptor;

public class IpSecAgent implements IpSecInterface {
    /** In general, IMS can have up to 3 security associations at the same time. */
    private static final int MAX_CONNECTOR = 3;

    private final int mSlotId;
    private final SparseArray<IpSecConnector> mConnectors;

    public IpSecAgent(int slotId) {
        mSlotId = slotId;
        mConnectors = new SparseArray<>(MAX_CONNECTOR);
    }

    @Override
    public void init(Context context) {
        // no-op
    }

    @Override
    public void cleanup() {
        // no-op
    }

    @Override
    public int addIpSecSaParameter(IpSecSaParameter param) {
        ImsLog.d(this, mSlotId, "IpSec: add=" + param.toString());

        IpSecConnector connector = mConnectors.get(param.getId());

        if (connector != null) {
            // The connector already exists.
            return SystemCallInterface.RESULT_OK;
        }

        connector = new IpSecConnector(param);

        if (!connector.init()) {
            ImsLog.e(this, mSlotId, "IpSec: Creating IpSecConnector failed.");
            return SystemCallInterface.RESULT_ERROR;
        }

        mConnectors.put(param.getId(), connector);

        return SystemCallInterface.RESULT_OK;
    }

    @Override
    public void removeIpSecSaParameter(int ipSecId) {
        IpSecConnector connector = mConnectors.get(ipSecId);

        ImsLog.d(this, mSlotId, "IpSec: remove="
                + (connector != null ? connector.getSaParameter().toString() : "not-found"));

        if (connector != null) {
            connector.markAsRemoved();
            connector.close();
            mConnectors.remove(ipSecId);
            return;
        }
    }

    @Override
    public int applyIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecConnector connector = mConnectors.get(ipSecId);

        if (connector != null) {
            if (!connector.applySa(spi, intFd, socketFd)) {
                return SystemCallInterface.RESULT_ERROR;
            }

            return SystemCallInterface.RESULT_OK;
        }

        return SystemCallInterface.RESULT_ERROR;
    }

    @Override
    public void removeIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecConnector connector = mConnectors.get(ipSecId);

        if (connector != null) {
            connector.removeSa(spi, intFd, socketFd);

            if (connector.isRemoved()) {
                if (connector.isAllSocketsDetached()) {
                    ImsLog.d(this, mSlotId, "IpSec: removeIpSecSa="
                            + connector.getSaParameter().toString());
                    mConnectors.remove(ipSecId);
                }
            }
        }
    }
}
