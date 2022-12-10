package com.android.imsstack.enabler.uce.interf;

/**
 * Interface used by the framework to receive the response of the publish request.
 */
public interface PublishResponse {
  /**
   * Provide the framework with a subsequent network response update to
   * {@link IUceApi#publishCapabilities(String, PublishResponse)}.
   *
   * If this network response also contains a “Reason” header, then the
   * {@link #onNetworkResponse(int, String, int, String)} method should be used instead.
   *
   * @param sipCode The SIP response code sent from the network for the operation
   * token specified.
   * @param reason The optional reason response from the network. If there is a reason header
   * included in the response, that should take precedence over the reason provided in the
   * status line. If the network provided no reason with the sip code, the string should be
   * empty.
   */
    void onNetworkResponse(int sipCode, String reason);

  /**
   * Provide the framework with a subsequent network response update to
   * {@link IUceApi#publishCapabilities(String, PublishResponse)} that also
   * includes a reason provided in the “reason” header. See RFC3326 for more
   * information.
   *
   * @param sipCode The SIP response code sent from the network.
   * @param reasonPhrase The optional reason response from the network. If the
   * network provided no reason with the sip code, the string should be empty.
   * @param reasonHeaderCause The “cause” parameter of the “reason” header
   * included in the SIP message.
   * @param reasonHeaderText The “text” parameter of the “reason” header
   * included in the SIP message.
   */
    void onNetworkResponse(int sipCode, String reasonPhrase, int reasonHeaderCause,
        String reasonHeaderText);

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
}
