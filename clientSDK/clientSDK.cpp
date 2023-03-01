#include "com_example_demo_FCClient.h"
#include <jni.h>
#include <iostream>
#include "base64.h"
#include "JsonToTree.h"


using namespace std;

char* jstringTostring(JNIEnv* env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);

		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	return rtn;
}

jstring stoJstring(JNIEnv* env, const char* pat)
{
	jclass strClass = env->FindClass("Ljava/lang/String;");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(strlen(pat));
	env->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}


void imgHandle(string paraStr, JNIEnv *env, jobject listener, jmethodID method) {
	//调用哦那onMessage函数
	char paramStr[] = "abcdeasdfasdfasf";
	cv::Mat src = Base2Mat(paraStr);
	string res = Mat2Base64(src, ".png");
	env->CallVoidMethod(listener, method, stoJstring(env, res.data()));
}

/*
 * Class:     com_mlight_floorcontrol_client_FCClient
 * Method:    onMsg
 * Signature: (Lcom/mlight/floorcontrol/client/FCListener;)V
 */
JNIEXPORT void JNICALL Java_com_example_demo_FCClient_eventHandle
(JNIEnv *env, jobject, jobject listener, jint event, jstring para) {
	cout << "this is event from java to cpp:" << event << endl;
	cout << "this is para from java to cpp:" << para << endl;
	//获得java的onMessage函数
	jclass cls = env->GetObjectClass(listener);
	jmethodID method = env->GetMethodID(cls, "onMessage", "(Ljava/lang/String;)V");
	//转string
	string paraStr = jstringTostring(env,para);
	string res1 = "";

	//处理过程
	switch (event) {
	case 1:
		//cv::Mat src= Base2Mat(paraStr);
		//res1 = Mat2Base64(src, ".png");
		imgHandle(paraStr,env,listener,method);
		break;
	case 2:
		jsonHandle(paraStr, env,listener, method);
		break;

	}
}

