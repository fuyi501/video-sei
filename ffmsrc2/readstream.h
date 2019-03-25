#ifndef READSTREAM_H
#define READSTREAM_H

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "in_video_stream.h"
#include "out_video_stream.h"
#include "json.hpp"

using json = nlohmann::json;

#define INIT_TIME 50 // 任意一个流初始化的次数上限，超过了退出程序
#define CHECK_CONFIG 5 // 每3s检查一次配置文件
#define TIME_ERROR_THRESHOLD 50 // 读取输入流失败的次数（达到该次数后重新初始化输入流）
#define RAW_SIZE 50 // 默认阻塞队列的长度,提前申请的*3左右
#define MULTIPLE 3 // 阻塞队列长度的倍数
#define DEBUG_OPEN // 开启调试模式
//#define SAVE_FILE // 表示video_center要存储视频文件

class read_stream{
public:
	read_stream();
	~read_stream();

	/**
	 * @brief      { 【接口函数】初始化视频输入流、一个视频文件输出流、一个视频转发输出流 }
	 *
	 * @param[in]  config  json格式的string，配置的具体内容
	 *
	 * @return     { 是否成功初始化流 }
	 */
	bool init(const std::string& config);

	void startrun();
	void settimeerror();


	/**
	 * @brief      { 【接口函数】运行流处理中心 }
	 */
	std::string run();
	std::string run(cv::Mat &image);



	/**
	 * @brief      { 根据配置文件初始化输入流 }
	 *
	 * @param[in]  config  配置文件内容
	 *
	 * @return     { 是否成功初始化输入流 }
	 */
	bool init_in_stream(const std::string& config);


	/**
	 * @brief      { 初始化AVPacket }
	 *
	 * @param      packet  The packet
	 */
	void init_AVPacket(AVPacket &packet);



private:
	in_video_stream in_stream;
	AVPacket packet_url;
	std::string video_stream_center_name;
	std::string in_video_stream_url; // 输入流的rtsp
	std::string video_config; // 配置的具体内容
	int write_time; // 写入文件的时间
	int mat_height,mat_width; // 解码成Mat的高和宽
	int time_error; // 操作失败的次数
	std::vector<cv::Mat> mat_cache;
	u_int mat_index; // mat_cache的下标
};

#endif //VIDEO_STREAM_CENTER_H

/* 注意：
  （1）配置文件的中mat_width和mat_height不能修改
*/
