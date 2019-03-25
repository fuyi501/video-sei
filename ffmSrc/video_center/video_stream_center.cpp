#include "video_stream_center.h"

video_stream_center::video_stream_center() {

}
video_stream_center::~video_stream_center() {}


bool video_stream_center::init(const std::string& config)
{
	mat_index = 0;
	time_error = 0;
	int ret;
	json stream_info;
	// 初始化流的参数
	try{
		stream_info = json::parse(config);
	}catch(nlohmann::detail::parse_error){
		LOG(ERROR)<<"video_stream_center::config file json'format is wrong";
	}
	try{
		this->video_stream_center_name = stream_info["video_stream_center_name"].get<std::string>();
	}catch(nlohmann::detail::type_error){
		LOG(ERROR)<<"fail to get video_stream_center'name";
	}
	this->video_config = config;
	
	// 更新文件流配置信息
	update_out_stream_file_info(this->video_config);

	std::cout<<"视频名字: "<<this->video_stream_center_name<<std::endl ;
	std::cout<<"输出路径: "<<this->out_video_stream_file_path<<std::endl ;
	std::cout<<"时间: "<<this->write_time<<std::endl ;
	std::cout<<"格式化: "<<this->filename_format<<std::endl ;
	std::cout<<"前缀: "<<this->filename_prefix<<std::endl ;
	std::cout<<"格式: "<<this->video_format<<std::endl ;


	// 初始化输入流
	ret = init_in_stream(config);
	if(!ret){
		LOG(ERROR)<<"fail to open in_video_stream";
	}else{
		LOG(INFO)<<"successed in opening in_video_stream";
	}

	std::cout<<"输入视频url: "<<this->in_video_stream_url<<std::endl ;
	std::cout<<"高: "<<this->mat_height<<std::endl ;
	std::cout<<"宽: "<<this->mat_width<<std::endl ;


	// 初始化转发输出流
	ret = init_out_stream_url(this->video_config);
	if(!ret){
		LOG(ERROR)<<"fail to open out_video_stream_url";
	}else{
		LOG(INFO)<<"successed in opening out_video_stream_url";
	}

	std::cout<<"输出视频 ip: "<<this->out_video_stream_url_ip<<std::endl ;
	std::cout<<"输出视频 port: "<<this->out_video_stream_url_port<<std::endl ;
	std::cout<<"输出视频 url: "<<this->out_video_stream_url<<std::endl ;



	// 初始化文件、转发输出流的裸流包
	init_AVPacket(packet_url);
	init_AVPacket(packet_file);

	// 文件输出流在流处理中心运行时候初始化

	return true;
}

void video_stream_center::update_out_stream_file_info(const std::string& config)
{
	json stream_info;
	try{
		std::cout<<config<<std::endl;
		stream_info = json::parse(config);
	}catch(nlohmann::detail::parse_error){
		LOG(ERROR)<<"update out_stream_file config file json'format is wrong";
	}
	try{
		this->out_video_stream_file_path = stream_info["out_video_stream_file"]["video_path"].get<std::string>();
		this->write_time = stream_info["out_video_stream_file"]["write_time"].get<int>();
		this->filename_format = stream_info["out_video_stream_file"]["filename_format"].get<std::string>();
		this->filename_prefix = stream_info["out_video_stream_file"]["filename_prefix"].get<std::string>();
		this->video_format = stream_info["out_video_stream_file"]["video_format"].get<std::string>();
	}catch(nlohmann::detail::type_error){
		LOG(ERROR)<<"out_stream_file parameters are wrong";
	}

	// 不存在相应的文件夹==>建立
	std::string video_dir = this->out_video_stream_file_path + "/" + this->video_stream_center_name;
	int state = access(video_dir.c_str(), R_OK|W_OK); // 头文件 #include <unistd.h>
	if(0 != state){
		video_dir = "mkdir " + video_dir;
		LOG(INFO)<<video_dir;
		system(video_dir.c_str());
	}
}


