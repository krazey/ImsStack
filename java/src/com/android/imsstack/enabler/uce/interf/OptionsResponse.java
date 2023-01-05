package com.android.imsstack.enabler.uce.interf;

import java.util.List;

public interface OptionsResponse {
  /**
   * Notify the framework that the command associated with this callback has failed.
   *
   * @param code The reason why the associated command has failed.
   *    COMMAND_CODE_SERVICE_UNKNOWN = 0 (Service is unknown)
   *    COMMAND_CODE_GENERIC_FAILURE = 1 (The command failed with an unknown error)
   *    COMMAND_CODE_INVALID_PARAM = 2 (Invalid parameter(s))
   *    COMMAND_CODE_FETCH_ERROR = 3 (Fetch error)
   *    COMMAND_CODE_REQUEST_TIMEOUT = 4 (Request timed out)
   *    COMMAND_CODE_INSUFFICIENT_MEMORY = 5 (Failure due to insufficient memory available)
   *    COMMAND_CODE_LOST_NETWORK_CONNECTION = 6 (Network connection is lost)
   *    COMMAND_CODE_NOT_SUPPORTED = 7 (Requested feature/resource is not supported)
   *    COMMAND_CODE_NOT_FOUND = 8 (Contact or resource is not found)
   *    COMMAND_CODE_SERVICE_UNAVAILABLE = 9 (Service is not available)
   *    COMMAND_CODE_NO_CHANGE = 10 (Command resulted in no change in state, ignoring)
   */
    void onCommandError(int code);

  /**
   * Send the response of a SIP OPTIONS capability exchange to the framework.
   * @param sipCode The SIP response code that was sent by the network in response
   * to the request sent by {@link IUceApi#sendOptionsCapabilityRequest}.
   * @param reason The optional SIP response reason sent by the network.
   * If none was sent, this should be an empty string.
   * @param theirCaps the contact's UCE capabilities associated with the
   * capability request.
   */
  void onNetworkResponse(int sipCode, String reason, List<String> theirCaps);
}
