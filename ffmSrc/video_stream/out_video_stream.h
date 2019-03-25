#ifndef OUT_VIDEO_STREAM_H
#define OUT_VIDEO_STREAM_H
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
#include <string>
#include <string.h>
#include <iostream>
#include "sei_packet.h"
// #include "json.hpp"
// using json = nlohmann::json;

class out_video_stream
{
public:

	out_video_stream();
	~out_video_stream();

	/**
	 * @brief      { 根据视频流的地址完成该流对象的初始化 }
	 *
	 * @param[in]  streamer_url  视频流的地址
	 *
	 * @return     { 流的初始化是否成功 }
	 */
	bool init(const std::string& url, AVFormatContext* i_fmt_ctx, const bool is_file, const int minute, const int fps);

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
	 * @brief      获取流的url.
	 *
	 * @param[in]  url   The url
	 */
	bool set_stream_url(const std::string& url);

	/**
	 * @brief      Sets the fps.
	 */
	int get_fps();

	void set_write_time(const int minute);

	int get_write_time();


	/**
	 * @brief      当self_define_info不为NULL的时候，作用是视频流添加私有流信息，否则将AVPacket写入文件中
	 *
	 * @param      cur_packet         The current packet
	 * @param[in]  self_defined_info  The self defined information
	 *
	 * @return     { 返回是否成功添加私有信息 }
	 */
	bool write(AVPacket& cur_packet,const std::string& self_info);

	// for audio
	/**
	 * @brief      { 初始化音频流 }
	 *
	 * @return     { 是否成功初始化音频流 }
	 */
	bool init_audio_stream();

private:
	AVFormatContext *o_fmt_ctx; //视频输出流
	AVFormatContext *i_fmt_ctx_audio; //音频输入流
	AVFormatContext *i_fmt_ctx; //保存对应的输入流信息
	AVPacket self_packet; //私有流的AVPacket
	AVPacket self_packet_copy;
	AVStream *o_video_stream;
	AVDictionary *dict;
	std::string stream_url; //流地址
	int fps; //帧率
	bool is_file; //是否为文件
	int write_time; // 写入文件的时间长度（以分钟为单位）
	bool is_opened;


};


#endif //OUT_VIDEO_STREAMER_H