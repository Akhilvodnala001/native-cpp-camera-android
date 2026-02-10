#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"NATIVE_CAM",__VA_ARGS__)

static JavaVM* gJvm = nullptr;
static jobject gActivity = nullptr;

static bool hasPermission=false, hasSurface=false;

static ANativeWindow* previewWindow=nullptr;
static ANativeWindow* imageWindow=nullptr;

static ACameraManager* cameraManager=nullptr;
static ACameraDevice* cameraDevice=nullptr;
static ACameraCaptureSession* captureSession=nullptr;
static ACaptureRequest* previewRequest=nullptr;
static AImageReader* imageReader=nullptr;

/* ---------- SAVE IMAGE ---------- */

void saveToGallery(JNIEnv* env,uint8_t* data,int len){
    jclass actCls=env->GetObjectClass(gActivity);
    jobject resolver=env->CallObjectMethod(
            gActivity,
            env->GetMethodID(actCls,"getContentResolver","()Landroid/content/ContentResolver;")
    );

    jclass mediaCls=env->FindClass("android/provider/MediaStore$Images$Media");
    jobject collection=env->GetStaticObjectField(
            mediaCls,
            env->GetStaticFieldID(mediaCls,"EXTERNAL_CONTENT_URI","Landroid/net/Uri;")
    );

    jclass valuesCls=env->FindClass("android/content/ContentValues");
    jobject values=env->NewObject(valuesCls,env->GetMethodID(valuesCls,"<init>","()V"));
    jmethodID put=env->GetMethodID(valuesCls,"put","(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallVoidMethod(values,put,env->NewStringUTF("mime_type"),env->NewStringUTF("image/jpeg"));

    jclass resolverCls=env->FindClass("android/content/ContentResolver");
    jobject uri=env->CallObjectMethod(
            resolver,
            env->GetMethodID(resolverCls,"insert",
                             "(Landroid/net/Uri;Landroid/content/ContentValues;)Landroid/net/Uri;"),
            collection,values);

    jobject out=env->CallObjectMethod(
            resolver,
            env->GetMethodID(resolverCls,"openOutputStream",
                             "(Landroid/net/Uri;)Ljava/io/OutputStream;"),uri);

    jbyteArray arr=env->NewByteArray(len);
    env->SetByteArrayRegion(arr,0,len,(jbyte*)data);

    jclass outCls=env->FindClass("java/io/OutputStream");
    env->CallVoidMethod(out,
                        env->GetMethodID(outCls,"write","([B)V"),arr);

    LOGI("Saved to gallery");
}

/* ---------- IMAGE CALLBACK ---------- */

void onImageAvailable(void*,AImageReader* reader){
    AImage* image=nullptr;
    if(AImageReader_acquireNextImage(reader,&image)!=AMEDIA_OK) return;

    uint8_t* data=nullptr;
    int len=0;
    AImage_getPlaneData(image,0,&data,&len);

    JNIEnv* env;
    gJvm->AttachCurrentThread(&env,nullptr);
    saveToGallery(env,data,len);
    gJvm->DetachCurrentThread();

    AImage_delete(image);

    /* RESTART PREVIEW AFTER CAPTURE */
    ACameraCaptureSession_setRepeatingRequest(
            captureSession,nullptr,1,&previewRequest,nullptr);
}

/* ---------- CAMERA CALLBACKS ---------- */

void onDisconnected(void*,ACameraDevice* dev){ ACameraDevice_close(dev); }
void onError(void*,ACameraDevice* dev,int){ ACameraDevice_close(dev); }

ACameraDevice_StateCallbacks devCb{nullptr,onDisconnected,onError};

/* ---------- START CAMERA ---------- */

void startCamera(){
    if(!hasPermission||!hasSurface||cameraDevice) return;

    cameraManager=ACameraManager_create();

    ACameraIdList* ids;
    ACameraManager_getCameraIdList(cameraManager,&ids);
    const char* camId=ids->cameraIds[0];

    ACameraManager_openCamera(cameraManager,camId,&devCb,&cameraDevice);
    ACameraManager_deleteCameraIdList(ids);

    AImageReader_new(1920,1080,AIMAGE_FORMAT_JPEG,2,&imageReader);
    AImageReader_getWindow(imageReader,&imageWindow);

    AImageReader_ImageListener listener{nullptr,onImageAvailable};
    AImageReader_setImageListener(imageReader,&listener);

    ACaptureSessionOutputContainer* container;
    ACaptureSessionOutputContainer_create(&container);

    ACaptureSessionOutput* prevOut;
    ACaptureSessionOutput_create(previewWindow,&prevOut);
    ACaptureSessionOutputContainer_add(container,prevOut);

    ACaptureSessionOutput* imgOut;
    ACaptureSessionOutput_create(imageWindow,&imgOut);
    ACaptureSessionOutputContainer_add(container,imgOut);

    ACameraDevice_createCaptureRequest(cameraDevice,
                                       TEMPLATE_PREVIEW,&previewRequest);

    ACameraOutputTarget* target;
    ACameraOutputTarget_create(previewWindow,&target);
    ACaptureRequest_addTarget(previewRequest,target);

    ACameraCaptureSession_stateCallbacks scb{};
    ACameraDevice_createCaptureSession(cameraDevice,container,&scb,&captureSession);

    ACameraCaptureSession_setRepeatingRequest(
            captureSession,nullptr,1,&previewRequest,nullptr);

    LOGI("Camera started");
}

/* ---------- STOP ---------- */

void stopCamera(){
    if(captureSession) ACameraCaptureSession_close(captureSession);
    if(cameraDevice) ACameraDevice_close(cameraDevice);
    if(cameraManager) ACameraManager_delete(cameraManager);

    captureSession=nullptr;
    cameraDevice=nullptr;
    cameraManager=nullptr;
}

/* ---------- JNI ---------- */

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativecamera_MainActivity_nativeSetSurface(
        JNIEnv* env,jobject activity,jobject surface){
    env->GetJavaVM(&gJvm);
    gActivity=env->NewGlobalRef(activity);
    previewWindow=ANativeWindow_fromSurface(env,surface);
    hasSurface=true;
    startCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativecamera_MainActivity_nativeOnPermissionResult(
        JNIEnv*,jobject,jboolean granted){
    hasPermission=granted;
    startCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativecamera_MainActivity_nativeOnResume(
        JNIEnv*,jobject){
    startCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativecamera_MainActivity_nativeOnPause(
        JNIEnv*,jobject){
    stopCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativecamera_MainActivity_nativeCaptureImage(
        JNIEnv*,jobject){

    if(!captureSession||!cameraDevice) return;

    ACaptureRequest* request;
    ACameraDevice_createCaptureRequest(
            cameraDevice,TEMPLATE_STILL_CAPTURE,&request);

    ACameraOutputTarget* target;
    ACameraOutputTarget_create(imageWindow,&target);
    ACaptureRequest_addTarget(request,target);

    int32_t orient=90;
    ACaptureRequest_setEntry_i32(
            request,ACAMERA_JPEG_ORIENTATION,1,&orient);

    ACameraCaptureSession_capture(
            captureSession,nullptr,1,&request,nullptr);

    LOGI("Still capture");
}
