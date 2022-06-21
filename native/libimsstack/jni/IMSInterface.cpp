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
#define LOG_TAG "ImsStackN"
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include <string.h>
#include <stdio.h>
#include <android/file_descriptor_jni.h>
#include <binder/Parcel.h>
#include <gui/Surface.h>
#include <nativehelper/JNIHelp.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include "ServiceTrace.h"

#include "CoreInterfaceFactory.h"
#include "IUIMS.h"
#include "ImsMain.h"
#include "JniSystem.h"

using namespace android;

#if defined(ALOGD)
#define IMS_LOGD ALOGD
#elif defined(LOGD)
#define IMS_LOGD LOGD
#else
#warning LOGD macro is not defined
#endif

#if defined(ALOGE)
#define IMS_LOGE ALOGE
#elif defined(LOGE)
#define IMS_LOGE LOGE
#else
#warning LOGE macro is not defined
#endif

__IMS_TRACE_TAG_ADAPT__;

static jclass s_classJniIms;
static jmethodID s_methodSendDataToJava;
static jmethodID s_methodSendDataToJavaEx;

// For system configuration on boot-up
static android::Parcel* s_pParcelForSystemConfigOnBootup = IMS_NULL;
static const char* s_szClassJniImsPath = "com/android/imsstack/jni/JNIIms";

static JavaVM* s_javaVm = NULL;

static JavaVM* GetJavaVm()
{
    return s_javaVm;
}

static JNIEnv* GetJniEnv()
{
    if (s_javaVm == NULL)
    {
        return NULL;
    }

    JNIEnv* env = NULL;

    if (s_javaVm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        return NULL;
    }

    return env;
}

static int SendDataToJava(long nNativeObject, const android::Parcel& objParcel)
{
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    IMS_TRACE_D("SendDataToJava :: object=%" PFLS_d, nNativeObject, 0, 0);

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

    jbyteArray baData = env->NewByteArray(objParcel.dataSize());
    jbyte* pBuffer = env->GetByteArrayElements(baData, NULL);

    if (pBuffer != NULL)
    {
        memcpy(pBuffer, objParcel.data(), objParcel.dataSize());

        env->ReleaseByteArrayElements(baData, pBuffer, 0);

        env->CallStaticIntMethod(s_classJniIms, s_methodSendDataToJava, jNativeObject, baData);
    }

    env->DeleteLocalRef(baData);

    return 1;
}

int SendDataToJavaEx(
        long nNativeObject, const android::Parcel& parcelIn, android::Parcel& parcelOut, int fd)
{
#define MAX_LOG_DISPLAY_COUNT 5

    static IMS_SINT32 nLogDisplayCount = 0;
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    ++nLogDisplayCount;

    if (nLogDisplayCount >= MAX_LOG_DISPLAY_COUNT)
    {
        IMS_TRACE_D("SendDataToJavaEx :: object=%" PFLS_d, nNativeObject, 0, 0);
        nLogDisplayCount = 0;
    }

    if ((s_classJniIms == NULL) || (s_methodSendDataToJavaEx == NULL))
    {
        IMS_TRACE_E(0, "SendDataToJavaEx : Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM* jvm = GetJavaVm();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendDataToJavaEx : AttachCurrentThread fail", 0, 0, 0);
        return 0;
    }

    jbyteArray baResultData = NULL;
    jbyteArray baMessageData = env->NewByteArray(parcelIn.dataSize());
    jbyte* pBuffer = env->GetByteArrayElements(baMessageData, NULL);

    if (pBuffer != NULL)
    {
        memcpy(pBuffer, parcelIn.data(), parcelIn.dataSize());

        env->ReleaseByteArrayElements(baMessageData, pBuffer, 0);

        jobject fileDescriptor = null;

        if (fd > 0)
        {
            fileDescriptor = AFileDescriptor_create(env);
            AFileDescriptor_setFd(env, fileDescriptor, fd);
        }

        baResultData = (jbyteArray)env->CallStaticObjectMethod(s_classJniIms,
                s_methodSendDataToJavaEx, jNativeObject, baMessageData, fileDescriptor);
    }

    env->DeleteLocalRef(baMessageData);

    if (baResultData == NULL)
    {
        IMS_TRACE_I("SendDataToJavaEx :: Result is null", 0, 0, 0);
        parcelOut.writeInt32(0);
        parcelOut.setDataPosition(0);
        return 1;
    }

    pBuffer = env->GetByteArrayElements(baResultData, NULL);

    if (pBuffer != NULL)
    {
        int nBuffSize = env->GetArrayLength(baResultData);

        if (nBuffSize == 0)
        {
            IMS_TRACE_D("SendDataToJavaEx :: Result(buffer-length) is zero", 0, 0, 0);
            parcelOut.writeInt32(1);
        }
        else
        {
            parcelOut.setData((const uint8_t*)pBuffer, nBuffSize);
        }

        env->ReleaseByteArrayElements(baResultData, pBuffer, 0);
    }
    else
    {
        IMS_TRACE_D("SendDataToJavaEx :: Result(buffer) is null", 0, 0, 0);
        parcelOut.writeInt32(0);
    }

    env->DeleteLocalRef(baResultData);

    parcelOut.setDataPosition(0);

    return 1;
}

