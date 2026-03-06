function coverageon() {
    IMS_STACK_PATH=packages/modules/ImsStack

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
    IMS_STACK_PATH=packages/modules/ImsStack
    TEST_MODULE_PATH=$IMS_STACK_PATH/tests/native

    COVERAGE_OUTPUT_DEVICE_PATH=/data/misc/trace
    COVERAGE_OUTPUT_PATH=coverage

    adb root
    adb shell rm "$COVERAGE_OUTPUT_DEVICE_PATH/*"
    atest $(get_test_name "$@") -- --log-level-display ERROR
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
    atest $command -- --log-level-display ERROR
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

function its() {
    if [ $# -eq 0 ]; then
        get_its_test_name "$@"
        return 1
    fi

    local command=$(get_its_test_name "$1")
    shift
    local extra_args="$@"

    clear
    echo "> atest -c ImsStackTest:$command $extra_args"
    atest -c ImsStackTest:$command $extra_args -- --log-level-display ERROR
}

function get_its_test_name() {
    if [ $# -eq 0 ]; then
        echo "Usage: its <TestPath.TestName> or its <TestPath.TestName#TestMethod> [--argument]"
        exit 1
    fi

    local test_arg="$1"
    local test_class_name
    local test_method

    if [[ "$test_arg" == *"#"* ]]; then
        test_class_name="${test_arg%%#*}"
        test_method="#${test_arg#*#}"
    else
        test_class_name="$test_arg"
        test_method=""
    fi

    local search_dir=""
    if [[ "$test_class_name" == *"."* ]]; then
        search_dir="/$(echo "${test_class_name%.*}" | sed 's/\./\//g')"
        test_class_name="${test_class_name##*.}"
    fi

    local script_dir=$(dirname "${BASH_SOURCE[0]}")
    local search_path="${script_dir}/ImsStackTest/java${search_dir}"
    local file_path=$(find "${search_path}" -name "${test_class_name}.java" | head -n 1)

    if [ -z "$file_path" ]; then
        echo "Test file not found for: ${test_arg}"
        exit 1
    fi

    local full_class_name=$(echo "$file_path" | sed -e 's|.*java/src/||' -e 's|.java||' -e 's|/|.|g')

    TEST_NAME="${full_class_name}${test_method}"

    echo "$TEST_NAME"
}
