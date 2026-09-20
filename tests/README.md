# Testing ImsStack

Use a complete Android source tree with the paired ImsMedia and CarrierSettings
repositories and a configured product. Connect a supported device or Cuttlefish
instance before running instrumentation/native tests.

## Main suites

```sh
m ImsStack ImsStackJavaTests ImsStackNativeTests
atest ImsStackJavaTests ImsStackNativeTests
```

`ImsStackJavaTests` covers Java agents and service integration.
`ImsStackNativeTests` is the aggregate native suite. Focus a run with:

```sh
atest ImsStackJavaTests:ConfigAgentTest
atest ImsStackJavaTests:CarrierSettingsPackageTest
atest ImsStackNativeTests:SipMessageFramingTest
```

After adding modules or tests, use `--rebuild-module-info` if the local atest
module index is stale. Build/run Android 16 QPR2 with the compatibility option and
Android 17 without it as separate configurations. The Android 16 build excludes
the Android 17 domain-selection emergency monitor/tests by design.

## Carrier package regression pass

The automated Java cases cover ordered SIM selection, exact MNC lengths, empty
MVNOs, ICCID prefixes, profile-path rejection, precedence and hardware constraints.
`CarrierSettingsPackageTest` covers system-app status, signatures, metadata,
missing and disabled packages. Package-event tests cover refreshed assets,
removal fallback, replacement events, unrelated packages and agent cleanup.

On a product image that preinstalls CarrierSettings:

1. Verify the selected profile and effective values for the test SIM.
2. Install a higher-version CarrierSettings APK signed with the same product key.
   Confirm values refresh without rebuilding/restarting ImsStack.
3. Verify a temporary replacement removal does not clear policy mid-update.
4. Disable/remove the package in the controlled test image and confirm external
   values disappear while framework/product policy remains.
5. Restore the package and repeat with multiple SIMs and after reboot.

A normally installed user APK does not satisfy the loader's system-app check.
Do not change live carrier policy without a controlled product/device test setup.
The independent importer and XML validator live in CarrierSettings.

## Media and lifecycle regression pass

Exercise registration recovery, LTE/IWLAN and NR changes, outgoing/incoming calls,
early local hangup, MO/MT SMS, AMR-NB/WB, Wi-Fi-only boot, idle recovery and SIM
replacement. Cover IPsec shutdown, QoS callback replacement and stale callbacks
after network loss. Retest the retained O2 DE and Orange PL cases explicitly.

Emergency testing follows the product's existing controlled fallback procedure.
Host parsing, sanitizer tests and successful packaging do not establish working
IMS on a modem or validate emergency behavior.
