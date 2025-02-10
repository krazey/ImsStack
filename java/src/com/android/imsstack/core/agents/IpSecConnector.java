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

import android.net.IpSecAlgorithm;
import android.net.IpSecManager;
import android.net.IpSecManager.ResourceUnavailableException;
import android.net.IpSecManager.SecurityParameterIndex;
import android.net.IpSecManager.SpiUnavailableException;
import android.net.IpSecTransform;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.IpSecManagerProxy;
import com.android.imsstack.system.IpSecSaParameter;
import com.android.imsstack.system.IpSecSaPolicy;
import com.android.imsstack.util.ImsLog;

import java.io.FileDescriptor;
import java.net.InetAddress;
import java.util.List;

public class IpSecConnector {
    private static final int MAX_TRANSFORM = 4;
    // ImsStack does not need to remove the binding between the socket and IpSec association,
    // because its socket will not be reused anymore after closing it.
    private static final boolean SUPPORT_REMOVE_IPSEC_TRANSFORM_PER_SOCKET = false;

    private final IpSecSaParameter mParam;
    private IpSecAlgorithm mEncryptionAlgorithm = null;
    private IpSecAlgorithm mIntegrityAlgorithm = null;
    // <SPI, Policy>
    private final SparseArray<Transform> mTransforms;
    private boolean mRemoved = false;

    public IpSecConnector(IpSecSaParameter param) {
        mParam = param;
        mTransforms = new SparseArray<>(MAX_TRANSFORM);
    }

    /**
     * Applies the security association with the given SPI and socket.
     *
     * @param spi The security parameter index.
     * @param intFd The socket file descriptor as integer.
     * @param socketFd The socket file descriptor.
     * @return {@code true} if it's successfully applied, {@code false} otherwise.
     */
    public boolean applySa(int spi, int intFd, FileDescriptor socketFd) {
        logd(this, "applySa - spi=" + spi + ", intFd=" + intFd);

        Transform transform = mTransforms.get(spi);

        if (transform == null) {
            loge(this, "Transform is not found");
            return false;
        }

        IpSecSaPolicy policy = mParam.getPolicy(spi);

        if (policy == null) {
            loge(this, "Policy is not found");
            return false;
        }

        final IpSecManagerProxy ismp = getIpSecManagerProxy();

        if (policy.getMode() == IpSecSaPolicy.MODE_TRANSPORT) {
            int direction = (policy.getDirection() == IpSecSaPolicy.DIRECTION_IN)
                    ? IpSecManager.DIRECTION_IN : IpSecManager.DIRECTION_OUT;

            try {
                ismp.applyTransportModeTransform(
                        socketFd,
                        direction,
                        transform.getIpSecTransform());
            } catch (IllegalArgumentException
                    | IllegalStateException
                    | UnsupportedOperationException e) {
                loge(this, "applyTransportMode - " + e.toString());
                return false;
            } catch (Throwable t) {
                loge(this, "applyTransportMode - " + t.toString());
                return false;
            }
        } else if (policy.getMode() == IpSecSaPolicy.MODE_TUNNEL) {
            // At the moment, IMS doesn't support this.
        }

        transform.addSocket(intFd, socketFd);

        return true;
    }

    /**
     * Removes the security association with the given SPI and socket.
     *
     * @param spi The security parameter index.
     * @param intFd The socket file descriptor as integer.
     * @param socketFd The socket file descriptor.
     */
    public void removeSa(int spi, int intFd, FileDescriptor socketFd) {
        logd(this, "removeSa - spi=" + spi + ", intFd=" + intFd);

        Transform transform = mTransforms.get(spi);

        if (transform == null) {
            loge(this, "Transform is not found");
            return;
        }

        if (!transform.removeSocket(intFd)) {
            // It's already removed.
            logi(this, "SA is already removed - spi=" + spi + ", intFd=" + intFd);
            return;
        }

        IpSecSaPolicy policy = mParam.getPolicy(spi);

        if (policy == null) {
            loge(this, "Policy is not found");
            return;
        }

        if (SUPPORT_REMOVE_IPSEC_TRANSFORM_PER_SOCKET) {
            final IpSecManagerProxy ismp = getIpSecManagerProxy();

            if (policy.getMode() == IpSecSaPolicy.MODE_TRANSPORT) {
                try {
                    ismp.removeTransportModeTransforms(socketFd);
                } catch (IllegalArgumentException
                        | IllegalStateException
                        | UnsupportedOperationException e) {
                    loge(this, "removeTransportMode - " + e.toString());
                    return;
                } catch (Throwable t) {
                    loge(this, "removeTransportMode - " + t.toString());
                    return;
                }
            } else if (policy.getMode() == IpSecSaPolicy.MODE_TUNNEL) {
                // Do nothing...
            }
        }
    }

