LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

	ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	VISAGE_LIBS := ../../../../lib/armeabi-v7a
	endif
	ifeq ($(TARGET_ARCH_ABI),x86)
	VISAGE_LIBS := ../../../../lib/x86
	endif
	LOCAL_MODULE := VisageVision
	LOCAL_SRC_FILES := $(VISAGE_LIBS)/libVisageVision.so
	include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

	OPENCV_LIB_TYPE := STATIC
	OPENCV_CAMERA_MODULES := on
	                
	include ../includeOpenCV.mk
	ifeq ("$(wildcard $(OPENCV_MK_PATH))","")
		#try to load OpenCV.mk from default install location
		include $(TOOLCHAIN_PREBUILT_ROOT)/user/share/OpenCV/OpenCV.mk
	else
		include $(OPENCV_MK_PATH)
	endif
	 
	VISAGE_HEADERS  := ../../../include
	
	ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	VISAGE_LIBS := ../../../lib/armeabi-v7a
	endif
	ifeq ($(TARGET_ARCH_ABI),x86)
	VISAGE_LIBS := ../../../lib/x86
	endif

LOCAL_C_INCLUDES +=  $(VISAGE_HEADERS) $(VISAGE_HEADERS)/libAfm $(VISAGE_HEADERS)/libVRMLParser $(VISAGE_HEADERS)/../source/libVision/swr
LOCAL_MODULE    := VisageTracker
LOCAL_SRC_FILES := VisageTracker.cpp \
				   NativeTrackerRenderer.cpp \
				   ActionUnitBinding.cpp

	LOCAL_SHARED_LIBRARIES := VisageVision	
	LOCAL_LDLIBS +=  -landroid -L$(VISAGE_LIBS) -L$(/jni) -lVisageVision -lGLESv1_CM -llog -ldl -Wl,--gc-sections  
	
	LOCAL_CFLAGS := -DANDROID_NDK \
					-DDISABLE_IMPORTGL \
					-DANDROID \
					-DVISAGE_STATIC \
					-ffast-math -O2 -funroll-loops -Wno-write-strings
	
	LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
