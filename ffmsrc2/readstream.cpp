#include "readstream.h"

read_stream::read_stream() {

}
read_stream::~read_stream() {}


bool read_stream::init(const std::string& config)
{
	mat_index = 0;
	time_error = 0;
	int ret;
	json stream_info;
	// 初始化流的参数
	std::cout<<"开始初始化 read_stream"<<std::endl;

	try{
		stream_info = json::parse(config);
	}catch(nlohmann::detail::parse_error){
		std::cout<<"read_stream::config file json'format is wrong";
	}
	try{
		this->video_stream_center_name = stream_info["video_stream_center_name"].get<std::string>();
	}catch(nlohmann::detail::type_error){
		std::cout<<"fail to get read_stream'name";
	}
	this->video_config = config;
	
	// 初始化输入流
	ret = init_in_stream(config);
	if(!ret){
		std::cout<<"fail to open in_video_stream";
	}else{
		std::cout<<"初始化成功输入流"<<std::endl ;
	}

	// std::cout<<"输入视频url: "<<in_video_stream_url<<std::endl ;
	// std::cout<<"高: "<<this->mat_height<<std::endl ;
	// std::cout<<"宽: "<<this->mat_width<<std::endl ;

	// 初始化文件、转发输出流的裸流包
	init_AVPacket(packet_url);

	time_error = 0;

	return true;
}

// 初始化视频输入流(包含初始化失败重新初始化)
bool read_stream::init_in_stream(const std::string& config)
{
	int ret = 0;
	int init_time = 0;
	while(true){
		json stream_info;
		// 如果已经打开过就释放
		std::cout<<"如果已经打开过就释放"<<std::endl ;
		if(in_stream.get_is_opened())
			in_stream.release();
		// 对输入流信息进行解析
		std::cout<<"初始化输入流信息"<<std::endl ;

		try{
			stream_info = json::parse(config);
		}catch(nlohmann::detail::parse_error){
			std::cout<<"init in_stream config file json'format is wrong";
		}
		try{
			this->in_video_stream_url = stream_info["in_video_stream"]["in_url"].get<std::string>();
			this->mat_height = stream_info["in_video_stream"]["mat_height"].get<int>();
			this->mat_width = stream_info["in_video_stream"]["mat_width"].get<int>();
		}catch(nlohmann::detail::type_error){
			std::cout<<"in_stream parameters are wrong";
		}
		ret = in_stream.init(in_video_stream_url,mat_height,mat_width);
		if(ret){
			std::cout<<"初始化 in_stream 成功，ret 为："<<ret<<std::endl ;
			return true;
		}else{
			std::cout<<"fail to init in_stream"<<std::endl;
			if(++init_time > INIT_TIME){
				return false;
			}
		}
	}
}

void read_stream::init_AVPacket(AVPacket &packet)
{
	memset(&packet, 0, sizeof(packet));
    av_init_packet(&packet);
}

void read_stream::startrun()
{
	// 提前申请好mat_cache的空间，以防在高速loop中Mat申请和释放空间存在问题
	// 这部分也不能放入init中
    for(uint i=0; i< RAW_SIZE*MULTIPLE; ++i){
        cv::Mat cache(cv::Size(this->mat_width,this->mat_height),CV_8UC3);
        mat_cache.push_back(cache);
    }
}

void read_stream::settimeerror()
{
	time_error = 0;
}

std::string read_stream::run(cv::Mat &image)
{
	int ret;
	std::cout<<"开始运行了"<<std::endl;
	
	
	std::string self_data_temp = "";

	// mat_cache中选取一个空间
	image = mat_cache[mat_index % (RAW_SIZE*MULTIPLE)];
	if(++mat_index == RAW_SIZE*MULTIPLE) mat_index = 0;

	std::cout<<"packet_url.size："<<packet_url.size<<std::endl;

	ret = in_stream.read(image, packet_url);

	if(!ret){
		std::cout<<"fail to read frame from in_stream";
		if(++time_error >= TIME_ERROR_THRESHOLD)
		{
			std::cout<<"the time of reading frame unsuccessfully are over threshold\n the center will be inited.";
			time_error = 0;
			init(this->video_config);
		}
	}
	
	// std::cout<<"图片："<<image<<std::endl;
	std::cout<<"packet_url.size："<<packet_url.size<<std::endl;
	std::cout<<"packet_data："<<packet_url.data<<std::endl;

	// only for test get_sei_data
	if(ret && 0 != packet_url.size){

		bool get_self_data = in_stream.get_sei_data(packet_url, self_data_temp);

		std::cout<<"get_self_data："<<get_self_data<<std::endl;

		if(get_self_data) {
			std::cout<<"得到的私有数据："<<self_data_temp<<std::endl;
			return self_data_temp;
		} else {
			std::cout<<"没有私有流数据"<<std::endl;
		}
	}
	
}
