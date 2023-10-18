package com.android.imsstack.enabler.uce.interf;

import java.util.Set;

/**
 * Interface used by the framework to respond to OPTIONS requests.
 */
public interface RemoteOptionsCallback {

  /**
   * Respond to a remote capability request from the contact specified with the
   * capabilities of this device.
   * @param ownCapabilities The capabilities of this device.
   * @param isBlocked Whether or not the user has blocked the number requesting the
   *         capabilities of this device. If true, the device should respond to the OPTIONS
   *         request with a 200 OK response and no capabilities.
   */
  void onRespondToCapabilityRequest(Set<String> ownCapabilities,
      boolean isBlocked);

  /**
   * Respond to a remote capability request from the contact specified with the
   * specified error.
   * @param code The SIP response code to respond with.
   * @param reason A non-null String containing the reason associated with the SIP code.
   */
  void onRespondToCapabilityRequestWithError(int code, String reason);
}