IMS_BOOL ReadDeviceConfig(IN android::Parcel* pParcel, OUT __DeviceConfig& objConfig)
{
    objConfig.nActiveModemCount = pParcel->readInt32();
    objConfig.nImsEmergencyEnabled = pParcel->readInt32();
    objConfig.nVoLteEnabled = pParcel->readInt32();
    objConfig.nVtEnabled = pParcel->readInt32();
    objConfig.nWfcEnabled = pParcel->readInt32();

    return IMS_TRUE;
}

__SystemConfig* CreateSystemConfig(IN android::Parcel* pParcel, OUT IMS_SINT32& nCount)
{
    nCount = pParcel->readInt32();

    if (nCount <= 0)
    {
        return IMS_NULL;
    }

    __SystemConfig* pSystemConfig =
            reinterpret_cast<__SystemConfig*>(malloc(sizeof(__SystemConfig) * nCount));

    if (pSystemConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    memset(pSystemConfig, 0, sizeof(__SystemConfig) * nCount);

    android::String16 str16;

    for (int i = 0; i < nCount; ++i)
    {
        __SystemConfig* pSc = &pSystemConfig[i];

        pSc->nSlotId = pParcel->readInt32();

        str16 = pParcel->readString16();
        android::String8 strOperator(str16);
        strncpy(pSc->acOperator, strOperator.string(), IMS_SC_SIZE_16);

        str16 = pParcel->readString16();
        android::String8 strCountry(str16);
        strncpy(pSc->acCountry, strCountry.string(), IMS_SC_SIZE_8);

        str16 = pParcel->readString16();
        android::String8 strEnablerType(str16);
        strncpy(pSc->acEnablerType, strEnablerType.string(), IMS_SC_SIZE_16);

        pSc->nExtraInfo = pParcel->readInt32();

        pSc->nFeatures = pParcel->readInt32();
        pSc->nServiceFeatures = pParcel->readInt32();
    }

    return pSystemConfig;
}

void DestroySystemConfig(IN __SystemConfig* pSystemConfig)
{
    if (pSystemConfig != IMS_NULL)
    {
        free(pSystemConfig);
    }
}

__SystemConfig* GetSystemConfigOnBootup(IN int& nCount)
{
    __SystemConfig* pConfig = IMS_NULL;

    nCount = 0;

    if (s_pParcelForSystemConfigOnBootup != IMS_NULL)
    {
        pConfig = CreateSystemConfig(s_pParcelForSystemConfigOnBootup, nCount);
    }

    return pConfig;
}

void SetSystemConfigOnBootup(IN android::Parcel* pParcel)
{
    if (s_pParcelForSystemConfigOnBootup != IMS_NULL)
    {
        delete s_pParcelForSystemConfigOnBootup;
    }

    s_pParcelForSystemConfigOnBootup = pParcel;
}

void JNI_AttachNativeThread(const char* threadName)
{
    IMS_TRACE_D("JNI_AttachNativeThread: name=%s", threadName, 0, 0);

    JavaVM* jvm = GetJavaVm();

    if (jvm == NULL)
    {
        return;
    }

    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_4;
    args.name = (char*)threadName;
    args.group = NULL;

    JNIEnv* env;
    jint result = jvm->AttachCurrentThread(&env, (void*)&args);

    if (result != JNI_OK)
    {
        IMS_TRACE_E(0, "JNI_AttachNativeThread: Attach failed - %s(%d)", __IMS_FUNC__, result, 0);
        return;
    }
}

void JNI_DetachNativeThread()
{
    IMS_TRACE_D("JNI_DetachNativeThread:", 0, 0, 0);

    JavaVM* jvm = GetJavaVm();

    if (jvm == NULL)
    {
        return;
    }

    jint result = jvm->DetachCurrentThread();

    if (result != JNI_OK)
    {
        IMS_TRACE_E(0, "JNI_DetachNativeThread : Detach failed", 0, 0, 0);
        return;
    }
}

static void JNI_Construct(JNIEnv* /*env*/, jobject /*object*/)
{
    // Memory and basic platform's initialization
    ImsMain::Initialize();

    // Configure the system configuration on boot-up
    {
        int nCount = 0;
        __SystemConfig* pConfig = GetSystemConfigOnBootup(nCount);
        ImsMain::SetConfiguration(SystemConfig::EVENT_ON_BOOT, nCount, pConfig);
        SetSystemConfigOnBootup(IMS_NULL);

        if (pConfig != IMS_NULL)
        {
            free(pConfig);
        }
    }

    ImsMain::Start();
}

static void JNI_Destruct(JNIEnv* /*env*/, jobject /*object*/)
{
    IMS_TRACE_I("JNI_Destruct", 0, 0, 0);

    ImsMain::Stop();

    ImsMain::Uninitialize();
}

static int JNI_SetConfiguration(JNIEnv* env, jobject /*object*/, jint event, jbyteArray baData)
{
    if (event == SystemConfig::EVENT_FEATURE_PERMISSIONS_CHANGED)
    {
        ImsMain::SetConfiguration(event, 0, IMS_NULL);
        return 1;
    }

    jbyte* pData = env->GetByteArrayElements(baData, NULL);
    int nDataSize = env->GetArrayLength(baData);

    if (nDataSize == 0)
    {
        env->ReleaseByteArrayElements(baData, pData, 0);
        return 0;
    }

    android::Parcel* pParcel = new android::Parcel();

    pParcel->setData((const uint8_t*)pData, nDataSize);
    pParcel->setDataPosition(0);

    env->ReleaseByteArrayElements(baData, pData, 0);

    // Keep the system configuration and use it to initialize IMS core
    // if the configuration is for boot-up (Initial start of IMS process).
    if (event == SystemConfig::EVENT_ON_BOOT)
    {
        SetSystemConfigOnBootup(pParcel);
    }
    else if (event == SystemConfig::EVENT_DEVICE_CONFIG)
    {
        __DeviceConfig objConfig;

        if (ReadDeviceConfig(pParcel, objConfig))
        {
            ImsMain::SetDeviceConfig(objConfig);
        }

        delete pParcel;
    }
    else
    {
        int nCount = 0;
        __SystemConfig* pSystemConfig = CreateSystemConfig(pParcel, nCount);

        delete pParcel;

        if (nCount == 0)
        {
            return 0;
        }

        ImsMain::SetConfiguration(event, nCount, pSystemConfig);

        DestroySystemConfig(pSystemConfig);
    }

    return 1;
}

static jlong JNI_GetInterface(
        JNIEnv* /*env*/, jobject /*object*/, jint nInterfaceType, jint nSlotId)
{
    BaseService* pService = IMS_NULL;

    if (nInterfaceType == IUIMS::SYSTEM_INTERFACE)
    {
        pService = new JniSystem(SendDataToJavaEx);
    }
    else
    {
        pService = CoreInterfaceFactory::GetInterface(nInterfaceType, SendDataToJava, nSlotId);
    }

    IMS_LOGD("JNI_GetInterface :: interfaceType=%d, object=%" PFLS_d, nInterfaceType,
            reinterpret_cast<IMS_SINTP>(pService));

    return static_cast<jlong>(reinterpret_cast<long>(pService));
}

static jint JNI_ReleaseInterface(JNIEnv* /*env*/, jobject /*object*/, jlong jNativeObject)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

#if defined(__IMS_CLANG__)
    IMS_LOGD("JNI_ReleaseInterface :: object=%ld", nNativeObject);
#else
    IMS_LOGD("JNI_ReleaseInterface :: object=%" PFLS_d, nNativeObject);
#endif

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService != IMS_NULL)
    {
        delete pService;
    }

    return 1;
}

