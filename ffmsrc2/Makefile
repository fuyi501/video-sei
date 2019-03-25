INC_ROOT := ../external/libs
OPENCV := $(INC_ROOT)/opencv2
FFMPEG := $(INC_ROOT)/ffmpeg3.4
LIBVPX := $(INC_ROOT)/libvpx-1.7.0-h439df22_0
BOOST := $(INC_ROOT)/boost
GLOG := $(INC_ROOT)/glog
GFLAG := $(INC_ROOT)/gflag

OPENCV_LIB_DIR :=  $(OPENCV)/lib
FFMPEG_LIB_DIR := $(FFMPEG)/lib
LIBVPX_LIB_DIR := $(LIBVPX)/lib
BOOST_LIB_DIR := $(BOOST)/lib
GLOG_LIB_DIR := $(GLOG)/lib
GFLAG_LIB_DIR := $(GFLAG)/lib

VIDEO_STREAM = ./video_stream
COMMON := ../common


INC_DIR := ./ $(COMMON) $(VIDEO_STREAM) \
			$(BOOST)/include $(OPENCV)/include $(GLOG)/include $(FFMPEG)/include $(LIBVPX)/include $(GFLAG)/include


TAG_DIR := .

#EXECUTABLE的值，生成可执行文件的地址，“./exe”表示在当前目录下生成exe可执行文件
EXECUTABLE := $(TAG_DIR)/video_read_stream

#LIBS的值，空格分隔，所用库的名称
# LIBS :=  pthread opencv_core opencv_imgproc opencv_highgui avcodec avformat swscale avutil glog gflags boost_date_time boost_thread boost_system vpx
LIBS := opencv_core opencv_highgui avcodec avformat swscale avutil 

#SOURCE的值，空格分隔，自己工程下的所用源文件，以下分别表示当前目录（makefile所在目录）下的.c文件，.cPP文件，当前目录下的sub_src子目录中的的.c文件，.cPP文件。如果你有很多个子文件夹，只需要将$(wildcard sub_src/*.c)中的sub_src换成你自己的目录，按照这样的方式添加完成即可。
SOURCE := $(wildcard *.c) $(wildcard *.cpp) $(wildcard ${VIDEO_STREAM}/*.cpp)

########## 以下基本上不用改,要改最可能改的就是参数############################################
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) $(patsubst %.d,%.cpp,$(MISSING_DEPS)))
#编译器参数
CFLAGS += -g -Wall -fopenmp

CXXFLAGS += $(CFLAGS) 
CPPFLAGS += -MD -std=c++11
CPPFLAGS += $(addprefix -I, $(INC_DIR))
#LMFLAGS自定义参数，主要用于使用一些特别的库时，链接需要特别指定
LMFLAGS := -fopenmp

RM := rm -f
$(EXECUTABLE) : $(OBJS)
	@$(RM) $@
	$(CXX) -o $(EXECUTABLE) $(OBJS) $(addprefix -L, $(OPENCV_LIB_DIR)) $(addprefix -L, $(LIBVPX_LIB_DIR)) $(addprefix -L, $(FFMPEG_LIB_DIR)) $(addprefix -L, $(BOOST_LIB_DIR)) $(addprefix -L, $(GLOG_LIB_DIR)) $(addprefix -L, $(GFLAG_LIB_DIR)) -Wl,-rpath=$(FFMPEG_LIB_DIR) -Wl,-rpath=$(OPENCV_LIB_DIR) -Wl,-rpath=$(GLOG_LIB_DIR) -Wl,-rpath=$(GFLAG_LIB_DIR) $(addprefix -l, $(LIBS)) -fopenmp


$(OBJS):$(SOURCE)


.PHONY : everything deps objs clean 

everything : $(EXECUTABLE)

deps : $(DEPS)

objs : $(OBJS)

clean :
	@$(RM) $(OBJS)
	@$(RM) $(DEPS)
	@$(RM) $(EXECUTABLE)

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)


