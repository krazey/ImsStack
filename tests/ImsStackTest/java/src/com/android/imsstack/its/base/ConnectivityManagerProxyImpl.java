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

import android.annotation.CallbackExecutor;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.QosCallback;
import android.net.QosCallbackException;
import android.net.QosSession;
import android.net.QosSocketInfo;
import android.os.Handler;
import android.telephony.data.EpsBearerQosSessionAttributes;
import android.telephony.data.NrQosSessionAttributes;
import android.util.ArraySet;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executor;

/**
 * An implementation class to access the {@link ConnectivityManager}.
 */
public class ConnectivityManagerProxyImpl implements ConnectivityManagerProxy {
    private static final long DEFAULT_EVENT_DELAY_TIME_MILLIS = 20; // 20 ms
    private static final int APN_DEFAULT = 0;
    private static final int APN_IMS = 1;
    private static final int APN_EMERGENCY = 2;
    private static final int APN_XCAP = 3;
    private static final int APN_MAX = 4;
    private final ArraySet<NetworkCallbackRecord> mNetworkCallbackRecords = new ArraySet<>();
    private final ArraySet<QosCallbackRecord> mQosCallbackRecords = new ArraySet<>();
    private final SparseArray<LinkProperties> mLinkProperties = new SparseArray<>();
    private int mQosSessionBearerType = 0;
    private final NetworkRecord[] mNetworkRecords = new NetworkRecord[APN_MAX];

    ConnectivityManagerProxyImpl() {
        for (int i = 0; i < mNetworkRecords.length; ++i) {
            mNetworkRecords[i] = null;
        }
    }

    @Override
    public void registerQosCallback(@NonNull final QosSocketInfo socketInfo,
            @CallbackExecutor @NonNull final Executor executor,
            @NonNull final QosCallback callback) {
        QosCallbackRecord qcr = new QosCallbackRecord(socketInfo, callback, executor);
        mQosCallbackRecords.add(qcr);

        if (isQosSessionBearerTypeValid()) {
            qcr.dispatchSessionAvailable(mQosSessionBearerType);
        }
    }

