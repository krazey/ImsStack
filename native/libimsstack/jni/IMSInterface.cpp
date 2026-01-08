/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG     "ImsStackN"
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include <string.h>
#include <stdio.h>
#include <android/file_descriptor_jni.h>
#include <binder/Parcel.h>
#include <nativehelper/JNIHelp.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include "DeviceConfig.h"
#include "INativeThreadMethods.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"

#include "CoreInterfaceFactory.h"
#include "ImsMain.h"
#include "JniObjectId.h"
#include "JniSystem.h"
#include "NativeCommands.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI");

#define JNI_IMS_OK    (0)
#define JNI_IMS_ERROR (-1)

static jclass s_classJniIms;
static jmethodID s_methodSendDataToJava;
static jmethodID s_methodSendDataToJavaForSystem;

static const char* s_szClassJniImsPath = "com/android/imsstack/jni/JniIms";

static JavaVM* s_javaVm = NULL;

static JavaVM* GetJavaVm()
{
    return s_javaVm;
}

static int SendDataToJava(long nNativeObject, const android::Parcel& objParcel)
{
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    IMS_TRACE_D("SendDataToJava: object=%" PFLS_x, nNativeObject, 0, 0);

    if ((s_classJniIms == NULL) || (s_methodSendDataToJava == NULL))
    {
        IMS_TRACE_E(0, "SendDataToJava: Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM* jvm = GetJavaVm();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendDataToJava: AttachCurrentThread fail", 0, 0, 0);
        return 0;
    }

    jbyteArray jData = env->NewByteArray(objParcel.dataSize());
    jbyte* pBuffer = env->GetByteArrayElements(jData, NULL);

    if (pBuffer != NULL)
    {
        memcpy(pBuffer, objParcel.data(), objParcel.dataSize());

        env->ReleaseByteArrayElements(jData, pBuffer, 0);

        env->CallStaticIntMethod(s_classJniIms, s_methodSendDataToJava, jNativeObject, jData);
    }

    env->DeleteLocalRef(jData);

    return 1;
}

static int SendDataToJavaForSystem(
        long nNativeObject, const android::Parcel& parcelIn, android::Parcel& parcelOut, int fd)
{
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    if ((s_classJniIms == NULL) || (s_methodSendDataToJavaForSystem == NULL))
    {
        IMS_TRACE_E(0, "SendDataToJavaForSystem: Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM* jvm = GetJavaVm();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendDataToJavaForSystem: AttachCurrentThread fail", 0, 0, 0);
        return 0;
    }

    jbyteArray jResultData = NULL;
    jbyteArray jData = env->NewByteArray(parcelIn.dataSize());
    jbyte* pBuffer = env->GetByteArrayElements(jData, NULL);

    if (pBuffer != NULL)
    {
        memcpy(pBuffer, parcelIn.data(), parcelIn.dataSize());

        env->ReleaseByteArrayElements(jData, pBuffer, 0);

        jobject fileDescriptor = null;

        if (fd > 0)
        {
            fileDescriptor = AFileDescriptor_create(env);
            AFileDescriptor_setFd(env, fileDescriptor, fd);
        }

        jResultData = (jbyteArray)env->CallStaticObjectMethod(s_classJniIms,
                s_methodSendDataToJavaForSystem, jNativeObject, jData, fileDescriptor);
    }

    env->DeleteLocalRef(jData);

    if (jResultData == NULL)
    {
        IMS_TRACE_I("SendDataToJavaForSystem: Result is null", 0, 0, 0);
        parcelOut.writeInt32(0);
        parcelOut.setDataPosition(0);
        return 1;
    }

    pBuffer = env->GetByteArrayElements(jResultData, NULL);

    if (pBuffer != NULL)
    {
        int nBuffSize = env->GetArrayLength(jResultData);

        if (nBuffSize == 0)
        {
            IMS_TRACE_D("SendDataToJavaForSystem: Result(buffer-length) is zero", 0, 0, 0);
            parcelOut.writeInt32(1);
        }
        else
        {
            parcelOut.setData(reinterpret_cast<const uint8_t*>(pBuffer), nBuffSize);
        }

        env->ReleaseByteArrayElements(jResultData, pBuffer, 0);
    }
    else
    {
        IMS_TRACE_D("SendDataToJavaForSystem: Result(buffer) is null", 0, 0, 0);
        parcelOut.writeInt32(0);
    }

    env->DeleteLocalRef(jResultData);

    parcelOut.setDataPosition(0);

    return 1;
}

