#ifndef IN_VIDEO_STREAM_H
#define IN_VIDEO_STREAM_H
#ifdef __cplusplus
extern "C"
{
#endif
#define __STDC_CONSTANT_MACROS
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#ifdef __cplusplus
}
#endif
#include <stdio.h>
#include <string>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <vector>
#include "sei_packet.h"


//#define HARDWARE_DECODE //硬件解码
#define SOFTWARE_DECODE //软件解码
#define INPUT_VIDEO_FILE //输入流是文件
#define GET_MAT // 需要解码成图片


//#define RAW_SIZE 50 // 默认阻塞队列的长度,提前申请的*3左右
//#define MULTIPLE 10 // 阻塞队列长度的倍数

class in_video_stream
{
public:
	in_video_stream();
	~in_video_stream();

	/**
	 * @brief      { 根据视频流的地址完成该流对象的初始化 }
	 *
	 * @param[in]  streamer_url  视频流的地址
	 *
	 * @return     { 流的初始化是否成功 }
	 */
	bool init(const std::string& streamer_url,const int mat_height_, const int mat_width_);

	/**
	 * @brief      { 释放流对象 }
	 */
	bool release();

	/**
	 * @brief      流是否被打开.
	 *
	 * @return     True if opened, False otherwise.
	 */
	bool get_is_opened();

	/**
	 * @brief      获取流的url.
	 *
	 * @return     The streamer url.
	 */
	std::string get_stream_url();

	/**
	 * @brief      设置流的url，当配置中流的url发生改变的时候重新初始化流.
	 *
	 * @param[in]  url   The url
	 */
	bool set_stream_url(const std::string& url, const int mat_height_, const int mat_width_);


	/**
	 * @brief      { 读取输入流，并将其解码为Mat，同时提取可能存在私有数据 }
	 *
	 * @param      image      解码后的Mat
	 * @param      pkt        裸流包
	 * @param      self_info  私有数据
	 *
	 * @return     { 是否成功读取输入流 }
	 */
	bool read(cv::Mat& image, AVPacket& packet);

	
	/**
	 * @brief 	  提取SEI中的未注册数据（私有）
	 * 
	 * @param     pkt   	裸流包
	 * @param 	  self_data 私有数据
	 * 
	 * @return 返回提取SEI数据是否成功
	 */
	bool get_sei_data(const AVPacket& packet, std::string &self_data);

	/**
	 * @brief      获取输入流的帧率.
	 *
	 * @return     The fps.
	 */
	int get_fps(); 

	/**
	 * @brief      Gets in video streamer information.
	 *
	 * @return     输入流相关的AVFormatContext指针对象.
	 */
	AVFormatContext* get_in_video_stream_info();

#ifdef GET_MAT
	void set_mat_size(const int mat_height_, const int mat_width_);

	void get_mat_size(int &mat_height_,int &mat_width_);
#endif // GET_MAT
	
private:

	AVFormatContext *i_fmt_ctx;
	AVStream *i_video_stream;
	AVFrame *h264_frame,*rgb_frame;
	AVDictionary *dict;
	AVCodecContext *i_cod_ctx;
	uint8_t *out_buffer;
    struct SwsContext *img_convert_ctx;
    AVCodec *i_codec;
	int pix_size,ret,got_picture;
	int video_index;
	std::string stream_url; //流地址
	bool is_opened;
	uint frame_index;
	uint mat_height; // 解码后Mat的高
	uint mat_width; // 解码后Mat的宽
	int64_t start_time;
	//std::vector<cv::Mat> mat_cache;
};



#endif //IN_VIDEO_STREAMER_H