    @Override
    public void unregisterQosCallback(@NonNull final QosCallback callback) {
        final ArraySet<QosCallbackRecord> recordsToRemove = new ArraySet<>();
        mQosCallbackRecords.forEach((r) -> {
            if (r.hasCallback(callback)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mQosCallbackRecords::remove);
    }

    @Override
    public void registerNetworkCallback(@NonNull NetworkRequest request,
            @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
        if (isNetworkRequestForWifi(request)) {
            // The network callback for Wi-Fi connection will not be registered at the moment.
            return;
        }
        NetworkCallbackRecord ncr = new NetworkCallbackRecord(request, networkCallback, handler);
        mNetworkCallbackRecords.add(ncr);

        NetworkRecord nr = getNetworkRecord(ncr);
        if (nr != null) {
            ncr.dispatchNetworkAvailable(nr);
        }
    }

    @Override
    public void unregisterNetworkCallback(@NonNull NetworkCallback networkCallback) {
        final ArraySet<NetworkCallbackRecord> recordsToRemove = new ArraySet<>();
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.hasCallback(networkCallback)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mNetworkCallbackRecords::remove);
    }

    @Override
    public @Nullable LinkProperties getLinkProperties(@Nullable Network network) {
        NetworkRecord nr = mNetworkRecords[APN_IMS];
        if (nr == null) {
            return null;
        }
        return nr.isSameNetwork(network) ? nr.getLinkProperties() : null;
    }

    @Override
    public void requestNetwork(@NonNull NetworkRequest request,
            @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
        NetworkCallbackRecord ncr = new NetworkCallbackRecord(request, networkCallback, handler);
        mNetworkCallbackRecords.add(ncr);

        NetworkRecord nr = getNetworkRecord(ncr);
        if (nr != null) {
            nr.setCapabilities(request);
            ncr.dispatchNetworkAvailable(nr);
        }
    }

    @Override
    public void registerSystemDefaultNetworkCallback(
            @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
        NetworkCallbackRecord ncr = new NetworkCallbackRecord(null, networkCallback, handler);
        mNetworkCallbackRecords.add(ncr);

        NetworkRecord nr = getNetworkRecord(ncr);
        if (nr != null) {
            nr.setCapabilities(null);
            ncr.dispatchNetworkAvailable(nr);
        }
    }

    /**
     * Sets the bearer type of {@link QosSession}.
     *
     * @param bearerType The bearer type. Possible values are:
     *                   {@link QosSession#TYPE_EPS_BEARER},
     *                   {@link QosSession#TYPE_NR_BEARER}
     */
    public void setQosSessionBearerType(int bearerType) {
        mQosSessionBearerType = bearerType;
    }

    /**
     * Sets the network information with the specified {@link Network} and {@link LinkProperties}.
     * If the {@link Network} object is null, all the network records will be set to null.
     *
     * @param network The {@link Network} object identifying the network.
     * @param properties The {@link LinkProperties} for the specified network.
     */
    public void setNetwork(@Nullable Network network, @Nullable LinkProperties properties) {
        if (network == null) {
            for (int i = 0; i < mNetworkRecords.length; ++i) {
                mNetworkRecords[i] = null;
            }
        } else {
            for (int i = 0; i < mNetworkRecords.length; ++i) {
                if (mNetworkRecords[i] == null) {
                    mNetworkRecords[i] = new NetworkRecord(network, properties);
                }
            }
        }
    }

    /**
     * Sets the network information with the specified {@link Network} and {@link LinkProperties}.
     * If the {@link Network} object is null, all the network records will be set to null.
     *
     * @param network The {@link Network} object identifying the network.
     * @param properties The {@link LinkProperties} for the specified network.
     * @param capability The network capability. Possible values are:
     *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public void setNetwork(@Nullable Network network, @Nullable LinkProperties properties,
            int capability) {
        int apnType = APN_MAX;

        if (capability == NetworkCapabilities.NET_CAPABILITY_IMS) {
            apnType = APN_IMS;
        } else if (capability == NetworkCapabilities.NET_CAPABILITY_EIMS) {
            apnType = APN_EMERGENCY;
        } else if (capability == NetworkCapabilities.NET_CAPABILITY_XCAP) {
            apnType = APN_XCAP;
        } else if (capability == NetworkCapabilities.NET_CAPABILITY_INTERNET) {
            apnType = APN_DEFAULT;
        }

        if (apnType == APN_MAX) {
            return;
        }

        if (network == null) {
            mNetworkRecords[apnType] = null;
        } else if (mNetworkRecords[apnType] == null) {
            mNetworkRecords[apnType] = new NetworkRecord(network, properties);
        }
    }

    /**
     * Replaces the PCSCF servers in this {@code LinkProperties} with the given {@link Collection}
     * of {@link InetAddress} objects.
     *
     * @param pcscfServers The {@link Collection} of PCSCF servers to set.
     */
    public void setPcscfServers(@NonNull Collection<InetAddress> pcscfServers) {
        if (mNetworkRecords[APN_IMS] != null) {
            mNetworkRecords[APN_IMS].setPcscfServers(pcscfServers);
        }
        if (mNetworkRecords[APN_EMERGENCY] != null) {
            mNetworkRecords[APN_EMERGENCY].setPcscfServers(pcscfServers);
        }
    }

    /**
     * Notifies the application that the network is connected and has declared
     * a new network ready for use.
     *
     * @param capability The network capability. Possible values are:
     *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public void notifyNetworkAvailable(int capability) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork() && r.hasCapability(capability)) {
                r.dispatchNetworkAvailable(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that if no network is found within the timeout time specified in
     * {@link #requestNetwork(NetworkRequest, NetworkCallback, int)} call or if the
     * requested network request cannot be fulfilled (whether or not a timeout was specified).
     *
     * @param capability The network capability. Possible values are:
     *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public void notifyNetworkUnavailable(int capability) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork() && r.hasCapability(capability)) {
                r.dispatchNetworkUnavailable();
            }
        });
    }

    /**
     * Notifies the application that the network is about to be lost, typically because there are
     * no outstanding requests left for it. This may be paired with a
     * {@link #notifyNetworkAvailable} call with the new replacement network for graceful
     * handover. This method is not guaranteed to be called before {@link #notifyNetworkLost}
     * is called, for example in case a network is suddenly disconnected.
     *
     * @param capability The network capability. Possible values are:
     *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public void notifyNetworkLosing(int capability) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork() && r.hasCapability(capability)) {
                r.dispatchNetworkLosing(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that a network disconnects or otherwise no longer satisfies
     * this request or callback.
     *
     * @param capability The network capability. Possible values are:
     *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public void notifyNetworkLost(int capability) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork() && r.hasCapability(capability)) {
                r.dispatchNetworkLost(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that the default network is connected and has declared
     * a new network ready for use.
     */
    public void notifyDefaultNetworkAvailable() {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkAvailable(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that if no network is found within the timeout time specified in
     * {@link #requestNetwork(NetworkRequest, NetworkCallback, int)} call or if the
     * requested network request cannot be fulfilled (whether or not a timeout was specified).
     */
    public void notifyDefaultNetworkUnavailable() {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkUnavailable();
            }
        });
    }

    /**
     * Notifies the application that the network is about to be lost, typically because there are
     * no outstanding requests left for it. This may be paired with a
     * {@link #notifyDefaultNetworkAvailable} call with the new replacement network for graceful
     * handover. This method is not guaranteed to be called before {@link #notifyDefaultNetworkLost}
     * is called, for example in case a network is suddenly disconnected.
     */
    public void notifyDefaultNetworkLosing() {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkLosing(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that a network disconnects or otherwise no longer satisfies
     * this request or callback.
     */
    public void notifyDefaultNetworkLost() {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkLost(getNetworkRecord(r));
            }
        });
    }

