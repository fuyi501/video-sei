#ifndef VIDEO_STREAM_CENTER_H
#define VIDEO_STREAM_CENTER_H

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <glog/logging.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
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

class video_stream_center{
public:
	video_stream_center();
	~video_stream_center();

	/**
	 * @brief      { 【接口函数】初始化视频输入流、一个视频文件输出流、一个视频转发输出流 }
	 *
	 * @param[in]  config  json格式的string，配置的具体内容
	 *
	 * @return     { 是否成功初始化流 }
	 */
	bool init(const std::string& config);



	/**
	 * @brief      { 【接口函数】运行流处理中心 }
	 */
	void run();


	/**
	 * @brief      { 根据配置文件初始化输入流 }
	 *
	 * @param[in]  config  配置文件内容
	 *
	 * @return     { 是否成功初始化输入流 }
	 */
	bool init_in_stream(const std::string& config);

	/**
	 * @brief      { 初始化转发输出流 }
	 *
	 * @param[in]  config  The configuration
	 *
	 * @return     { 是否成功初始化转发输出流 }
	 */
	bool init_out_stream_url(const std::string& config);

	/**
	 * @brief      { 初始化文件输出流 }
	 *
	 * @param[in]  config  The configuration
	 *
	 * @return     { 是否成功输出化 }
	 */
	bool init_out_stream_file(const std::string& config);


	/**
	 * @brief      { 更新文件输出流的参数 }
	 *
	 * @param[in]  config  配置文件内容
	 *
	 */
	void update_out_stream_file_info(const std::string& config);


	/**
	 * @brief      { 检查流的配置是否发生改变，并根据配置的修改来重新初始化流 }
	 *
	 * @param[in]  config  string类型的配置内容
	 *
	 * @return     { ture表示配置未发生改变或者按照新的配置重新完成，false表示按照新的配置初始化失败 }
	 */
	bool check_config(const std::string& all_config_str);
	// 通过配置服务器的客户端获取配置文件内容，检查是否发生参数的变化，如果存在变化重新初始化相应的流
	// 暂时传入的参数为配置文件的地址


	/**
	 * @brief      { 初始化AVPacket }
	 *
	 * @param      packet  The packet
	 */
	void init_AVPacket(AVPacket &packet);

	/**
	 * @brief      根据配置文件生成视频文件名
	 *
	 * @return     视频文件的名字
	 */
	std::string get_filename();
	// 暂时使用默认的格式来命名

	// 用于开线程
	void start();


private:
	in_video_stream in_stream;
	out_video_stream out_stream_url;
	out_video_stream out_stream_file;
	AVPacket packet_url,packet_file;
	boost::mutex mtx; //互斥量
	std::string video_stream_center_name;
	std::string in_video_stream_url; // 输入流的rtsp
	std::string out_video_stream_url; // 转发流的url
	std::string out_video_stream_url_ip; // 转发流的url的ip
	std::string out_video_stream_url_port; // 转发流的url的端口号
	std::string out_video_stream_file_path; // 视频存储的路径
	std::string filename_format; // 文件的命名格式
	std::string filename_prefix; // 视频文件前缀
	std::string video_format; // 视频格式
	std::string video_config; // 配置的具体内容
	std::string audio_path; //音频文件的位置
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
