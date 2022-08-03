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

#include "ServiceThread.h"
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

#define JNI_IMS_OK    (0)
#define JNI_IMS_ERROR (-1)

static jclass s_classJniIms;
static jmethodID s_methodSendDataToJava;
static jmethodID s_methodSendDataToJavaForSystem;

// For system configuration on boot-up
static android::Parcel* s_pParcelForSystemConfigOnBootup = IMS_NULL;
static const char* s_szClassJniImsPath = "com/android/imsstack/jni/JniIms";

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

int SendDataToJavaForSystem(
        long nNativeObject, const android::Parcel& parcelIn, android::Parcel& parcelOut, int fd)
{
#define MAX_LOG_DISPLAY_COUNT 5

    static IMS_SINT32 nLogDisplayCount = 0;
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    ++nLogDisplayCount;

    if (nLogDisplayCount >= MAX_LOG_DISPLAY_COUNT)
    {
        IMS_TRACE_D("SendDataToJavaForSystem :: object=%" PFLS_d, nNativeObject, 0, 0);
        nLogDisplayCount = 0;
    }

    if ((s_classJniIms == NULL) || (s_methodSendDataToJavaForSystem == NULL))
    {
        IMS_TRACE_E(0, "SendDataToJavaForSystem : Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM* jvm = GetJavaVm();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendDataToJavaForSystem : AttachCurrentThread fail", 0, 0, 0);
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
        IMS_TRACE_I("SendDataToJavaForSystem :: Result is null", 0, 0, 0);
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
            IMS_TRACE_D("SendDataToJavaForSystem :: Result(buffer-length) is zero", 0, 0, 0);
            parcelOut.writeInt32(1);
        }
        else
        {
            parcelOut.setData((const uint8_t*)pBuffer, nBuffSize);
        }

        env->ReleaseByteArrayElements(jResultData, pBuffer, 0);
    }
    else
    {
        IMS_TRACE_D("SendDataToJavaForSystem :: Result(buffer) is null", 0, 0, 0);
        parcelOut.writeInt32(0);
    }

    env->DeleteLocalRef(jResultData);

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

void JniAttachNativeThread(const char* threadName)
{
    IMS_TRACE_D("JniAttachNativeThread: name=%s", threadName, 0, 0);

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
        IMS_TRACE_E(0, "JniAttachNativeThread: Attach failed - %s(%d)", __IMS_FUNC__, result, 0);
        return;
    }
}

void JniDetachNativeThread()
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
    inline virtual ~NativeThreadMethods() {}

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
    // Memory and basic platform's initialization
    ImsMain::Initialize();
    ThreadService::SetNativeThreadMethods(&s_objNativeThreadMethods);

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

static void JniIms_nativeDeInit(JNIEnv* /*env*/, jobject /*object*/)
{
    IMS_TRACE_I("JniIms_nativeDeInit", 0, 0, 0);

    ImsMain::Stop();

    ImsMain::Uninitialize();
}

static int JniIms_nativeSendCommand(
        JNIEnv* env, jobject /*object*/, jint cmd, jint slotId, jbyteArray jData)
{
    (void)slotId;

    if (cmd == SystemConfig::EVENT_FEATURE_PERMISSIONS_CHANGED)
    {
        ImsMain::SetConfiguration(cmd, 0, IMS_NULL);
        return JNI_IMS_OK;
    }

    jbyte* pData = env->GetByteArrayElements(jData, NULL);
    int nDataSize = env->GetArrayLength(jData);

    if (nDataSize == 0)
    {
        env->ReleaseByteArrayElements(jData, pData, 0);
        return JNI_IMS_ERROR;
    }

    android::Parcel* pParcel = new android::Parcel();

    pParcel->setData((const uint8_t*)pData, nDataSize);
    pParcel->setDataPosition(0);

    env->ReleaseByteArrayElements(jData, pData, 0);

    // Keep the system configuration and use it to initialize IMS core
    // if the configuration is for boot-up (Initial start of IMS process).
    if (cmd == SystemConfig::EVENT_ON_BOOT)
    {
        SetSystemConfigOnBootup(pParcel);
    }
    else if (cmd == SystemConfig::EVENT_DEVICE_CONFIG)
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
            return JNI_IMS_ERROR;
        }

        ImsMain::SetConfiguration(cmd, nCount, pSystemConfig);

        DestroySystemConfig(pSystemConfig);
    }

    return JNI_IMS_OK;
}

