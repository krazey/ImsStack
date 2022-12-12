package com.android.imsstack.enabler.uce.interf;

import android.net.Uri;

import java.util.Set;
/**
 * The interface that is used by the framework to listen to events from the vendor RCS stack
 * regarding capabilities exchange using presence server and OPTIONS.
 */
public interface UceEventListener {
    /**
     * Notify the framework that the ImsService has refreshed the PUBLISH
     * internally, which has resulted in a new PUBLISH result.
     * This method must return both SUCCESS (200 OK) and FAILURE (300+) codes in order to
     * keep the AOSP stack up to date.
     *
     * @param reasonCode : listener
     * @param reasonPhase : listener
     * @param reasonHeaderCause : listener
     * @param reasonHeaderText : listener
     */
    void onPublishUpdated(int reasonCode, String reasonPhase, int reasonHeaderCause,
        String reasonHeaderText);

    /**
     * Notify the framework that the device's capabilities have been unpublished
     * from the network.
     *
     */
    void onUnPublish();

  /**
   * Trigger the framework to provide a capability update using
   * {@link IUceApi#publishCapabilities}.
   *      CAPABILITY_UPDATE_TRIGGER_UNKNOWN = 0 (The reason for the request is unknown)
   *      CAPABILITY_UPDATE_TRIGGER_ETAG_EXPIRED = 1 (When the Entity Tag (ETag) is  expiring)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED = 2
   *                  (requested due to moving to LTE with VoPS disabled)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED = 3
   *                  (requested due to moving to LTE with VoPS enabled)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD = 4 (requested due to moving to eHRPD)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_HSPAPLUS = 5 (requested due to moving to HSPA+)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G = 6 (requested due to moving to 3G)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_2G = 7 (requested due to moving to 2G)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_WLAN = 8 (requested due to moving to WLAN)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_IWLAN = 9 (requested due to moving to IWAL)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_DISABLED = 10
   *                (due to moving to 5G NR with VoPS disabled)
   *      CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_ENABLED = 11
   *                (due to moving to 5G NR with VoPS enabled)
   *
   * This is typically used when trying to generate an initial PUBLISH for a new subscription to
   * the network. The device will cache all presence publications after boot until this method is
   * called the first time.
   * @param publishTriggerType The reason for the capability update request.
   */
    void onRequestPublishCapabilities(int publishTriggerType);

    /**
     * Notify the framework that the ImsService has received an Options request from the Remote.
     *
     * @param contactUri The URI associated with the remote contact that is
     * requesting capabilities.
     * @param remoteCapabilities The remote contact's capability information. The capability
     * information is in the format defined in RCC.07 section 2.6.1.3.
     * @param cb The callback of this request which is sent from the remote user.
     */
    void onRemoteCapabilityRequest(Uri contactUri, Set<String> remoteCapabilities,
        RemoteOptionsCallback cb);
}
