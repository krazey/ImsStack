# ImsStack - AOSP userspace IMS for community ROMs

This repository is a community-maintained fork of
[AOSP ImsStack](https://android.googlesource.com/platform/packages/modules/ImsStack/+/refs/heads/android17-release)
based on the Android 17 release. It provides the SIP/IMS signalling and Android
`ImsService` side of a userspace IMS implementation. An explicit compatibility
path also allows this release stack to build on Android 16 QPR2.

The project is split across three repositories:

| Repository | Role |
| --- | --- |
| [ImsStack](https://github.com/krazey/ImsStack) | IMS registration, SIP signalling, MMTEL, SMS over IMS, UT and framework integration. |
| [ImsMedia](https://github.com/krazey/ImsMedia) | RTP/media service, codec plumbing and Android audio input/output. |
| [imsstack-carrier-config-ext](https://github.com/krazey/imsstack-carrier-config-ext) | Shared carrier profiles, including the transferable carrier policy imported from [PhhIms](https://github.com/krazey/ims). |

Device-specific package selection, CarrierConfig, framework overlays, QNS/IWLAN
policy, sepolicy and audio-HAL work belong in the device or common device tree.
The three repositories above intentionally contain no Samsung, Exynos or other
device-family integration.

This is not a universal drop-in or carrier-certified IMS implementation. A
working ROM still depends on carrier provisioning, the radio/data HAL, a usable
IMS APN, P-CSCF discovery, Android telephony configuration and correct audio
routing.

## Upstream base and branches

The current work is based on the Android 17 release snapshots:

- ImsStack: `1e3981c23117d75d5ad304b32bb030327ad33eaa`
- ImsMedia: `0101cb4caf48dbcc3b8fbc6ab3e41137b90b8e67`
- AOSP tag: `android-17.0.0_r1`

In both module repositories, `android17-release` preserves the release base and
`main` carries the small community patch series on top. This makes future
upstream comparison and rebasing straightforward.

The current ImsStack native adapter is paired with the Android 17 ImsMedia API.
Android 16 QPR2 products must therefore use `ImsMedia/main` as well. The
`android16-qpr2-release` ImsMedia branch is retained only as an upstream
reference; it is not source-compatible with this ImsStack revision.

The carrier extension preserves the history of AOSP's former
`java/assets/carrier_config` directory through
`5f934dd60abdf29c2581dbdd2dd9dedb76277a18`, immediately before commit
`0fa1e52eec0251bafd5a52411f5bb291b8cc4c58` removed the public profiles in
favour of an external library. The history is path-filtered, so commit hashes
change while original authors, messages and dates remain intact.

## Current project status

The Android 17 ImsStack/ImsMedia pair now completes a full LineageOS 23.2
(Android 16 QPR2) product build. Extended Samsung Exynos9810 `starlte` testing
with O2 Germany confirms repeated LTE and IWLAN registration, incoming and
outgoing calls, AMR-WB media, incoming IMS SMS on both accesses, CS-routed
USSD and cellular PANI while the master Location switch is disabled. The
latest soak completed ten connected calls and eleven media sessions without
an ImsStack or ImsMedia crash or ANR. This remains narrow engineering
validation, not carrier certification.

| Area | Current status |
| --- | --- |
| IMS registration | Repeated LTE/IWLAN registration and idle reselection are confirmed on `starlte` with O2 Germany. A pre-fix IWLAN-to-LTE switch sometimes waited for the 128-second SIP Timer F; the complete local-transport release below still needs a device retest. Long-idle recovery, multi-SIM and other carriers remain open. |
| VoLTE voice | Repeated incoming and outgoing call setup, media, reject and clean hangup are confirmed with AMR-WB. DTMF, hold, call waiting and broader codec fallback still require testing. |
| VoWiFi | Registration and repeated calls are confirmed. Idle switching is break-before-make; seamless active-call handover is not validated. |
| SMS over IMS | Two MT messages were received successfully, one over LTE and one over IWLAN. MO SMS still requires validation. |
| USSD | Two `*100#` requests completed successfully through the circuit-switched modem, with a temporary 2G transition. IMS USSI is not validated. |
| Audio | Eleven AMR-WB media sessions opened and closed cleanly in the latest soak. Routing, volume, echo, Bluetooth and headset behaviour remain device/HAL dependent. |
| EVS | Disabled. The open Android 17 ImsMedia source negotiates EVS but contains no working EVS encoder or decoder. |
| Android 16 QPR2 | Full product build and basic O2 Germany voice bring-up are confirmed with the compatibility adapter. |
| Android 17 | This is the native source API level, but a full Android 17 product build and device test are still pending. |
| Video calling | Present upstream but excluded from the initial validation scope. Keep the carrier feature disabled until media and dialer handling are tested. |
| Emergency IMS | Present upstream but should be gated during bring-up. Keep a tested CS emergency fallback and do not enable emergency MMTEL without appropriate network testing. |
| RCS | Present upstream but gated independently from MMTEL. It is not part of the initial voice/SMS migration. |

## Changes carried on top of AOSP

### ImsStack

- Track IMSI, SPN, GID1-prefix and GID2-prefix selectors used by carrier
  policy, and reload configuration when those SIM fields change.
- Load the optional carrier extension after the canonical carrier-ID profile,
  allowing reusable MCC-MNC policy without replacing AOSP selection.
- Reapply carrier-extension values after Android CarrierConfig so standard
  settings such as the outgoing Request-URI type are not silently reset.
- Treat imported VoLTE, VoWiFi and SMS switches as disable-only gates over
  Android CarrierConfig. Imported data cannot incorrectly enable a service
  disabled by the framework's carrier policy.
- Separate cellular and IWLAN voice-precondition settings so LTE bearer QoS
  assumptions are not automatically reused for Wi-Fi calls.
- Expose emergency-MMTEL and RCS feature advertisement as overlayable resource
  switches.
- Let products without dedicated-bearer QoS reporting disable IMS
  preconditions and use the default bearer through an overlayable switch.
- Release an idle IWLAN or cross-SIM registration locally when Android begins
  tearing down its data bearer. Close every SIP socket bound to the departing
  local address before registration destroys IPsec state, rather than retaining
  the dying interface until SIP Timer F expires. Active IMS calls retain the
  graceful stop path.
- Disable EVS by default until a working open codec implementation exists.
- Provide an opt-in Android 16 QPR2 adapter for the domain-selection emergency
  callback introduced in Android 17.

### ImsMedia

- Terminate the product package list correctly in `imsmedia.mk`.
- Propagate deferred media-node startup errors instead of reporting a running
  call after capture or playback failed to start.
- Require AAudio streams to reach `STARTED`, close failed streams and use a
  consistent startup timeout.
- Retry AAudio in shared mode when a HAL cannot provide an exclusive
  low-latency stream.
- Clean up partially opened streams during initial startup and recovery.
- Remove an invalid unused JNI registration that aborts ImsMedia at load time.
- Carry narrowly scoped system-ext policy for handed-off RTP/RTCP sockets and
  SchedulingPolicyService access.

### Carrier configuration extension

The extension currently contains:

- 384 carrier-ID/MCC-MNC profiles from the last public AOSP carrier set;
- 988 generated MCC-MNC extension profiles;
- 1,304 directly transferable mappings from the PhhIms carrier database;
- deterministic generation and source checksums for later refreshes.

The carrier import is pinned to the PhhIms source snapshot
`69ff68aad4e82fe56406b0893a7ebfe60d7debec`.

Only policy with a direct ImsStack equivalent is imported. Number rewriting,
special short-code routing, arbitrary SIP-header shaping and carrier-specific
CS fallback rules are not guessed. Those paths need protocol traces and generic
stack mechanisms before adding new carrier data.

## Android source integration

The paths below match the AOSP/LineageOS source layout used by the modules.

### Local manifest example

Use the remote name configured for your tree:

```xml
<project path="packages/modules/ImsStack"
         remote="github"
         name="krazey/ImsStack"
         revision="main" />
<project path="packages/modules/ImsMedia"
         remote="github"
         name="krazey/ImsMedia"
         revision="main" />
<project path="vendor/lineage/imsstack-carrier-config-ext"
         remote="github"
         name="krazey/imsstack-carrier-config-ext"
         revision="main" />
```

If the platform manifest already owns `packages/modules/ImsStack` or
`packages/modules/ImsMedia`, replace or override those projects rather than
adding duplicate paths.

### Product packages

Add the shared carrier module to the Soong namespace, inherit ImsMedia's
product list and select ImsStack as the MMTEL provider:

```make
PRODUCT_SOONG_NAMESPACES += \
    vendor/lineage/imsstack-carrier-config-ext

$(call inherit-product, packages/modules/ImsMedia/imsmedia.mk)
$(call soong_config_set,imsstack_namespace,use_carrier_config_ext,true)

PRODUCT_PACKAGES += \
    ImsStack \
    Iwlan \
    QualifiedNetworksService
```

The upstream carrier-extension selector is string-valued. Use
`soong_config_set` for it; `soong_config_set_bool` has a different Soong type.

The Android 17 stack uses a domain-selection emergency callback that does not
exist in Android 16. Products building this release stack on Android 16 QPR2
must select the compatibility adapter:

```make
$(call soong_config_set_bool,imsstack_namespace,use_android16_telephony_compat,true)
```

This disables only domain-selection emergency-mode notifications. Emergency
callback-mode tracking remains available. Keep emergency MMTEL disabled on an
Android 16 integration unless the missing framework path is backported and
validated.

`imsmedia.mk` adds `ImsMediaService`, `libimsmedia` and its preinstall
allowlist. `ImsStack` itself is a privileged, platform-signed `system_ext` app.

Remove the previous userspace IMS provider and its package-specific permission
files from the product. Two MMTEL providers must not be selected at the same
time.

### Telephony provider overlay

The Telephony resource overlay must point MMTEL at ImsStack:

```xml
<string name="config_ims_mmtel_package" translatable="false">
    com.android.imsstack
</string>
```

Do not select ImsStack as the RCS provider merely because the service contains
upstream RCS code. Enable and bind RCS only after a separate validation pass.

### Initial device-capability overlay

The fork makes emergency MMTEL, RCS advertisement and dedicated-bearer QoS
support overlayable. A conservative bring-up overlay targeting
`com.android.imsstack` should contain:

```xml
<resources>
    <bool name="config_imsstack_emergency_mmtel_feature">false</bool>
    <bool name="config_imsstack_rcs_feature">false</bool>
</resources>
```

The AOSP-compatible defaults remain `true`; products must explicitly gate
features they have not validated. Legacy data HALs that cannot report IMS
dedicated-bearer QoS through Android's `QosCallback` must also set:

```xml
<bool name="config_imsstack_dedicated_bearer_qos_supported">false</bool>
```

That override disables SDP QoS preconditions, treats voice media as usable on
the default bearer and prevents call setup from waiting for a callback the
device cannot deliver. Do not set it on devices that expose dedicated-bearer
state correctly. The override is validated on the initial `starlte` setup: it
removed the eight-second resource-reservation failure from repeated calls.

### ImsMedia SELinux policy

ImsStack creates RTP and RTCP sockets and hands them to ImsMedia over Binder.
The ImsMedia repository carries narrowly scoped system-ext policy for this
cross-domain descriptor use and media-thread scheduling. Include it from the
product's board configuration:

```make
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    packages/modules/ImsMedia/sepolicy/system_ext/private
```

Inheriting `imsmedia.mk` does not add this policy directory. Without it, media
session startup or teardown can fail with `radio`-to-`platform_app` UDP socket
denials. Changes under this directory require a policy/image rebuild; pushing
only an APK or shared library cannot update SELinux policy. The policy includes
the handed-off socket `shutdown` permission; the corresponding AVC disappeared
in the soak test, but this media permission does not release ImsStack's SIP
registration sockets during an IWLAN bearer change.

### Serving-cell identity with Location disabled

ImsStack builds cellular `P-Access-Network-Info` from the serving cell. On
Android 16 QPR2, telephony hides cell identity while the master Location switch
is disabled unless the package is present in
`config_serviceStateLocationAllowedPackages`. The manifest permissions and
`allow-ignore-location-settings` sysconfig entry do not bypass this telephony
check.

Extend the product's framework resource overlay while preserving any existing
entries:

```xml
<string-array name="config_serviceStateLocationAllowedPackages">
    <item>com.android.phone</item>
    <item>com.android.imsstack</item>
</string-array>
```

This does not enable Location or GPS. It lets the privileged IMS service retain
the serving-cell identity needed for cellular PANI. Some carriers reject an
otherwise valid outgoing call when `utran-cell-id-3gpp` is missing, so test with
the user-visible Location switch disabled. This overlay is validated on the
initial `starlte` setup: Location remained disabled while all 42 observed LTE
PANI headers contained a serving-cell identity.

### CarrierConfig and data-service policy

CarrierConfig remains authoritative for whether Android exposes IMS services.
At minimum, validate the relevant carrier entries for:

- `carrier_volte_available_bool`
- `carrier_vt_available_bool`
- `carrier_wfc_ims_available_bool`
- `carrier_wfc_supports_wifi_only_bool`
- `carrier_default_wfc_ims_enabled_bool`
- `carrier_default_wfc_ims_mode_int`
- `imssms.sms_over_ims_supported_bool`

Do not enable these globally in a production tree unless every affected SIM
profile is intended to inherit the same policy.

Some legacy data HALs cannot request an IMS bearer with the MMTEL capability.
For those devices only, `ims.request_ims_pdn_without_mmtel_bool` may be needed.
Likewise, LTE/IWLAN handover policy belongs in the device CarrierConfig overlay
because the safe behaviour depends on the radio and data HAL. A device that
requires break-before-make must not advertise seamless handover merely because
the userspace stack supports both accesses.

### Idle IWLAN teardown

During a break-before-make IWLAN-to-cellular switch, Android can report the IMS
data bearer as disconnecting and remove IKE before a SIP de-REGISTER response
can return. A graceful stop on that dying path retains the SIP sockets and their
IPsec interface references until the 128-second non-INVITE Timer F expires,
delaying creation of the replacement cellular IMS bearer.

When the registered access is IWLAN or cross-SIM and no IMS call is active,
ImsStack therefore reports the data-disconnected state, closes every SIP socket
bound to the old local IMS address and then destroys registration and IPsec
state. Closing the shared transport first is important: destroying only the
registration clients can leave the SIP listener sockets attached to the old
IPsec interface. The normal data-deactivated event remains responsible for
starting registration on the next bearer. LTE data loss and active IMS calls
retain the existing graceful stop path.

This change addresses idle bearer reselection only; it does not claim seamless
active-call handover. After integrating it, verify that an idle IWLAN-to-LTE
switch logs `DestroyLocalTransport`, ends `DestroyAllSockets` with zero sockets,
does not leave an `ipsec` usage-count wait and registers VoLTE within the
device's normal break-before-make interval.

### Always-on VPN during QNS startup

QNS must classify the physical Wi-Fi link as IPv4, IPv6 or dual stack before it
can apply the carrier's IPv6-only Wi-Fi policy. Android's app-default network
callback can instead return an always-on, non-bypassable VPN when that VPN
starts before QNS. In that boot order QNS never sees the Wi-Fi link properties,
leaves `linkProtocolType:UNKNOWN` and treats the link as blocked when
`qns.block_ipv6_only_wifi_bool` is enabled. No IWLAN data call reaches ImsStack.

Products using AOSP QNS should make `IwlanNetworkStatusTracker` register a
system-default network callback and declare
`android.permission.NETWORK_SETTINGS`.
This keeps QNS on ConnectivityService's physical system default while IMS and
other app traffic can remain routed through the VPN. It does not weaken the
IPv6-only safeguard. The workaround before that QNS fix is to disable the VPN,
toggle Wi-Fi so QNS learns the link protocol, and then re-enable the VPN.

This issue is independent of the selected IMS application. A specific IMS
network request keeps SIP and media sockets off the VPN, but it cannot correct
QNS policy when QNS has already suppressed IWLAN at boot.

## Building and tests

Build the directly affected unit-test targets first:

```sh
m ImsStackJavaTests \
  ImsStackNativeConfigTests \
  ImsStackNativeEnablerAosTests \
  ImsStackNativeEnablerMtcTests \
  ImsMediaJavaUnitTests \
  ImsMediaNativeCoreTests \
  QualifiedNetworksServiceTests
```

Then build the full product for the first flash. Provider selection, privileged
apps, native libraries, overlays, carrier data and potentially vendor policy all
change together, so a module-only first flash is not a reliable test.

For a later `userdebug` iteration that changes only ImsStack Java, resources or
the compiled carrier-extension assets, build and replace the complete app
directory. Products that fold `system_ext` into the system image use paths like
these:

```sh
m ImsStack
adb root
adb remount
adb push \
  out/target/product/<product>/system/system_ext/priv-app/ImsStack \
  /system/system_ext/priv-app/
adb reboot
```

The directory push keeps the APK and matching dex-preopt files together. Check
`adb shell pm path com.android.imsstack` first on products with a standalone
`system_ext` image because their local and mounted paths can differ.

This shortcut is not sufficient for native ImsStack or ImsMedia changes,
SELinux policy, framework resources, CarrierConfig or product/vendor overlays.
Build and update every affected artifact or flash the containing image. In
particular, media-policy changes require a rebuilt system policy.

## First-boot checks

Verify that both new services are installed and the old provider is absent:

```sh
adb shell pm path com.android.imsstack
adb shell pm path com.android.telephony.imsmedia
adb shell pm path me.phh.ims
adb shell dumpsys activity service \
    com.android.imsstack/.imsservice.ImsService
adb shell dumpsys ims
adb shell dumpsys carrier_config
```

The first two package lookups should report `system_ext` paths. The PhhIms
lookup should report an unknown package. IMS registration should settle without
a continuous acquire/register/reconnect loop.

## Minimum device test matrix

Run tests in this order so registration and basic media failures are separated
from more complex dialer behaviour:

1. Cold-boot LTE registration, airplane-mode recovery and recovery after a
   temporary IMS bearer loss.
2. Disable the master Location switch, place an outgoing VoLTE call and verify
   that cellular PANI still contains the serving-cell identity.
3. Outgoing and incoming VoLTE calls with ringback, two-way audio, mute,
   speaker, wired headset, Bluetooth, DTMF and clean local/remote hangup.
4. AMR-NB fallback and AMR-WB/HD negotiation. EVS must not appear in SDP.
5. Incoming reject, busy, CANCEL, hold/resume, call waiting, swap and conference
   behaviour supported by the dialer/network.
6. MO and MT SMS over LTE, including RP result and fallback behaviour.
7. VoWiFi registration and calls, including Wi-Fi-only airplane mode and a
   cold boot with the user's always-on VPN already enabled.
8. Idle LTE to IWLAN and IWLAN to LTE switching, including after a completed
   Wi-Fi call. Verify that the idle IWLAN registration is released locally and
   the switch does not wait for the 128-second SIP Timer F.
9. Active-call access changes according to the device's declared handover
   policy. Never infer success from a stale UI icon; verify RTP and call state.
10. Dual-SIM registration, default voice-SIM changes and both physical slots.
11. Reboot and repeat the basic registration/call/SMS checks after every
    carrier-policy or handover change.

Emergency IMS must use a separate, controlled test plan. During initial
bring-up, retain and verify the device's existing CS emergency path.

## Carrier migration notes

The extension first loads the canonical Android carrier-ID profile, then merges
an optional `carrier_config_ext_mccmnc_<MCC><MNC>.xml` profile. IMSI, SPN,
GID1-prefix and GID2-prefix selectors remain data driven.

Important early regression targets include:

- Jio protected UDP/IPsec response routing;
- Tele2 Kazakhstan identity, access headers, number handling and CSFB;
- Singtel SIP message size and header requirements;
- Vodafone Turkey short codes and access-network headers;
- LG U+ authentication behaviour.

The PhhIms implementation of one of these carriers is reference behaviour, not
code to transplant automatically. Prefer a generally useful ImsStack mechanism
plus carrier data, backed by stock and failing traces.

To refresh generated policy after a PhhIms database update, follow the
[carrier-extension README](https://github.com/krazey/imsstack-carrier-config-ext#refreshing-from-phhims).
Normal product builds consume committed XML and do not require PhhIms or Python.

## Debugging

Useful framework state:

```sh
adb shell dumpsys ims
adb shell dumpsys telephony.registry
adb shell dumpsys carrier_config
adb shell dumpsys connectivity
adb shell dumpsys package com.android.imsstack
adb shell dumpsys package com.android.telephony.imsmedia
```

For a broad first-pass capture:

```sh
adb logcat -b all -v threadtime | grep -iE \
  'ImsStack|ImsMedia|MmTel|Iwlan|Qns|P-CSCF|P-Access-Network-Info|REGISTER|401|IPsec|INVITE|PRACK|ACK|BYE|CANCEL|RTP|SMS|QosCallback|avc:'
```

Useful first checks for failures seen during the initial bring-up:

| Symptom | First check |
| --- | --- |
| Call ends about eight seconds after `183`/PRACK with `LOCAL_CALL_RESOURCE_RESERVATION_FAILED` | The device may not expose dedicated-bearer state through `QosCallback`; verify the device-capability overlay. |
| Cellular INVITE gets `404`/`UNALLOCATED_NUMBER`, while the same SIP URI works over IWLAN | Compare cellular PANI and verify serving-cell access with Location disabled. |
| ImsMedia `openSession` or graph startup fails beside UDP socket AVCs | Verify that the ImsMedia system-ext policy directory is included and flashed. |
| ImsMedia logs a UDP socket `shutdown` AVC during hangup | Verify that the current ImsMedia system-ext policy was included in the flashed policy. |
| IWLAN-to-LTE waits while an `ipsec` interface remains in use | Look for de-REGISTER immediately followed by IKE teardown. The fixed idle path logs `release idle IWLAN registration locally` and `DestroyLocalTransport`; `DestroyAllSockets(E)` should report zero sockets. |
| VoWiFi never starts when an always-on VPN is active at boot | If QNS reports Wi-Fi available but `linkProtocolType:UNKNOWN` and `iwlanEnable:false`, apply the system-default-network QNS fix. This occurs before ImsStack registration. |
| Android 16 build reports many ImsMedia accessor or `VideoConfig` errors | The QPR2 ImsMedia source was selected accidentally; use the paired `ImsMedia/main`. |

Also capture the radio buffer and, for crashes or boot failures, the previous
kernel log. Logs can contain phone numbers, IMS identities, IP addresses and
carrier credentials or challenges; redact them before publishing.

## Keeping changes in the correct repository

- Put generally valid SIP, registration, framework and policy mechanisms in
  ImsStack.
- Put generally valid RTP, codec and Android audio fixes in ImsMedia.
- Put carrier values with existing typed equivalents in
  `imsstack-carrier-config-ext`.
- Put radio/data-HAL workarounds, package selection, overlays, sepolicy and
  audio routing in the device tree.

Keep protocol changes, media changes, carrier data and device integration in
separate commits. This makes regressions bisectable and allows fixes to move
upstream without carrying unrelated device policy.

## License

ImsStack and ImsMedia follow their AOSP Apache-2.0 licensing. The carrier
extension contains the Apache-2.0 AOSP carrier baseline and generated files
derived from GPL-2.0-only PhhIms data; individual files and the extension's
license metadata identify the applicable terms.