static jint JNI_SendData(JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray baData)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    IMS_TRACE_D("JNI_SendData :: object=%" PFLS_d, nNativeObject, 0, 0);

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService == IMS_NULL)
    {
        IMS_TRACE_D("JNI_SendData: BaseService is null", 0, 0, 0);
        return -1;
    }

    int nResult;
    android::Parcel parcel;

    jbyte* pBuff = env->GetByteArrayElements(baData, NULL);
    int nBuffSize = env->GetArrayLength(baData);

    parcel.setData((const uint8_t*)pBuff, nBuffSize);
    parcel.setDataPosition(0);

    nResult = pService->SendData(parcel);

    env->ReleaseByteArrayElements(baData, pBuff, 0);

    return nResult;
}

jbyteArray JNI_SendDataEx(JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray baData)
{
#define MAX_LOG_DISPLAY_COUNT 5

    static int nLogDisplayCount = 0;
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    nLogDisplayCount++;

    if (nLogDisplayCount >= MAX_LOG_DISPLAY_COUNT)
    {
        IMS_TRACE_I("JNI_SendDataEx :: object=%" PFLS_d, nNativeObject, 0, 0);
        nLogDisplayCount = 0;
    }

    android::Parcel parcelOut;
    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService == IMS_NULL)
    {
        IMS_TRACE_D("JNI_SendDataEx: BaseService is null", 0, 0, 0);
    }
    else
    {
        android::Parcel parcel;

        jbyte* pBuff = env->GetByteArrayElements(baData, NULL);
        int nBuffSize = env->GetArrayLength(baData);

        parcel.setData((const uint8_t*)pBuff, nBuffSize);
        parcel.setDataPosition(0);

        pService->SendData(parcel, parcelOut);

        env->ReleaseByteArrayElements(baData, pBuff, 0);
    }

    if (parcelOut.dataSize() == 0)  // error case
    {
        parcelOut.writeInt32(1);
    }

    jbyteArray byteArray = env->NewByteArray(parcelOut.dataSize());
    env->SetByteArrayRegion(byteArray, 0, parcelOut.dataSize(), (jbyte*)parcelOut.data());

    return byteArray;
}

