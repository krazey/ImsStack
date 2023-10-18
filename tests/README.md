# ImsStack UnitTests Suite

## 1. Introduction
- The `tests/` directory in _ImsStack_ contains all the _ImsStack_'s Unit Tests.
- As _ImsStack_ consists of both java and native code,
  Unit Tests are divided into two major testmodules.
    * `ImsStackJavaTests`
    * `ImsStackNativeTests`
- A testgroup is a collection of multiple testmodules. _ImsStack_ has three testgroups:
    * `ImsStack-alltests`  - _consists two major testmodules explained above._
    * `ImsStack-javatests` - _consists all junit testmodules._
    * `ImsStack-nativetests` - _consists all gtest testmodules._
- Testmodules are further layered for convenience of test run as mentioned below:
    * `ImsStackJavaTests`
    * `ImsStackNativeTests`
        * `ImsStackNativeConfigTests`
        * `ImsStackNativeEnablerTests`
            * `ImsStackNativeEnablerAosTests`
            * `ImsStackNativeEnablerMediaTests`
            * `ImsStackNativeEnablerMtcTests`
            * `ImsStackNativeEnablerMtsTests`
        * `ImsStackNativeEngineTests`
        * `ImsStackNativePlatformTests`
        * `ImsStackNativeProtocolTests`
            * `ImsStackNativeProtocolSipTests`
        * `ImsStackNativeJniTests`
        * `ImsStackNativeMainTests`

## 2. Procedure to run tests with atest

Create and connect Cuttlefish before executing atest commands.

```
gcert
acloud create --local-image
acloud reconnect
```
#### 2.1 To run all tests in ImsStack
```
atest -c --rebuild-module-info
```

#### 2.2 To run all tests in a testgroup
A testgroup is a collection of multiple testmodules.

```
atest --test-mapping :<TESTGROUP>
```
Example: To run all testmodules in `ImsStack-nativetests` testgroup.
```
atest --test-mapping :ImsStack-nativetests
```
Example: To run all testmodules in `ImsStack-javatests` testgroup.
```
atest --test-mapping :ImsStack-javatests
```

#### 2.3 To run all tests in a testmodule
```
atest <TESTMODULE>
```
Example: To run all tests in `ImsStackNativeTests` testmodule
```
atest ImsStackNativeTests
```

#### 2.4 To run all tests in a test class
```
atest <TESTMODULE>:<TESTCLASS>
```
Example: To run all tests in _SipViaHeaderTest_ test class in `ImsStackNativeTests` testmodule
```
atest ImsStackNativeTests:SipViaHeaderTest
```

## 2.5 To build and run tests if new test is added

- Add `--rebuild-module-info` as last argument for any atest command to build and run.
- Add `-c` as first argument for any atest command to clean old cache.

Example: To build and run a test module along with clear cache:
```
atest -c <TESTMODULE> --rebuild-module-info
```
Example: To build and run `ImsStackNativeTests` testmodule if new test is added
```
atest -c ImsStackNativeTests --rebuild-module-info
```


## 3. Procedure to run tests without atest

After building _ImsStack_ with `mm` command, connect Cuttlefish.

#### 3.1 Sync tests to Cuttlefish/device
```
$ adb root
$ adb remount
$ adb sync
```
#### 3.2.1 Run native tests on Cuttlefish/device
```
adb shell ./data/nativetest64/<TESTMODULE>/<TESTMODULE>
```
Example: To run `ImsStackNativeTests` testmodule on CVD/device
```
adb shell ./data/nativetest64/ImsStackNativeTests/ImsStackNativeTests
```
#### 3.2.2 Run Java tests on Cuttlefish/device
Testapk should be installed on device
```
adb shell am instrument -w <test_package_name>/<runner_class>
```
Example: To run `ImsStackJavaTests` testmodule on CVD/device
```
adb shell am instrument -w com.android.imsstack.tests.java.imsapp/androidx.test.runner.AndroidJUnitRunner
```

## 4. Folder Paths of Testmodules

| TESTMODULE | TESTCASES PATH |
| ------ | ------ |
|ImsStackJavaTests|/java/|
|ImsStackNativeTests|/native/libimsstack/|
    |ImsStackNativeConfigTests|/native/libimsstack/config/|
    |ImsStackNativeEnablerTests|/native/libimsstack/enabler/|
        |ImsStackNativeEnablerAosTests|/native/libimsstack/enabler/aos/|
        |ImsStackNativeEnablerMediaTests|/native/libimsstack/enabler/media/|
        |ImsStackNativeEnablerMtcTests|/native/libimsstack/enabler/mtc/|
        |ImsStackNativeEnablerMtsTests|/native/libimsstack/enabler/mts/|
    |ImsStackNativeEngineTests|/native/libimsstack/engine/|
    |ImsStackNativeJniTests|/native/libimsstack/jni/|
    |ImsStackNativeMainTests|/native/libimsstack/main/|
    |ImsStackNativePlatformTests|/native/libimsstack/platform/|
    |ImsStackNativeProtocolTests|/native/libimsstack/protocol/|
        |ImsStackNativeEnablerMtsTests|/native/libimsstack/protocol/sip/|