// 初始化视频输入流(包含初始化失败重新初始化)
bool video_stream_center::init_in_stream(const std::string& config)
{
	int ret = 0;
	int init_time = 0;
	while(true){
		json stream_info;
		// 如果已经打开过就释放
		if(in_stream.get_is_opened())
			in_stream.release();
		// 对输入流信息进行解析
		try{
			stream_info = json::parse(config);
		}catch(nlohmann::detail::parse_error){
			LOG(ERROR)<<"init in_stream config file json'format is wrong";
		}
		try{
			this->in_video_stream_url = stream_info["in_video_stream"]["in_url"].get<std::string>();
			this->mat_height = stream_info["in_video_stream"]["mat_height"].get<int>();
			this->mat_width = stream_info["in_video_stream"]["mat_width"].get<int>();
		}catch(nlohmann::detail::type_error){
			LOG(ERROR)<<"in_stream parameters are wrong";
		}
		ret = in_stream.init(in_video_stream_url,mat_height,mat_width);
		if(ret){
			return true;
		}else{
			std::cout<<"fail to init in_stream"<<std::endl;
			if(++init_time > INIT_TIME){
				return false;
			}
		}
	}
}



bool video_stream_center::init_out_stream_url(const std::string& config)
{
	int ret = 0;
	int init_time = 0;
	while(true){
		json stream_info;
		// 如果已经打开过就释放
		if(out_stream_url.get_is_opened())
			out_stream_url.release();
		try{
			stream_info = json::parse(config);
		}catch(nlohmann::detail::parse_error){
			LOG(ERROR)<<"init out_stream_url config file json'format are wrong";
		}
		try{
			this->out_video_stream_url_ip = stream_info["out_video_stream_url"]["out_url_ip"].get<std::string>();
			this->out_video_stream_url_port = stream_info["out_video_stream_url"]["out_url_port"].get<std::string>();
			this->out_video_stream_url = out_video_stream_url_ip + ":" + out_video_stream_url_port;
		}catch(nlohmann::detail::type_error){
			LOG(ERROR)<<"out_stream_url parameters are wrong";
		}
		ret = out_stream_url.init(out_video_stream_url,
			in_stream.get_in_video_stream_info(),
			false,-1,in_stream.get_fps());
		if(ret){
			return true;
		}else{
			std::cout<<"fail to init out_stream_url"<<std::endl;
			if(++init_time > INIT_TIME){
				return false;
			}
		}
	}
}


bool video_stream_center::init_out_stream_file(const std::string& config)
{
	int ret = 0;
	int init_time = 0;
	while(true){
		if(out_stream_file.get_is_opened())
			out_stream_file.release();
		update_out_stream_file_info(config);
		std::string filename = get_filename();
		ret = out_stream_file.init(filename,in_stream.get_in_video_stream_info(),true,write_time,in_stream.get_fps());
		if(ret){
			return true;
		}else{
			LOG(ERROR)<<"fail to init out_stream_file";
			if(++init_time > INIT_TIME){
				return false;
			}
		}
	}
}


void video_stream_center::init_AVPacket(AVPacket &packet)
{
	memset(&packet, 0, sizeof(packet));
    av_init_packet(&packet);
}


std::string video_stream_center::get_filename()
{
	// 需要通过配置文件中的命名格式来命名，暂时没有实现
	std::string str_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
	// 这时候strTime里存放时间的格式是YYYYMMDDTHHMMSS，日期和时间用大写字母T隔开了
	int pos = str_time.find('T');
    str_time.replace(pos,1,std::string(""));
    std::string filename = out_video_stream_file_path + "/" + video_stream_center_name + "/" + filename_prefix + str_time + video_format;
    LOG(INFO)<<"out_video_stream_file = "<<filename;
    return filename;
}


// 通过配置服务器的客户端获取配置文件内容，检查是否发生参数的变化，如果存在变化重新初始化相应的流
// 暂时传入的参数为配置文件的地址（实际上应该是无参数的）

/*
 * 1、输出文件的文件 video_path(存储地址)、write_time(写入时长)、filename_format(文件命名格式)、
 * filename_prefix(文件前缀)、video_format(视频文件格式) 发生改变的时候先存储完当前正在存储的文件，
 * 再去重新初始化新的输出文件流，总结：文件输出流配置更替都是在当前文件完成存储后执行更新操作
 * 
 * 2、转发输出流out_url发生改变时候，立即重新初始化转发输出流文件
 * 
 * 3、输入流的配置发生改变，先重新初始化输入流，然后分别重新初始化转发输出流和文件输出流
 */
bool video_stream_center::check_config(const std::string& all_config_str)
{
	return true;
}


