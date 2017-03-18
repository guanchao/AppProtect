LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := substrate
LOCAL_SRC_FILES := libsubstrate.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_SRC_FILES := hello.cpp

LOCAL_LDLIBS := -llog 
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := hackcodejiagu
LOCAL_SRC_FILES := hackcodejiagu.cpp

LOCAL_LDLIBS := -llog 
LOCAL_LDLIBS += -L$(LOCAL_PATH)  -lsubstrate
include $(BUILD_SHARED_LIBRARY)