static jlong JniIms_nativeGetInterface(
        JNIEnv* /*env*/, jobject /*object*/, jint interfaceType, jint slotId)
{
    BaseService* pService = IMS_NULL;

    if (interfaceType == IUIMS::SYSTEM_INTERFACE)
    {
        pService = new JniSystem(SendDataToJavaForSystem);
    }
    else
    {
        pService = CoreInterfaceFactory::GetInterface(interfaceType, SendDataToJava, slotId);
    }

    IMS_LOGD("JniIms_nativeGetInterface :: interface=%d, object=%" PFLS_d, interfaceType,
            reinterpret_cast<IMS_SINTP>(pService));

    return static_cast<jlong>(reinterpret_cast<long>(pService));
}

static void JniIms_nativeReleaseInterface(JNIEnv* /*env*/, jobject /*object*/, jlong jNativeObject)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

#if defined(__IMS_CLANG__)
    IMS_LOGD("JniIms_nativeReleaseInterface :: object=%ld", nNativeObject);
#else
    IMS_LOGD("JniIms_nativeReleaseInterface :: object=%" PFLS_d, nNativeObject);
#endif

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService != IMS_NULL)
    {
        delete pService;
    }
}

static jint JniIms_nativeSendData(
        JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray jData)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    IMS_TRACE_D("JniIms_nativeSendData :: object=%" PFLS_d, nNativeObject, 0, 0);

    BaseService* pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService == IMS_NULL)
    {
        IMS_TRACE_D("JniIms_nativeSendData: BaseService is null", 0, 0, 0);
        return JNI_IMS_ERROR;
    }

    jbyte* pBuff = env->GetByteArrayElements(jData, NULL);
    int nBuffSize = env->GetArrayLength(jData);

    android::Parcel parcel;
    parcel.setData((const uint8_t*)pBuff, nBuffSize);
    parcel.setDataPosition(0);

    int nResult = pService->SendData(parcel);

    env->ReleaseByteArrayElements(jData, pBuff, 0);

    return (nResult == 1) ? JNI_IMS_OK : JNI_IMS_ERROR;
}

jbyteArray JniIms_nativeSendDataForSystem(
        JNIEnv* env, jobject /*object*/, jlong jNativeObject, jbyteArray jData)
{
#define MAX_LOG_DISPLAY_COUNT 5

    static int nLogDisplayCount = 0;
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    nLogDisplayCount++;

    if (nLogDisplayCount >= MAX_LOG_DISPLAY_COUNT)
    {
        IMS_TRACE_I("JniIms_nativeSendDataForSystem :: object=%" PFLS_d, nNativeObject, 0, 0);
        nLogDisplayCount = 0;
    }

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

        parcel.setData((const uint8_t*)pBuff, nBuffSize);
        parcel.setDataPosition(0);

        pService->SendData(parcel, parcelOut);

        env->ReleaseByteArrayElements(jData, pBuff, 0);
    }

    if (parcelOut.dataSize() == 0)  // error case
    {
        parcelOut.writeInt32(1);
    }

    jbyteArray byteArray = env->NewByteArray(parcelOut.dataSize());
    env->SetByteArrayRegion(byteArray, 0, parcelOut.dataSize(), (jbyte*)parcelOut.data());

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

    if (jniRegisterNativeMethods(
                env, s_szClassJniImsPath, s_jniImsMethods, NELEM(s_jniImsMethods)) < 0)
    {
        IMS_LOGE("IMSInterface_OnLoad: RegisterNatives failed");
        return -1;
    }

    s_methodSendDataToJava = env->GetStaticMethodID(s_classJniIms, "sendDataToJava", "(J[B)I");
    s_methodSendDataToJavaForSystem = env->GetStaticMethodID(
            s_classJniIms, "sendDataToJavaForSystem", "(J[BLjava/io/FileDescriptor;)[B");

    if ((s_methodSendDataToJava == NULL) || (s_methodSendDataToJavaForSystem == NULL))
    {
        IMS_LOGE("IMSInterface_OnLoad: GetStaticMethodID failed");
        return -1;
    }

    return JNI_VERSION_1_4;
}
