#!/usr/bin/env bash

#
# Copyright (C) 2024 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

##### App specific parameters #####

PACKAGE_NAME='com.android.imsstack'
MODULE_NAME='ImsStack'
MODULE_PATH='packages/modules/ImsStack'

TEST_PACKAGE='com.android.imsstack.tests'
TEST_RUNNER="$TEST_PACKAGE/androidx.test.runner.AndroidJUnitRunner"

##### End app specific parameters #####

REPORT_TYPE='html'
REPORTER_JAR=$ANDROID_HOST_OUT/framework/jacoco-cli.jar
OUTPUT_DIR="$ANDROID_BUILD_TOP/out/coverage/$MODULE_NAME"

echo ""
echo "Generating coverage report"
echo "Output dir: $OUTPUT_DIR"
echo "Report type: $REPORT_TYPE"

adb root
adb wait-for-device

# location on the device to store coverage results, need to be accessible by the app
REMOTE_COVERAGE_OUTPUT_FILE="/storage/self/primary/Download/coverage.ec"

COVERAGE_OUTPUT_FILE="$ANDROID_BUILD_TOP/out/$PACKAGE_NAME.ec"
COVERAGE_CLASS_FILE="$ANDROID_BUILD_TOP/out/soong/.intermediates/$MODULE_PATH/java/ImsStack/android_common/jacoco-report-classes/ImsStack.jar"

echo ""
echo "RUNNING TESTS $TEST_RUNNER"
echo "============================================"
adb shell am instrument -e coverage true -e coverageFile "$REMOTE_COVERAGE_OUTPUT_FILE" -w "$TEST_RUNNER"
echo "============================================"

mkdir -p "$OUTPUT_DIR"

adb pull "$REMOTE_COVERAGE_OUTPUT_FILE" "$COVERAGE_OUTPUT_FILE"

java -jar "$REPORTER_JAR" \
  report "$COVERAGE_OUTPUT_FILE" \
  --$REPORT_TYPE "$OUTPUT_DIR" \
  --classfiles "$COVERAGE_CLASS_FILE" \
  --sourcefiles "$ANDROID_BUILD_TOP/$MODULE_PATH/java/src"


# Echo the file as URI to quickly open the result using ctrl-click in terminal
if [[ $REPORT_TYPE == html ]] ; then
  echo "COVERAGE RESULTS IN:"
  echo "file://$OUTPUT_DIR/index.html"
fi
