/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20131211  joonhun.shin@             Created
    </table>

    Description
        This file defines the entry point of JNI method load.
*/

#define LOG_TAG "ImsStackN"
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include <string.h>
#include <stdio.h>
#include <android/file_descriptor_jni.h>
#include <binder/Parcel.h>
// Google_IMS_IF :: VIDEO_CALL_PROVIDER {
#include <gui/Surface.h>
// Google_IMS_IF :: VIDEO_CALL_PROVIDER }
#include <utils/Log.h>
#include <utils/threads.h>
#include <nativehelper/JNIHelp.h>

#define IMS_STL_USE

#include "ServiceTrace.h"
#include "IUIMS.h"
#include "JniSystem.h"

#include "CoreInterfaceFactory.h"
#include "ImsMain.h"

using namespace android;

#if defined(ALOGD)
#define IMS_LOGD            ALOGD
#elif defined(LOGD)
#define IMS_LOGD            LOGD
#else
#warning LOGD macro is not defined
#endif

#if defined(ALOGE)
#define IMS_LOGE            ALOGE
#elif defined(LOGE)
#define IMS_LOGE            LOGE
#else
#warning LOGE macro is not defined
#endif

__IMS_TRACE_TAG_ADAPT__;

static jclass gClass_JNIIms;
static jmethodID gMethod_sendData2Java;
static jmethodID gMethod_sendData2JavaEx;

// For system configuration on boot-up
static android::Parcel *gpParcelForSystemConfigOnBootup = IMS_NULL;
static const char *gClassIMSPathName = "com/android/imsstack/jni/JNIIms";

static JavaVM *gJVM = NULL;

static JavaVM* GetJavaVM()
{
    return gJVM;
}

static JNIEnv* GetJNIEnv()
{
    if (gJVM == NULL)
    {
        return NULL;
    }

    JNIEnv* env = NULL;

    if (gJVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        return NULL;
    }

    return env;
}

static int SendData2Java(long nNativeObject, const android::Parcel &objParcel)
{
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    IMS_TRACE_D("SendData2Java :: object=%" PFLS_d, nNativeObject, 0, 0);

    if ((gClass_JNIIms == NULL) || (gMethod_sendData2Java == NULL))
    {
        IMS_TRACE_E(0, "SendData2Java: Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM *jvm = GetJavaVM();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendData2Java: AttachCurrentThread fail", 0, 0, 0);
        return 0;
    }

    jbyteArray baData = env->NewByteArray(objParcel.dataSize());
    jbyte *pBuffer = env->GetByteArrayElements(baData, NULL);

    if (pBuffer != NULL)
    {
        memcpy(pBuffer, objParcel.data(), objParcel.dataSize());

        env->ReleaseByteArrayElements(baData, pBuffer, 0);

        env->CallStaticIntMethod(gClass_JNIIms, gMethod_sendData2Java, jNativeObject, baData);
    }

    env->DeleteLocalRef(baData);

    return 1;
}