    /**
     * Closes all the resources for the {@link IpSecConnector}.
     */
    public void close() {
        for (int i = 0; i < mTransforms.size(); ++i) {
            Transform transform = mTransforms.valueAt(i);
            SparseArray<FileDescriptor> sockets = transform.getSockets();

            if (sockets.size() > 0) {
                logi(this, "Sockets exist on close.");

                for (int j = 0; j < sockets.size(); ++j) {
                    int intFd = sockets.keyAt(j);
                    FileDescriptor socketFd = sockets.valueAt(j);

                    removeSa(transform.getSecurityParameterIndex().getSpi(), intFd, socketFd);
                }
            }

            transform.close();
        }

        mTransforms.clear();

        mEncryptionAlgorithm = null;
        mIntegrityAlgorithm = null;
    }

    /**
     * Returns the {@link IpSecSaParameter} of this connector.
     */
    public IpSecSaParameter getSaParameter() {
        return mParam;
    }

    /**
     * Initializes this connector.
     *
     * @return {@code true} if it's successfully initialized, {@code false} otherwise.
     */
    public boolean init() {
        if (mParam.getEncryptionAlgorithm() == IpSecSaParameter.ENCRYPTION_ALGORITHM_NULL) {
            // Do not create IpSecAlgorithm.
            mEncryptionAlgorithm = null;
        } else {
            mEncryptionAlgorithm = createEncryptionAlgorithm(mParam);

            if (mEncryptionAlgorithm == null) {
                loge(this, "EncryptionAlgorithm is null");
                return false;
            }
        }

        mIntegrityAlgorithm = createIntegrityAlgorithm(mParam);

        if (mIntegrityAlgorithm == null) {
            loge(this, "IntegrityAlgorithm is null");
            return false;
        }

        List<IpSecSaPolicy> saPolicys = mParam.getPolicys();

        for (int i = 0; i < saPolicys.size(); i++) {
            IpSecSaPolicy saPolicy = saPolicys.get(i);
            int spi = saPolicy.getSpi();

            if (mTransforms.get(spi) != null) {
                // Ignores the duplicate SA policy (TCP/UDP)
                logd(this, "Duplicate SA policy for a different transport - ignored.");
                continue;
            }

            Transform transform = new Transform();

            if (!transform.create(saPolicy, mIntegrityAlgorithm, mEncryptionAlgorithm))
            {
                close();
                return false;
            }

            mTransforms.put(spi, transform);
        }

        return true;
    }

    /**
     * Checks whether all the sockets for this connector are detached.
     *
     * @return {@code true} if all the sockets are detached, {@code false} otherwise.
     */
    public boolean isAllSocketsDetached() {
        for (int i = 0; i < mTransforms.size(); ++i) {
            Transform transform = mTransforms.valueAt(i);
            SparseArray<FileDescriptor> sockets = transform.getSockets();

            if (sockets.size() > 0) {
                return false;
            }
        }

        return true;
    }

    public boolean isRemoved() {
        return mRemoved;
    }

    /** Marks this connector as removed. */
    public void markAsRemoved() {
        mRemoved = true;
    }

