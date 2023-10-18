package com.android.imsstack.enabler.uce.interf;

public class UceApiConstant {

  /**
   * Service is unknown.
   */
  public static final int COMMAND_CODE_SERVICE_UNKNOWN = 0;

  /**
   * The command failed with an unknown error.
   */
  public static final int COMMAND_CODE_GENERIC_FAILURE = 1;

  /**
   * Invalid parameter(s).
   */
  public static final int COMMAND_CODE_INVALID_PARAM = 2;

  /**
   * Fetch error.
   */
  public static final int COMMAND_CODE_FETCH_ERROR = 3;

  /**
   * Request timed out.
   */
  public static final int COMMAND_CODE_REQUEST_TIMEOUT = 4;

  /**
   * Failure due to insufficient memory available.
   */
  public static final int COMMAND_CODE_INSUFFICIENT_MEMORY = 5;

  /**
   * Network connection is lost.
   */
  public static final int COMMAND_CODE_LOST_NETWORK_CONNECTION = 6;

  /**
   * Requested feature/resource is not supported.
   */
  public static final int COMMAND_CODE_NOT_SUPPORTED = 7;

  /**
   * Contact or resource is not found.
   */
  public static final int COMMAND_CODE_NOT_FOUND = 8;

  /**
   * Service is not available.
   */
  public static final int COMMAND_CODE_SERVICE_UNAVAILABLE = 9;

  /**
   * Command resulted in no change in state, ignoring.
   */
  public static final int COMMAND_CODE_NO_CHANGE = 10;

  /**
   * A capability update has been requested but the reason is unknown.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_UNKNOWN = 0;

  /**
   * A capability update has been requested due to the Entity Tag (ETag) expiring.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_ETAG_EXPIRED = 1;

  /**
   * A capability update has been requested due to moving to LTE with VoPS disabled.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED = 2;

  /**
   * A capability update has been requested due to moving to LTE with VoPS enabled.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED = 3;

  /**
   * A capability update has been requested due to moving to eHRPD.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD = 4;

  /**
   * A capability update has been requested due to moving to HSPA+.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_HSPAPLUS = 5;

  /**
   * A capability update has been requested due to moving to 3G.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G = 6;

  /**
   * A capability update has been requested due to moving to 2G.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_2G = 7;

  /**
   * A capability update has been requested due to moving to WLAN
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_WLAN = 8;

  /**
   * A capability update has been requested due to moving to IWLAN
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_IWLAN = 9;

  /**
   * A capability update has been requested due to moving to 5G NR with VoPS disabled.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_DISABLED = 10;

  /**
   * A capability update has been requested due to moving to 5G NR with VoPS enabled.
   */
  public static final int CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_ENABLED = 11;

}