static JNINativeMethod s_jniMethods[] = {
        {"construct",        "()V",     (void*)JNI_Construct       },
        {"destruct",         "()V",     (void*)JNI_Destruct        },
        {"getInterface",     "(II)J",   (void*)JNI_GetInterface    },
        {"releaseInterface", "(J)I",    (void*)JNI_ReleaseInterface},
        {"sendData",         "(J[B)I",  (void*)JNI_SendData        },
        {"sendDataEx",       "(J[B)[B", (void*)JNI_SendDataEx      },
        {"setConfiguration", "(I[B)I",  (void*)JNI_SetConfiguration},
};

int IMSInterface_GetSurface(
        const String8& str8Class, const String8& str8SurfaceName, long& nSurfaceObject)
{
    JNIEnv* pEnv = GetJniEnv();

    if (pEnv == NULL)
    {
        IMS_TRACE_E(0, "JNIEnv is null", 0, 0, 0);
        return (-1);
    }

    nSurfaceObject = 0;

    // Find Surface member variable in UCMediaSession
    jclass clazz = pEnv->FindClass(str8Class.string());
    jfieldID surfaceFid = (clazz == NULL)
            ? NULL
            : pEnv->GetStaticFieldID(clazz, str8SurfaceName.string(), "Landroid/view/Surface;");
    jobject surfaceObject =
            (surfaceFid == NULL) ? NULL : pEnv->GetStaticObjectField(clazz, surfaceFid);

    if (surfaceObject != NULL)
    {
        // Find a native object from android.view.Surface
        jclass surfaceClass = pEnv->FindClass("android/view/Surface");
        jfieldID surfaceNativeObject = (surfaceClass == NULL)
                ? NULL
                : pEnv->GetFieldID(surfaceClass, "mNativeObject", "J");
        Surface* pSurface = (surfaceNativeObject == NULL)
                ? NULL
                : (Surface*)pEnv->GetLongField(surfaceObject, surfaceNativeObject);

        if (pSurface == NULL)
        {
            nSurfaceObject = 0;
        }
        else
        {
            pSurface->incStrong(0);
            nSurfaceObject = reinterpret_cast<long>(pSurface);
        }
    }

    IMS_TRACE_D("IMSInterface_GetSurface - class=%s, surface=%s(%" PFLS_x ")",
            str8Class.getPathLeaf().string(), str8SurfaceName.string(), nSurfaceObject);

    return 1;
}

void IMSInterface_ReleaseSurface(long nSurface)
{
    if (nSurface == 0)
    {
        return;
    }

    Surface* pSurface = reinterpret_cast<Surface*>(nSurface);

    if (pSurface != NULL)
    {
        IMS_TRACE_D(
                "IMSInterface_ReleaseSurface - strongCount=%d", pSurface->getStrongCount(), 0, 0);

        pSurface->decStrong(0);
    }
}

jint IMSInterface_OnLoad(JavaVM* vm, JNIEnv* env)
{
    s_javaVm = vm;

    jclass jclassIms = env->FindClass(s_szClassJniImsPath);

    if (jclassIms == NULL)
    {
        IMS_LOGE("IMSInterface_OnLoad: FindClass failed");
        return -1;
    }

    s_classJniIms = (jclass)env->NewGlobalRef(jclassIms);

    if (jniRegisterNativeMethods(env, s_szClassJniImsPath, s_jniMethods, NELEM(s_jniMethods)) < 0)
    {
        IMS_LOGE("IMSInterface_OnLoad: RegisterNatives failed");
        return -1;
    }

    s_methodSendDataToJava = env->GetStaticMethodID(s_classJniIms, "sendData2Java", "(J[B)I");
    s_methodSendDataToJavaEx = env->GetStaticMethodID(
            s_classJniIms, "sendData2JavaEx", "(J[BLjava/io/FileDescriptor;)[B");

    if ((s_methodSendDataToJava == NULL) || (s_methodSendDataToJavaEx == NULL))
    {
        IMS_LOGE("IMSInterface_OnLoad: GetStaticMethodID failed");
        return -1;
    }

    return JNI_VERSION_1_4;
}
