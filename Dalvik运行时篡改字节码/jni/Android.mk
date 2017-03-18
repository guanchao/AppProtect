LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := demo1
LOCAL_SRC_FILES := demo1.cpp
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)