void video_stream_center::run()
{
	int file_index = 0;
	int frame_num,ret;
	//clock_t begin,end;
	//begin = clock();
	boost::posix_time::ptime begin,end;
	boost::posix_time::millisec_posix_time_system_config::time_duration_type time_interval;  
	begin = boost::posix_time::microsec_clock::universal_time();
	cv::Mat image;
	time_error = 0;

	// 提前申请好mat_cache的空间，以防在高速loop中Mat申请和释放空间存在问题
	// 这部分也不能放入init中
    for(uint i=0; i< RAW_SIZE*MULTIPLE; ++i){
        cv::Mat cache(cv::Size(this->mat_width,this->mat_height),CV_8UC3);
        mat_cache.push_back(cache);
    }

	while(true)
	{

#ifdef SAVE_FILE // 判断是否需要存储视频文件
		ret = init_out_stream_file(this->video_config);
		if(ret){
			LOG(INFO)<<"init out_stream_file successfully";
		}else{
			LOG(ERROR)<<"init out_stream_file unsuccessfully";
		}
		frame_num = out_stream_file.get_fps()*60*out_stream_file.get_write_time(); // 计算一个文件要存储的帧数
		file_index = 0;
#endif // SAVE_FILE
		
		while(true)
		{
			std::string self_data_temp = "";

			// mat_cache中选取一个空间
			image = mat_cache[mat_index % (RAW_SIZE*MULTIPLE)];
			if(++mat_index == RAW_SIZE*MULTIPLE) mat_index = 0;

			ret = in_stream.read(image, packet_url);

			// std::cout<<"图片："<<image<<std::endl;
			std::cout<<"packet_url："<<packet_url.size<<std::endl;
			if(!ret){
				LOG(ERROR)<<"fail to read frame from in_stream";
				if(++time_error >= TIME_ERROR_THRESHOLD)
				{
					LOG(ERROR)<<"the time of reading frame unsuccessfully are over threshold\n the center will be inited.";
					time_error = 0;
					init(this->video_config);
					break;
				}
				continue;
			}

			// only for test get_sei_data
			if(ret && 0 != packet_url.size){
				std::cout<<"进来了："<<std::endl;
				bool get_self_data = in_stream.get_sei_data(packet_url, self_data_temp);
				std::cout<<"get_self_data："<<get_self_data<<std::endl;
				if(get_self_data) std::cout<<"得到的私有数据："<<self_data_temp<<std::endl;
				//else std::cout<<"No SEI data"<<std::endl;
			}

#ifdef SAVE_FILE
			// 将裸流包复制一份给文件输出流
			av_packet_ref(&packet_file,&packet_url);
#endif // SAVE_FILE


			json result_json;
			result_json["labels"] = {1,3,4,5,6,10};
			result_json["boxes"][0] = {0.35, 0.25, 0.57, 0.69};
			result_json["boxes"][1] = {0.31, 0.12, 0.63, 0.66};
			result_json["boxes"][2] = {0.45, 0.23, 0.63, 0.43};
			result_json["boxes"][3] = {0.21, 0.12, 0.43, 0.34};
			result_json["boxes"][4] = {0.13, 0.10, 0.32, 0.42};
			result_json["boxes"][5] = {0.12, 0.62, 0.23, 0.74};
			result_json["score"] = {0.87,0.99,0.78,0.89,0.95,0.89};
			std::string result_str = result_json.dump();
			// 写入url
			ret = out_stream_url.write(packet_url,result_str);
			if(!ret){
				LOG(ERROR)<<"write to out_video_stream_url unsuccessfully";
				++time_error;
			}else{
				std::cout<<ret<<" 添加私有流信息成功"<<std::endl;
			}

#ifdef SAVE_FILE 
			// 写入文件
			ret = out_stream_file.write(packet_file,""); //不需要写入SEI时候传入""即可
			if(!ret){
				LOG(ERROR)<<"write to out_video_stream_file unsuccessfully";
				++time_error;
			}
#endif //SAVE_FILE

			// 判断操作次数是否超过阈值
			// if(time_error >= TIME_ERROR_THRESHOLD)
			// {
			// 	time_error = 0;
			// 	this->init(this->video_config);
			// 	break;
			// }

			// 判断是否需要更新配置
			end = boost::posix_time::microsec_clock::universal_time();
			time_interval = end - begin;
			// 得到两个时间间隔的秒数;    
			int sec = time_interval.total_seconds();   
			//std::cout<<sec<<std::endl;
		}
	}
}

// 用于开启子线程
void video_stream_center::start()
{
	boost::function0<void> f = boost::bind(&video_stream_center::run, this);
	boost::thread *thr = new boost::thread(f);
}