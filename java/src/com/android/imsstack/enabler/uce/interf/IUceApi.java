package com.android.imsstack.enabler.uce.interf;

import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.UceEventListener;

import android.net.Uri;

import java.util.Set;
import java.util.Collection;

public interface IUceApi {
    /**
     * Add the listener to listen to publish event trigger or received options request from remote.
     *
     * @param listener : listener
     */
    void setListener(UceEventListener listener);

    /**
     * Notify the uce agent that the Carrier Config has changed.
     *
     */
    void carrierConfigChanged();
    /**
     * The capabilities of this device have been updated and should be published to the network.
     *
     * @param pidfXml : Xml body of device capability.It will be included of Publish message body.
     * @param cb : Callback to inform the processing result of the request
     */
    void publishCapabilities(String pidfXml, PublishResponse cb);

   /**
     * Query remote capability via Subscribe
     *
     * @param uris : remote MSISDNs for capability discovery via Presence.
     * @param cb : Callback to inform the processing result of the request
     */
    void subscribeCapabilities(Collection<Uri> uris, SubscribeResponse cb);

    /**
     * Query remote capability via Options
     *
     * @param contactUri : remote MSISDN for capability discovery via Options
     * @param myCapabilities : the feature tags of capability.It will be added to Contact header
     * @param cb : Callback to inform the processing result of the request
     */
    void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
        OptionsResponse cb);
}
