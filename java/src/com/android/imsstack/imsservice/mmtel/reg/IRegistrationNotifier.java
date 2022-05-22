package com.android.imsstack.imsservice.mmtel.reg;

import android.annotation.NonNull;
import android.net.Uri;

import java.util.Set;

public interface IRegistrationNotifier {
    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param networkType the radio access technology.
     * @param featureTags Type of Set<String>.
     */
    public void notifyRegistered(int networkType, @NonNull Set<String> featureTags);

    /**
     * Notify the application that the device is trying to connect to the IMS network.
     *
     * @param networkType the radio access technology.
     * @param featureTags Type of Set<String>.
     */
    public void notifyRegistering(int networkType, @NonNull Set<String> featureTags);

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param reason the disconnected reason.
     */
    public void notifyDeregistered(int reason);

    /**
     * Notify the framework that the handover from the current radio technology
     * to the other technology has failed.
     *
     * @param networkType the current network type (before handover)
     * @param reason the handover failure reason.
     */
    public void notifyTechnologyChangeFailed(int networkType, int reason);

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter
     * out this device's {@link Uri}s during conference calling.
     *
     * @param uris the network provisioned public user identities.
     */
    public void notifyAssociatedUriChanged(Uri[] uris);
}
