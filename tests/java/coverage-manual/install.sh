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

if [ -z $TARGET_ARCH ]; then
  TARGET_ARCH='arm64'
fi
TEST_PACKAGE='com.android.imsstack.tests'
TEST_MODULE_NAME='ImsStackJavaTests'
TEST_MODULE_PATH='packages/modules/ImsStack/tests/java'
TEST_MODULE_INSTALL_PATH="testcases/$TEST_MODULE_NAME/$TARGET_ARCH/$TEST_MODULE_NAME.apk"

##### End app specific parameters #####

# location on the device to store coverage results, need to be accessible by the app
REMOTE_COVERAGE_OUTPUT_FILE="/storage/self/primary/Download/coverage.ec"

if [ -z $ANDROID_BUILD_TOP ]; then
  echo "You need to source and lunch before you can use this script"
  exit 1
fi

source $ANDROID_BUILD_TOP/build/envsetup.sh

set -e # fail early

echo ""
echo "BUILDING TEST PACKAGE $TEST_PACKAGE"
echo "============================================"
(cd "$ANDROID_BUILD_TOP/$TEST_MODULE_PATH" && EMMA_INSTRUMENT=true EMMA_INSTRUMENT_STATIC=true mma -j32)
echo "============================================"

adb root
adb wait-for-device

adb shell rm -f "$REMOTE_COVERAGE_OUTPUT_FILE"

adb install -r -g "$OUT/$TEST_MODULE_INSTALL_PATH"

echo "Rebooting..."
adb reboot
adb wait-for-device
