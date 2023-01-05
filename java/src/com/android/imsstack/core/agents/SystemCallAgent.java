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

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.IpSecSaParameter;
import com.android.imsstack.system.SystemCallInterface;
import com.android.imsstack.system.SystemInterface;

import java.io.FileDescriptor;

public class SystemCallAgent implements SystemCallInterface {
    private final int mSlotId;

    public SystemCallAgent(int slotId) {
        mSlotId = slotId;

        setSystemCallInterface(this);
    }

    public void destroy() {
        setSystemCallInterface(null);
    }

    public void setSystemCallInterface(SystemCallInterface systemCall) {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

        if (system != null) {
            system.setSystemCallInterface(systemCall);
        }
    }

    /**
     * Returns the carrier configuration.
     *
     * @return A carrier configuration object.
     */
    @Override
    public CarrierConfig getCarrierConfig() {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);
        return (config != null) ? config.getCarrierConfig() : null;
    }

    /**
     * Add an IpSec security association parameter.
     *
     * @param param The IpSec SA parameter.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    @Override
    public int addIpSecSaParameter(IpSecSaParameter param) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);
        return (ipsec != null) ? ipsec.addIpSecSaParameter(param) : RESULT_ERROR;
    }

    /**
     * Remove an IpSec security association parameter with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     */
    @Override
    public void removeIpSecSaParameter(int ipSecId) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);

        if (ipsec != null) {
            ipsec.removeIpSecSaParameter(ipSecId);
        }
    }

    /**
     * Apply the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    @Override
    public int applyIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);
        return (ipsec != null) ? ipsec.applyIpSecSa(ipSecId, spi, intFd, socketFd) : RESULT_ERROR;
    }

    /**
     * Remove the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     */
    @Override
    public void removeIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);

        if (ipsec != null) {
            ipsec.removeIpSecSa(ipSecId, spi, intFd, socketFd);
        }
    }

    /**
     * Returns the ISIM state as a string.
     *
     * @return The ISIM state string.
     */
    @Override
    public String getIsimState() {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);
        return (sim != null) ? sim.getIsimStateString() : "UNKNOWN";
    }

    /**
     * Reads the file attributes of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     */
    @Override
    public int readIsimFileAttributes(int fileId) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.readIsimFileAttributes(fileId);
            return 1;
        }

        return 0;
    }

    /**
     * Reads the value of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @param index The index of the record for the given file.
     */
    @Override
    public int readIsimRecord(int fileId, int index) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.readIsimRecord(fileId, index);
            return 1;
        }

        return 0;
    }

    /**
     * Returns the response of ISIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     */
    @Override
    public int requestIsimAuthentication(String nonce, long owner) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.requestSimAuthentication(Sim.APP_TYPE_ISIM, nonce, owner);
            return 1;
        }

        return 0;
    }

    /**
     * Returns the response of USIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     */
    @Override
    public int requestUsimAuthentication(String nonce, long owner) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.requestSimAuthentication(Sim.APP_TYPE_USIM, nonce, owner);
            return 1;
        }

        return 0;
    }

    /**
     * Returns the flag specifying whether the IMS voice call is supported on the LTE network.
     */
    @Override
    public boolean isImsVoiceCallSupported() {
        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        return (dcnw != null) ? dcnw.isVops() : false;
    }
}