static IMS_UINTP GetCommandParam(IN JNIEnv* env, IN jint cmd, IN jbyteArray jData)
{
    if (jData == NULL)
    {
        return 0;
    }

    jbyte* pData = env->GetByteArrayElements(jData, NULL);
    int nDataLen = env->GetArrayLength(jData);

    if (nDataLen == 0)
    {
        env->ReleaseByteArrayElements(jData, pData, 0);
        return 0;
    }

    android::Parcel objData;

    objData.setData(reinterpret_cast<const uint8_t*>(pData), nDataLen);
    objData.setDataPosition(0);

    env->ReleaseByteArrayElements(jData, pData, 0);

    IMS_UINTP pnParam = 0;

    switch (cmd)
    {
        case NativeCommands::CMD_SET_DEVICE_CONFIG:
        {
            __DeviceConfig* pConfig = new __DeviceConfig();

            pConfig->nSupportedSimCount = objData.readInt32();
            pConfig->nActiveSimCount = objData.readInt32();
            pConfig->nImsEmergencyEnabled = objData.readInt32();
            pConfig->nVoLteEnabled = objData.readInt32();
            pConfig->nVtEnabled = objData.readInt32();
            pConfig->nWfcEnabled = objData.readInt32();

            pnParam = reinterpret_cast<IMS_UINTP>(pConfig);
            break;
        }
        default:
            // no-op
            break;
    }

    return pnParam;
}

static void ReleaseCommandParam(IN jint cmd, IN IMS_UINTP pnParam)
{
    if (pnParam == 0)
    {
        return;
    }

    switch (cmd)
    {
        case NativeCommands::CMD_SET_DEVICE_CONFIG:
        {
            __DeviceConfig* pConfig = reinterpret_cast<__DeviceConfig*>(pnParam);
            delete pConfig;
            break;
        }
        default:
            // no-op
            break;
    }
}

static void JniAttachNativeThread(const char* threadName)
{
    IMS_TRACE_D("JniAttachNativeThread: name=%s", threadName, 0, 0);

    JavaVM* jvm = GetJavaVm();

    if (jvm == NULL)
    {
        return;
    }

    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_4;
    args.name = threadName;
    args.group = NULL;

    JNIEnv* env;
    jint result = jvm->AttachCurrentThread(&env, static_cast<void*>(&args));

    if (result != JNI_OK)
    {
        IMS_TRACE_E(0, "JniAttachNativeThread: Attach failed - %s(%d)", __IMS_FUNC__, result, 0);
        return;
    }
}

static void JniDetachNativeThread()
{
    IMS_TRACE_D("JniDetachNativeThread:", 0, 0, 0);

    JavaVM* jvm = GetJavaVm();

    if (jvm == NULL)
    {
        return;
    }

    jint result = jvm->DetachCurrentThread();

    if (result != JNI_OK)
    {
        IMS_TRACE_E(0, "JniDetachNativeThread : Detach failed", 0, 0, 0);
        return;
    }
}

class NativeThreadMethods : public INativeThreadMethods
{
public:
    inline NativeThreadMethods() {}
    ~NativeThreadMethods() override = default;

    NativeThreadMethods(const NativeThreadMethods&) = delete;
    NativeThreadMethods& operator=(const NativeThreadMethods&) = delete;

public:
    inline void AttachNativeThread(IN const IMS_CHAR* pszName) override
    {
        JniAttachNativeThread(pszName);
    }

    inline void DetachNativeThread() override { JniDetachNativeThread(); }
};

static NativeThreadMethods s_objNativeThreadMethods;

static void JniIms_nativeInit(JNIEnv* /*env*/, jobject /*object*/)
{
    IMS_TRACE_I("JniIms_nativeInit", 0, 0, 0);
    ImsMain::Initialize();
    ThreadService::SetNativeThreadMethods(&s_objNativeThreadMethods);
    ImsMain::Start();
}

static void JniIms_nativeDeInit(JNIEnv* /*env*/, jobject /*object*/)
{
    IMS_TRACE_I("JniIms_nativeDeInit", 0, 0, 0);
    ImsMain::Stop();
    ImsMain::Uninitialize();
}

static int JniIms_nativeSendCommand(
        JNIEnv* env, jobject /*object*/, jint cmd, jint slotId, jbyteArray jData)
{
    IMS_UINTP pnParam = GetCommandParam(env, cmd, jData);
    ImsMain::SendCommand(cmd, slotId, pnParam);
    ReleaseCommandParam(cmd, pnParam);
    return JNI_IMS_OK;
}

static jlong JniIms_nativeGetInterface(
        JNIEnv* /*env*/, jobject /*object*/, jint interfaceType, jint slotId)
{
    BaseService* pService = IMS_NULL;

    if (interfaceType == JniObjectId::SYSTEM)
    {
        pService = new JniSystem(SendDataToJavaForSystem);
    }
    else
    {
        pService = CoreInterfaceFactory::GetInterface(interfaceType, SendDataToJava, slotId);
    }

    IMS_TRACE_I("JniIms_nativeGetInterface: interface=%d, object=%p", interfaceType, pService, 0);

    return static_cast<jlong>(reinterpret_cast<long>(pService));
}