    /**
     * Notifies the application that an error occurs on a registered callback. Once called,
     * the callback is automatically unregistered and the callback will no longer receive calls.
     */
    public void notifyQosError() {
        if (!isQosSessionBearerTypeValid()) {
            return;
        }
        mQosCallbackRecords.forEach((r) -> r.dispatchError());
    }

    /**
     * Notifies the application that a Qos Session for EPS/NR first becomes available to
     * the callback or if its attributes have changed.
     */
    public void notifyQosSessionAvailable() {
        if (!isQosSessionBearerTypeValid()) {
            return;
        }
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionAvailable(mQosSessionBearerType));
    }

    /**
     * Notifies the application that a Qos Session is lost.
     */
    public void notifyQosSessionLost() {
        if (!isQosSessionBearerTypeValid()) {
            return;
        }
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionLost(mQosSessionBearerType));
    }

    private NetworkRecord getNetworkRecord(NetworkCallbackRecord ncr) {
        if (ncr.isForDefaultNetwork()) {
            return mNetworkRecords[APN_DEFAULT];
        } else if (ncr.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS)) {
            return mNetworkRecords[APN_IMS];
        } else if (ncr.hasCapability(NetworkCapabilities.NET_CAPABILITY_XCAP)) {
            return mNetworkRecords[APN_XCAP];
        } else if (ncr.hasCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)) {
            return mNetworkRecords[APN_EMERGENCY];
        }
        return null;
    }

    private boolean isQosSessionBearerTypeValid() {
        return mQosSessionBearerType == QosSession.TYPE_EPS_BEARER
                || mQosSessionBearerType == QosSession.TYPE_NR_BEARER;
    }

    private static boolean isNetworkRequestForWifi(NetworkRequest request) {
        return request.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)
                && request.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
    }

    private static final class NetworkRecord {
        private final Network mNetwork;
        private final LinkProperties mLinkProperties;
        private NetworkCapabilities mCapabilities;

        NetworkRecord(Network network, LinkProperties properties) {
            mNetwork = network;
            mLinkProperties = new LinkProperties(properties);
        }

        Network getNetwork() {
            return mNetwork;
        }

        NetworkCapabilities getCapabilities() {
            return mCapabilities;
        }

        LinkProperties getLinkProperties() {
            return mLinkProperties;
        }

        boolean isSameNetwork(Network network) {
            return mNetwork.equals(network);
        }

        void setCapabilities(NetworkRequest request) {
            NetworkCapabilities.Builder builder = new NetworkCapabilities.Builder();

            if (request == null) {
                // Default network capabilities.
                builder.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
                builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
            } else {
                int[] capabilities = request.getCapabilities();
                int[] transportTypes = request.getTransportTypes();

                for (int capability : capabilities) {
                    builder.addCapability(capability);
                }

                for (int transportType : transportTypes) {
                    builder.addTransportType(transportType);
                }

                builder.setNetworkSpecifier(request.getNetworkSpecifier());
            }

            mCapabilities = builder.build();
        }

        void setPcscfServers(@NonNull Collection<InetAddress> pcscfServers) {
            mLinkProperties.setPcscfServers(pcscfServers);
        }
    }

    private static final class QosCallbackRecord {
        private static final QosSession EPS_SESSION =
                new QosSession(10001, QosSession.TYPE_EPS_BEARER);
        private static final QosSession NR_SESSION =
                new QosSession(10002, QosSession.TYPE_NR_BEARER);
        private final QosSocketInfo mSocketInfo;
        private final QosCallback mCallback;
        private final Executor mScheduler;

        QosCallbackRecord(QosSocketInfo socketInfo, QosCallback callback, Executor scheduler) {
            mSocketInfo = socketInfo;
            mCallback = callback;
            mScheduler = scheduler;
        }

        boolean hasCallback(QosCallback callback) {
            return mCallback.equals(callback);
        }

        void dispatchError() {
            mScheduler.execute(() -> {
                mCallback.onError(new QosCallbackException("Qos session unavailable"));
            });
        }

        void dispatchSessionAvailable(int qosSessionType) {
            mScheduler.execute(() -> {
                final List<InetSocketAddress> remoteAddresses;
                if (mSocketInfo.getRemoteSocketAddress() != null) {
                    remoteAddresses = List.of(mSocketInfo.getRemoteSocketAddress());
                } else {
                    remoteAddresses = Collections.emptyList();
                }
                if (qosSessionType == QosSession.TYPE_EPS_BEARER) {
                    mCallback.onQosSessionAvailable(EPS_SESSION,
                            new EpsBearerQosSessionAttributes(1, 41, 41, 39, 39, remoteAddresses));
                } else if (qosSessionType == QosSession.TYPE_NR_BEARER) {
                    mCallback.onQosSessionAvailable(NR_SESSION,
                            new NrQosSessionAttributes(1, 11, 41, 41, 39, 39, 20, remoteAddresses));
                }
            });
        }

        void dispatchSessionLost(int qosSessionType) {
            mScheduler.execute(() -> {
                if (qosSessionType == QosSession.TYPE_EPS_BEARER) {
                    mCallback.onQosSessionLost(EPS_SESSION);
                } else if (qosSessionType == QosSession.TYPE_NR_BEARER) {
                    mCallback.onQosSessionLost(NR_SESSION);
                }
            });
        }
    }

    private static final class NetworkCallbackRecord {
        private final NetworkRequest mRequest;
        private final NetworkCallback mCallback;
        private final Handler mScheduler;
        private boolean mAvailable = false;

        NetworkCallbackRecord(NetworkRequest request, NetworkCallback callback, Handler scheduler) {
            mRequest = request;
            mCallback = callback;
            mScheduler = scheduler;
        }

        boolean hasCallback(NetworkCallback callback) {
            return mCallback.equals(callback);
        }

        boolean hasCapability(int capability) {
            return mRequest.hasCapability(capability);
        }

        boolean isForDefaultNetwork() {
            return mRequest == null;
        }

        void dispatchNetworkAvailable(NetworkRecord nr) {
            if (nr == null || nr.getCapabilities() == null || mAvailable) {
                return;
            }
            mAvailable = true;
            mScheduler.postDelayed(() -> {
                mCallback.onAvailable(nr.getNetwork());
                mCallback.onCapabilitiesChanged(nr.getNetwork(), nr.getCapabilities());
                mCallback.onLinkPropertiesChanged(nr.getNetwork(), nr.getLinkProperties());
            }, DEFAULT_EVENT_DELAY_TIME_MILLIS);
        }

        void dispatchNetworkUnavailable() {
            mAvailable = false;
            mScheduler.postDelayed(() -> {
                mCallback.onUnavailable();
            }, DEFAULT_EVENT_DELAY_TIME_MILLIS);
        }

        void dispatchNetworkLosing(NetworkRecord nr) {
            if (nr == null) {
                return;
            }
            mScheduler.postDelayed(() -> {
                mCallback.onLosing(nr.getNetwork(), 0);
            }, DEFAULT_EVENT_DELAY_TIME_MILLIS);
        }

        void dispatchNetworkLost(NetworkRecord nr) {
            if (nr == null) {
                return;
            }
            mAvailable = false;
            mScheduler.postDelayed(() -> {
                mCallback.onLost(nr.getNetwork());
            }, DEFAULT_EVENT_DELAY_TIME_MILLIS);
        }
    }
}