int SendData2JavaEx(long nNativeObject,
        const android::Parcel &parcelIn, android::Parcel &parcelOut, int fd)
{
#define MAX_LOG_DISPLAY_COUNT 5

    static IMS_SINT32 nLogDisplayCount = 0;
    JNIEnv* env;
    jlong jNativeObject = nNativeObject;

    ++nLogDisplayCount;

    if (nLogDisplayCount >= MAX_LOG_DISPLAY_COUNT)
    {
        IMS_TRACE_D("SendData2JavaEx :: object=%" PFLS_d, nNativeObject, 0, 0);
        nLogDisplayCount = 0;
    }

    if ((gClass_JNIIms == NULL) || (gMethod_sendData2JavaEx == NULL))
    {
        IMS_TRACE_E(0, "SendData2JavaEx : Method is null", 0, 0, 0);
        return 0;
    }

    JavaVM *jvm = GetJavaVM();

    if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
    {
        IMS_TRACE_E(0, "SendData2JavaEx : AttachCurrentThread fail", 0, 0, 0);
        return 0;
    }

    jbyteArray baResultData = NULL;
    jbyteArray baMessageData = env->NewByteArray(parcelIn.dataSize());
    jbyte *pBuffer = env->GetByteArrayElements(baMessageData, NULL);

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

        baResultData = (jbyteArray)env->CallStaticObjectMethod(gClass_JNIIms,
                gMethod_sendData2JavaEx, jNativeObject, baMessageData, fileDescriptor);
    }

    env->DeleteLocalRef(baMessageData);

    if (baResultData == NULL)
    {
        IMS_TRACE_I("SendData2JavaEx :: Result is null", 0, 0, 0);
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
            IMS_TRACE_D("SendData2JavaEx :: Result(buffer-length) is zero", 0, 0, 0);
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
        IMS_TRACE_D("SendData2JavaEx :: Result(buffer) is null", 0, 0, 0);
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

    __SystemConfig *pstSystemConfig = reinterpret_cast<__SystemConfig*>(
            malloc(sizeof(__SystemConfig) * nCount));

    if (pstSystemConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    memset(pstSystemConfig, 0, sizeof(__SystemConfig) * nCount);

    android::String16 str16;

    for (int i = 0; i < nCount; ++i)
    {
        __SystemConfig *pstSC = &pstSystemConfig[i];

        pstSC->nSlotId = pParcel->readInt32();

        str16 = pParcel->readString16();
        android::String8 strOperator(str16);
        strncpy(pstSC->acOperator, strOperator.string(), IMS_SC_SIZE_16);

        str16 = pParcel->readString16();
        android::String8 strCountry(str16);
        strncpy(pstSC->acCountry, strCountry.string(), IMS_SC_SIZE_8);

        str16 = pParcel->readString16();
        android::String8 strEnablerType(str16);
        strncpy(pstSC->acEnablerType, strEnablerType.string(), IMS_SC_SIZE_16);

        pstSC->nExtraInfo = pParcel->readInt32();

        pstSC->nFeatures = pParcel->readInt32();
        pstSC->nServiceFeatures = pParcel->readInt32();
    }

    return pstSystemConfig;
}

void DestroySystemConfig(IN __SystemConfig* pstSystemConfig)
{
    if (pstSystemConfig != IMS_NULL)
    {
        free(pstSystemConfig);
    }
}

__SystemConfig* GetSystemConfigOnBootup(IN int& nCount)
{
    __SystemConfig* pstConfig = IMS_NULL;

    nCount = 0;

    if (gpParcelForSystemConfigOnBootup != IMS_NULL)
    {
        pstConfig = CreateSystemConfig(gpParcelForSystemConfigOnBootup, nCount);
    }

    return pstConfig;
}

void SetSystemConfigOnBootup(IN android::Parcel* pParcel)
{
    if (gpParcelForSystemConfigOnBootup != IMS_NULL)
    {
        delete gpParcelForSystemConfigOnBootup;
    }

    gpParcelForSystemConfigOnBootup = pParcel;
}

void JNI_AttachNativeThread(const char* threadName)
{
    IMS_TRACE_D("JNI_AttachNativeThread: name=%s", threadName, 0, 0);

    JavaVM *jvm = GetJavaVM();

    if (jvm == NULL)
    {
        return;
    }

    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_4;
    args.name = (char*) threadName;
    args.group = NULL;

    JNIEnv* env;
    jint result = jvm->AttachCurrentThread(&env, (void*) &args);

    if (result != JNI_OK)
    {
        IMS_TRACE_E(0, "JNI_AttachNativeThread: Attach failed - %s(%d)",
                __IMS_FUNC__, result, 0);
        return;
    }
}

void JNI_DetachNativeThread(void)
{
    IMS_TRACE_D("JNI_DetachNativeThread:", 0, 0, 0);

    JavaVM *jvm = GetJavaVM();

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

static void JNI_Construct(JNIEnv* /* env */, jobject /* object */)
{
    // Memory and basic platform's initialization
    ImsMain::Initialize();

    // Configure the system configuration on boot-up
    {
        int nCount = 0;
        __SystemConfig* pstConfig = GetSystemConfigOnBootup(nCount);
        ImsMain::SetConfiguration(SystemConfig::EVENT_ON_BOOT, nCount, pstConfig);
        SetSystemConfigOnBootup(IMS_NULL);

        if (pstConfig != IMS_NULL)
        {
            free(pstConfig);
        }
    }

    ImsMain::Start();
}

static void JNI_Destruct(JNIEnv* /* env */, jobject /* object */)
{
    IMS_TRACE_I("JNI_Destruct", 0, 0, 0);

    ImsMain::Stop();

    ImsMain::Uninitialize();
}

static int JNI_SetConfiguration(JNIEnv *env, jobject /* object */,
        jint event, jbyteArray baData)
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
        __SystemConfig* pstSystemConfig = CreateSystemConfig(pParcel, nCount);

        delete pParcel;

        if (nCount == 0)
        {
            return 0;
        }

        ImsMain::SetConfiguration(event, nCount, pstSystemConfig);

        DestroySystemConfig(pstSystemConfig);
    }

    return 1;
}

static jlong JNI_GetInterface(JNIEnv* /* env */, jobject /* object */, jint nInterfaceType, jint nSlotId)
{
    BaseService* pService = IMS_NULL;

    if (nInterfaceType == IUIMS::SYSTEM_INTERFACE)
    {
        pService = new JniSystem(SendData2JavaEx);
    }
    else
    {
        pService = CoreInterfaceFactory::GetInterface(nInterfaceType, SendData2Java, nSlotId);
    }

    IMS_LOGD("JNI_GetInterface :: interfaceType=%d, object=%" PFLS_d,
            nInterfaceType, reinterpret_cast<IMS_SINTP>(pService));

    return static_cast<jlong>(reinterpret_cast<long>(pService));
}

