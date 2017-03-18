LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := demo
LOCAL_SRC_FILES := demo.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)