static void JniIms_nativeReleaseInterface(JNIEnv* /*env*/, jobject /*object*/, jlong jNativeObject)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

#if defined(__IMS_CLANG__)
    IMS_TRACE_I("JniIms_nativeReleaseInterface: object=%lx", nNativeObject, 0, 0);
#else
    IMS_TRACE_I("JniIms_nativeReleaseInterface: object=%" PFLS_x, nNativeObject, 0, 0);
#endif

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService != IMS_NULL)
    {
        pService->Destroy();
    }
}

static jint JniIms_nativeSendData(
        JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray jData)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    IMS_TRACE_D("JniIms_nativeSendData :: object=%" PFLS_x, nNativeObject, 0, 0);

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService == IMS_NULL)
    {
        IMS_TRACE_D("JniIms_nativeSendData: BaseService is null", 0, 0, 0);
        return JNI_IMS_ERROR;
    }

    jbyte* pBuff = env->GetByteArrayElements(jData, NULL);
    int nBuffSize = env->GetArrayLength(jData);

    android::Parcel parcel;
    parcel.setData(reinterpret_cast<const uint8_t*>(pBuff), nBuffSize);
    parcel.setDataPosition(0);

    int nResult = pService->SendData(parcel);

    env->ReleaseByteArrayElements(jData, pBuff, 0);

    return (nResult == 1) ? JNI_IMS_OK : JNI_IMS_ERROR;
}

static jbyteArray JniIms_nativeSendDataForSystem(
        JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray jData)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);
    android::Parcel parcelOut;
    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService == IMS_NULL)
    {
        IMS_TRACE_D("JniIms_nativeSendDataForSystem: BaseService is null", 0, 0, 0);
    }
    else
    {
        android::Parcel parcel;

        jbyte* pBuff = env->GetByteArrayElements(jData, NULL);
        int nBuffSize = env->GetArrayLength(jData);

        parcel.setData(reinterpret_cast<const uint8_t*>(pBuff), nBuffSize);
        parcel.setDataPosition(0);

        pService->SendData(parcel, parcelOut);

        env->ReleaseByteArrayElements(jData, pBuff, 0);
    }

    if (parcelOut.dataSize() == 0)  // error case
    {
        parcelOut.writeInt32(1);
    }

    jbyteArray byteArray = env->NewByteArray(parcelOut.dataSize());
    env->SetByteArrayRegion(byteArray, 0, parcelOut.dataSize(),
            reinterpret_cast<jbyte*>(const_cast<uint8_t*>(parcelOut.data())));

    return byteArray;
}

// clang-format off
static JNINativeMethod s_jniImsMethods[] = {
        /* name, signature, funcPtr */
        { "nativeInit",              "()V",     (void*)JniIms_nativeInit              },
        { "nativeDeInit",            "()V",     (void*)JniIms_nativeDeInit            },
        { "nativeGetInterface",      "(II)J",   (void*)JniIms_nativeGetInterface      },
        { "nativeReleaseInterface",  "(J)V",    (void*)JniIms_nativeReleaseInterface  },
        { "nativeSendData",          "(J[B)I",  (void*)JniIms_nativeSendData          },
        { "nativeSendDataForSystem", "(J[B)[B", (void*)JniIms_nativeSendDataForSystem },
        { "nativeSendCommand",       "(II[B)I", (void*)JniIms_nativeSendCommand       },
};
// clang-format on

jint IMSInterface_OnLoad(JavaVM* vm, JNIEnv* env)
{
    s_javaVm = vm;

    jclass jclassIms = env->FindClass(s_szClassJniImsPath);

    if (jclassIms == NULL)
    {
        ALOGE("IMSInterface_OnLoad: FindClass failed");
        return -1;
    }

    s_classJniIms = (jclass)env->NewGlobalRef(jclassIms);

    if (jniRegisterNativeMethods(
                env, s_szClassJniImsPath, s_jniImsMethods, NELEM(s_jniImsMethods)) < 0)
    {
        ALOGE("IMSInterface_OnLoad: RegisterNatives failed");
        return -1;
    }

    s_methodSendDataToJava = env->GetStaticMethodID(s_classJniIms, "sendDataToJava", "(J[B)I");
    s_methodSendDataToJavaForSystem = env->GetStaticMethodID(
            s_classJniIms, "sendDataToJavaForSystem", "(J[BLjava/io/FileDescriptor;)[B");

    if ((s_methodSendDataToJava == NULL) || (s_methodSendDataToJavaForSystem == NULL))
    {
        ALOGE("IMSInterface_OnLoad: GetStaticMethodID failed");
        return -1;
    }

    return JNI_VERSION_1_4;
}
