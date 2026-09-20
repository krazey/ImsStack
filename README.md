# ImsStack for community Android builds

This fork provides IMS signalling and Android's `ImsService` implementation for
community product integration. It uses the paired Android 17 ImsMedia fork for
RTP/media and the separate CarrierSettings APK for carrier data. Android 16 QPR2
compatibility remains an explicit build option.

This fork is based on AOSP `android17-release` at
`1e3981c23117d75d5ad304b32bb030327ad33eaa`. Use the matching Android 17 ImsMedia
API even on an Android 16 product; the compatibility option adapts telephony APIs,
not the media interface.

## Repository responsibilities

| Project | Responsibility |
| --- | --- |
| ImsStack | SIP, IMS registration/calls/SMS, Android service integration and policy selection |
| ImsMedia | RTP/RTCP, AMR and device audio/video paths |
| CarrierSettings | Separately built APK containing AOSP profiles, reviewed Google data and explicit corrections |
| Device/product tree | Telephony package selection, radio/APNs, QNS/IWLAN, overlays, audio policy and SELinux integration |

CarrierSettings is `org.lineageos.carriersettings`, built as `CarrierSettings`.
Its assets are read at runtime; they are not statically linked into ImsStack.
The former `imsstack-carrier-config-ext` module/Soong option is retired for this
revision. Changing compatible carrier data only requires rebuilding its APK.

## Product integration

Place the repositories at:

```text
packages/modules/ImsStack
packages/modules/ImsMedia
packages/apps/CarrierSettings
```

Add to the product makefile:

```make
$(call inherit-product, packages/modules/ImsMedia/imsmedia.mk)
$(call inherit-product, packages/apps/CarrierSettings/carrier_settings.mk)
PRODUCT_PACKAGES += ImsStack Iwlan QualifiedNetworksService
```

`imsmedia.mk` supplies the media service, native library and its user-installation
allowlist. CarrierSettings supplies its own system-user installation allowlist.
Use the same product platform signing certificate for ImsStack and CarrierSettings.
Remove the old extension path and `use_carrier_config_ext` setting from product
configuration when migrating.

For Android 16 QPR2, also set:

```make
$(call soong_config_set_bool,imsstack_namespace,use_android16_telephony_compat,true)
```

This selects the compatibility emergency-mode monitor while retaining emergency
callback-mode tracking. Keep emergency MMTEL disabled until the product's complete
emergency/fallback path has been validated. Native Android 17 builds leave this
option unset.

Select `com.android.imsstack` in Telephony's `config_ims_mmtel_package`. Include the
paired media policy directory in BoardConfig:

```make
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += packages/modules/ImsMedia/sepolicy/system_ext/private
```

An overlay targeting `com.android.imsstack` can gate optional features during
bring-up:

```xml
<resources>
    <bool name="config_imsstack_emergency_mmtel_feature">false</bool>
    <bool name="config_imsstack_rcs_feature">false</bool>
</resources>
```

Only products whose data HAL cannot report dedicated-bearer QoS should set:

```xml
<bool name="config_imsstack_dedicated_bearer_qos_supported">false</bool>
```

That switch disables precondition/QoS waits consistently and allows voice on the
default bearer. It is a device constraint, not a carrier-name workaround.

## Carrier data and precedence

ImsStack accepts CarrierSettings only when it is an enabled system app or its
update, is signed like ImsStack, and declares data schema version 1. It opens only
assets and does not load executable code. Package changes refresh the active SIM
configurations; replacement's temporary removal is ignored. Missing/untrusted or
incompatible packages contribute no external carrier data.

The Google index selects the first matching named profile, including empty MVNOs.
MNC length is exact. GID1/ICCID prefixes, IMSI wildcards and literal SPN prefixes
retain their source meaning. The importer rejects unknown selector fields.

Later configuration layers take precedence:

1. Built-in defaults, then CarrierSettings AOSP carrier/parent profiles.
2. Android CarrierConfig and optional public asset profiles.
3. Filtered Google default and the selected named profile.
4. Explicit MCC/MNC corrections from CarrierSettings.
5. Product `R.xml.carrier_config_override`, then the framework AP IMS hidden bundle.
6. Existing test overrides and no-SIM cache handling.
7. Final product/media capability constraints.

Imported settings are applied after framework defaults so default-valued keys do
not erase them. CarrierSettings excludes service availability, APNs, emergency,
QNS/IWLAN, vendor policy and unsupported media settings. Service enablement stays
with Android CarrierConfig. EVS remains disabled because the paired open media
implementation has no encoder/decoder. CarrierSettings documents its complete
allowlist, source hashes, versions and regeneration process.

## Downstream behavior

This fork supports Android 16 compatibility, configurable optional features,
Wi-Fi preconditions, idle IWLAN release, initial registration restart
after access changes, VoWiFi bootstrap, NR capability/fallback handling, incoming
PRACK/response policy and early call termination fixes.

It also protects debug broadcasts, validates XML array sizes and SIP stream
framing, orders asynchronous IPsec release, tracks per-socket QoS ownership,
bounds audio QoS retries and rejects stale network callbacks after replacement
or loss. IPsec shutdown drains remaining connectors and rejects late additions.

Telephony's SMSC type-of-number is preserved. Carrier-specific corrections
include O2 DE and Orange PL; Orange response shaping is limited to initial
incoming final responses and handles quoted Contact display names. CarrierSettings
documents the supported import policy and the separately maintained corrections.

## Validation

Build and run in a complete Android tree:

```sh
m CarrierSettings ImsStack ImsMediaService ImsStackJavaTests ImsStackNativeTests
atest ImsStackJavaTests ImsStackNativeTests
```

Build the Android 16 and native Android 17 configurations separately. See
[tests/README.md](tests/README.md) for focused suites and the package-update pass.
Run the carrier importer/validator when updating data and the media tests when
changing media code.

The platform Java suite covers carrier selection, package trust, update events
and fallback behavior. Native tests cover components including SIP framing.
Host checks and successful APK packaging complement these suites; they do not
establish that IMS works with a product's modem, audio HAL or network.

Before release, validate registration, outgoing/incoming calls, SMS, Wi-Fi-only
boot, handover, long-idle recovery and multi-SIM behavior on the target product.
Include the O2 DE and Orange PL cases when those corrections apply. Emergency
calls require the product's controlled emergency/fallback test procedure.
