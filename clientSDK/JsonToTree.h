#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "json.hpp"
#include <jni.h>
#include <opencv2/opencv.hpp>
using json = nlohmann::json;
using namespace std;
void jsonHandle(string paraStr, JNIEnv *env, jobject listener, jmethodID method);