    private static IpSecManagerProxy getIpSecManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(IpSecManagerProxy.class);
    }

    private static IpSecAlgorithm createEncryptionAlgorithm(IpSecSaParameter param) {
        String algorithmName = null;
        int algorithm = param.getEncryptionAlgorithm();
        byte[] key = param.getCk();

        if (algorithm == IpSecSaParameter.ENCRYPTION_ALGORITHM_AES_CBC) {
            algorithmName = IpSecAlgorithm.CRYPT_AES_CBC;
        } else if (algorithm == IpSecSaParameter.ENCRYPTION_ALGORITHM_NULL) {
            algorithmName = null; // IpSecAlgorithm.CRYPT_NULL;
        } else {
            // des-ede3-cbc
        }

        if (algorithmName == null) {
            return null;
        }

        return new IpSecAlgorithm(algorithmName, key, 0);
    }

    private static IpSecAlgorithm createIntegrityAlgorithm(IpSecSaParameter param) {
        String algorithmName = null;
        int algorithm = param.getIntegrityAlgorithm();
        byte[] key = param.getIk();
        int truncatedBits = 96;

        if (algorithm == IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_MD5_96) {
            algorithmName = IpSecAlgorithm.AUTH_HMAC_MD5;
        } else if (algorithm == IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_SHA_1_96) {
            algorithmName = IpSecAlgorithm.AUTH_HMAC_SHA1;
        }

        if (algorithmName == null) {
            return null;
        }

        return new IpSecAlgorithm(algorithmName, key, truncatedBits);
    }

    private static void logd(Object o, String s) {
        ImsLog.d(o, "IpSec: " + s);
    }

    private static void loge(Object o, String s) {
        ImsLog.e(o, "IpSec: " + s);
    }

    private static void logi(Object o, String s) {
        ImsLog.i(o, "IpSec: " + s);
    }

    /**
     * A wrapper class to manage the {@link IpSecTransform} and its related sockets.
     */
    private static class Transform {
        private static final int MAX_SOCKET = 2;
        private SecurityParameterIndex mSpi;
        private IpSecTransform mTransform;
        private final SparseArray<FileDescriptor> mSockets;

        public Transform() {
            mSockets = new SparseArray<>(MAX_SOCKET);
        }

        public IpSecTransform getIpSecTransform() {
            return mTransform;
        }

        public SecurityParameterIndex getSecurityParameterIndex() {
            return mSpi;
        }

        public void close() {
            logd(this, "Sockets=" + mSockets.size());

            try {
                if (mTransform != null) {
                    mTransform.close();
                    mTransform = null;
                }

                if (mSpi != null) {
                    mSpi.close();
                    mSpi = null;
                }
            } catch (Throwable t) {
                loge(this, "close - " + t.toString());
            }
        }

        public boolean create(IpSecSaPolicy policy,
                IpSecAlgorithm integrity, IpSecAlgorithm encryption) {
            final IpSecManagerProxy ismp = getIpSecManagerProxy();

            if (mSpi == null) {
                try {
                    InetAddress remoteIp = InetAddress.getByName(policy.getRemoteIp());
                    mSpi = ismp.allocateSecurityParameterIndex(remoteIp, policy.getSpi());
                } catch (SpiUnavailableException | ResourceUnavailableException e) {
                    loge(this, "Allocating SPI failed - " + e.toString());
                    return false;
                } catch (Throwable t) {
                    loge(this, "Error - " + t.toString());
                    return false;
                }
            }

            if (mTransform == null) {
                try {
                    InetAddress localIp = InetAddress.getByName(policy.getLocalIp());
                    IpSecTransform.Builder builder =
                            new IpSecTransform.Builder(AppContext.getInstance());
                    builder.setAuthentication(integrity);

                    if (encryption != null) {
                        builder.setEncryption(encryption);
                    }

                    if (policy.getMode() == IpSecSaPolicy.MODE_TRANSPORT) {
                        mTransform = builder.buildTransportModeTransform(localIp, mSpi);
                    } else if (policy.getMode() == IpSecSaPolicy.MODE_TUNNEL) {
                        logd(this, "Invalid argument - unsupported mode (tunnel)");
                        close();
                        return false;
                    }
                } catch (Throwable t) {
                    loge(this, "Error - " + t.toString());
                    close();
                    return false;
                }
            }

            return true;
        }

        public SparseArray<FileDescriptor> getSockets() {
            return mSockets;
        }

        public void addSocket(int intFd, FileDescriptor socketFd) {
            mSockets.put(intFd, socketFd);
        }

        public boolean removeSocket(int intFd) {
            if (mSockets.contains(intFd)) {
                mSockets.remove(intFd);
                return true;
            }

            return false;
        }
    }
}
