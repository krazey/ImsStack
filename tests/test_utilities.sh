function coverageon() {
    IMS_STACK_PATH=vendor/google/services/ImsStack

    export CLANG_COVERAGE=true
    export NATIVE_COVERAGE_PATHS="$IMS_STACK_PATH/native/libimsstack $IMS_STACK_PATH/tests/native/libimsstack"
}

function coveragegen() {
    if [ -z $ANDROID_BUILD_TOP ]; then
        echo "You need to source and lunch before you can use this script"
        return 1
    fi

    cd $ANDROID_BUILD_TOP

    IMS_STACK_BIN_PATH=$OUT/symbols/system_ext/lib64/libimsstack.so
    if [ ! -e $IMS_STACK_BIN_PATH ]; then
        IMS_STACK_BIN_PATH=$OUT/symbols/system_ext/lib/libimsstack.so
    fi
    IMS_STACK_PATH=vendor/google/services/ImsStack
    TEST_MODULE_PATH=$IMS_STACK_PATH/tests/native

    COVERAGE_OUTPUT_DEVICE_PATH=/data/misc/trace
    COVERAGE_OUTPUT_PATH=coverage

    adb root
    adb shell rm "$COVERAGE_OUTPUT_DEVICE_PATH/*"
    atest $(get_test_name "$@")
    atest --latest-result

    rm -rf $COVERAGE_OUTPUT_PATH
    adb pull $COVERAGE_OUTPUT_DEVICE_PATH $COVERAGE_OUTPUT_PATH

    llvm-profdata merge --sparse -o $COVERAGE_OUTPUT_PATH/coverage.profdata $COVERAGE_OUTPUT_PATH/*.profraw
    llvm-cov show -format=html -output-dir=$COVERAGE_OUTPUT_PATH -instr-profile $COVERAGE_OUTPUT_PATH/coverage.profdata $IMS_STACK_BIN_PATH
}

function imstest() {
    command=$(get_test_name "$@")

    clear
    echo "> atest $command"
    atest $command
}

function imstestf() {
    command=$(get_test_name "$@")

    clear
    echo "> atest $command"
    atest $command | egrep -A 2 '( Failure|Expected: |Actual: |, Failed: |: FAILED|RUNNER ERROR:|Which is:)'
}

function get_test_name() {
    if [ $# -eq 0 ]; then
        TEST_NAME=ImsStackNativeTests
    elif [ $# -eq 1 ]; then
        if [[ $1 = mtc ]]; then
            TEST_NAME=ImsStackNativeEnablerMtcTests
        elif [[ $1 = mts ]]; then
            TEST_NAME=ImsStackNativeEnablerMtsTests
        elif [[ $1 = media ]]; then
            TEST_NAME=ImsStackNativeEnablerMediaTests
        elif [[ $1 = aos ]]; then
            TEST_NAME=ImsStackNativeEnablerAosTests
        elif [[ $1 = engine ]]; then
            TEST_NAME=ImsStackNativeEngineTests
        elif [[ $1 = platform ]]; then
            TEST_NAME=ImsStackNativePlatformTests
        elif [[ $1 = protocol ]]; then
            TEST_NAME=ImsStackNativeProtocolTests
        elif [[ $1 = enabler ]]; then
            TEST_NAME=ImsStackNativeEnablerTests
        elif [[ $1 = *"#"* ]]; then
            TEST_NAME=ImsStackNativeTests:$1
        else
            TEST_NAME=$(find . -name $1.cpp)
        fi
    elif [ $# -eq 2 ]; then
        if [[ $1 == *, ]]; then
            TEST_NAME=ImsStackNativeTests:${1:0:${#1}-1}#$2
        else
            TEST_NAME=ImsStackNativeTests:$1#$2
        fi
    fi

    echo "$TEST_NAME"
}
