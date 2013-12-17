LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hid-gadget-test

LOCAL_SRC_FILES := hid-gadget-test.c

include $(BUILD_EXECUTABLE)
