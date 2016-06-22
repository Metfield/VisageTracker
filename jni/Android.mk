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

LOCAL_C_INCLUDES +=  jni/includes/rapidjson
LOCAL_C_INCLUDES +=  jni/includes/glm
LOCAL_C_INCLUDES +=  jni/includes/libpng-android/include
LOCAL_C_INCLUDES +=  $(VISAGE_HEADERS) $(VISAGE_HEADERS)/libAfm $(VISAGE_HEADERS)/libVRMLParser $(VISAGE_HEADERS)/../source/libVision/swr
LOCAL_MODULE    := VisageTracker
LOCAL_SRC_FILES := VisageTracker.cpp \
				   NativeTrackerRenderer.cpp \
				   ActionUnitBinding.cpp \
				   AndroidCameraCapture.cpp \
				   ModelLoader.cpp \
				   tiny_obj_loader.cc \
				   includes/libpng-android/png.c \
				   includes/libpng-android/pngerror.c \
			       includes/libpng-android/pngget.c \
				   includes/libpng-android/pngmem.c \
				   includes/libpng-android/pngpread.c \
				   includes/libpng-android/pngread.c \
				   includes/libpng-android/pngrio.c \
				   includes/libpng-android/pngrtran.c \
				   includes/libpng-android/pngrutil.c \
				   includes/libpng-android/pngset.c \
				   includes/libpng-android/pngtrans.c \
				   includes/libpng-android/pngwio.c \
				   includes/libpng-android/pngwrite.c \
				   includes/libpng-android/pngwtran.c \
				   includes/libpng-android/pngwutil.c

	LOCAL_SHARED_LIBRARIES := VisageVision	
	LOCAL_LDLIBS +=  -landroid -L$(VISAGE_LIBS) -L$(/jni) -lVisageVision -lGLESv2 -llog -ldl -Wl,--gc-sections  
	
	LOCAL_CFLAGS := -DANDROID_NDK \
					-DDISABLE_IMPORTGL \
					-DANDROID \
					-DVISAGE_STATIC \
					-ffast-math -O2 -funroll-loops -Wno-write-strings
	
	LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