static jint JNI_ReleaseInterface(JNIEnv* /* env */, jobject /* object */, jlong jNativeObject)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

#if defined(__IMS_CLANG__)
    IMS_LOGD("JNI_ReleaseInterface :: object=%ld", nNativeObject);
#else
    IMS_LOGD("JNI_ReleaseInterface :: object=%" PFLS_d, nNativeObject);
#endif

    BaseService *pService = reinterpret_cast<BaseService*>(nNativeObject);

    if (pService != IMS_NULL)
    {
        delete pService;
    }

    return 1;
}

static jint JNI_SendData(JNIEnv* env, jobject /* object */, jlong jNativeObject, jbyteArray baData)
{
    long nNativeObject = INT64_TO_SINTP(jNativeObject);

    IMS_TRACE_D("JNI_SendData :: object=%" PFLS_d, nNativeObject, 0, 0);

    BaseService *pService = reinterpret_cast<BaseService*>(nNativeObject);

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
    BaseService *pService = reinterpret_cast<BaseService*>(nNativeObject);

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

    if (parcelOut.dataSize() == 0) // error case
    {
        parcelOut.writeInt32(1);
    }

    jbyteArray byteArray = env->NewByteArray(parcelOut.dataSize());
    env->SetByteArrayRegion(byteArray, 0, parcelOut.dataSize(), (jbyte*)parcelOut.data());

    return byteArray;
}

static JNINativeMethod gMethods[] =
{
    {"construct", "()V", (void*)JNI_Construct},
    {"destruct", "()V", (void*)JNI_Destruct},
    {"getInterface", "(II)J", (void*)JNI_GetInterface},
    {"releaseInterface", "(J)I", (void*)JNI_ReleaseInterface},
    {"sendData", "(J[B)I", (void*)JNI_SendData},
    {"sendDataEx", "(J[B)[B", (void*)JNI_SendDataEx},
    {"setConfiguration", "(I[B)I", (void*)JNI_SetConfiguration},
};

// Google_IMS_IF :: VIDEO_CALL_PROVIDER {
int IMSInterface_GetSurface(const String8& str8Class,
        const String8& str8SurfaceName, long& nSurfaceObject)
{
    JNIEnv *pEnv = GetJNIEnv();

    if (pEnv == NULL)
    {
        IMS_TRACE_E(0, "JNIEnv is null", 0, 0, 0);
        return (-1);
    }

    nSurfaceObject = 0;

    // Find Surface member variable in UCMediaSession
    jclass clazz = pEnv->FindClass(str8Class.string());
    jfieldID surfaceFid = (clazz == NULL) ?
            NULL : pEnv->GetStaticFieldID(clazz,
                    str8SurfaceName.string(), "Landroid/view/Surface;");
    jobject surfaceObject = (surfaceFid == NULL) ?
            NULL : pEnv->GetStaticObjectField(clazz, surfaceFid);

    if (surfaceObject != NULL)
    {
        // Find a native object from android.view.Surface
        jclass surfaceClass = pEnv->FindClass("android/view/Surface");
        jfieldID surfaceNativeObject = (surfaceClass == NULL) ?
                NULL : pEnv->GetFieldID(surfaceClass, "mNativeObject", "J");
        Surface *pSurface = (surfaceNativeObject == NULL) ?
                NULL : (Surface*)pEnv->GetLongField(surfaceObject, surfaceNativeObject);

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
        IMS_TRACE_D("IMSInterface_ReleaseSurface - strongCount=%d",
                pSurface->getStrongCount(), 0, 0);

        pSurface->decStrong(0);
    }
}
// Google_IMS_IF :: VIDEO_CALL_PROVIDER }

jint IMSInterface_OnLoad(JavaVM* vm, JNIEnv* env)
{
    gJVM = vm;

    jclass jclassIms = env->FindClass(gClassIMSPathName);

    if (jclassIms == NULL)
    {
        IMS_LOGE("IMSInterface_OnLoad: FindClass failed");
        return -1;
    }

    gClass_JNIIms = (jclass)env->NewGlobalRef(jclassIms);

    if (jniRegisterNativeMethods(env,
            gClassIMSPathName, gMethods, NELEM(gMethods) ) < 0)
    {
        IMS_LOGE("IMSInterface_OnLoad: RegisterNatives failed");
        return -1;
    }

    gMethod_sendData2Java = env->GetStaticMethodID(gClass_JNIIms, "sendData2Java", "(J[B)I");
    gMethod_sendData2JavaEx = env->GetStaticMethodID(
            gClass_JNIIms, "sendData2JavaEx", "(J[BLjava/io/FileDescriptor;)[B");

    if ((gMethod_sendData2Java == NULL) || (gMethod_sendData2JavaEx == NULL))
    {
        IMS_LOGE("IMSInterface_OnLoad: GetStaticMethodID failed");
        return -1;
    }

    return JNI_VERSION_1_4;
}
