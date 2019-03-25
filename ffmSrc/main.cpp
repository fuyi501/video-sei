#include <vector>
#include "video_center/video_stream_center.h"

std::string read_config(const std::string &confpath)
{
    LOG(INFO)<<"打开配置文件";
    std::ifstream in;
    // std::cout<<confpath<<std::endl;
    in.open(confpath);
    if(!in)
    {
        LOG(ERROR)<<"config file open failed!";
        return "null";
    }
    std::ostringstream buf;
    char ch;
    while(buf&&in.get(ch))
    {
        buf.put(ch);
    }
    std::string conf_j = buf.str();
    in.close();
    LOG(INFO)<<"文件关闭";
    return conf_j;
}

void init_log(const std::string& config)
{
    json config_json = json::parse(config);
    std::string log_error_format = config_json["video_stream_center_common"]["log_error_format"].get<std::string>();
    std::string log_info_format = config_json["video_stream_center_common"]["log_info_format"].get<std::string>();
    google::InitGoogleLogging("");
    google::SetLogDestination(google::GLOG_INFO, log_info_format.c_str());
    google::SetLogDestination(google::GLOG_ERROR, log_error_format.c_str());
    google::InstallFailureSignalHandler(); 
    std::cout<<"init glog ok!"<<std::endl;
}

int main()
{
    std::string config_str = read_config("./video_stream.config");
	json config_json = json::parse(config_str);

    std::vector<json> v_video_center_json = config_json["video_stream_center"];
    
    for(auto iter = v_video_center_json.begin(); iter != v_video_center_json.end(); ++iter){
        std::string video_center_config = (*iter).dump();
        video_stream_center *video_center = new video_stream_center();
        video_center->init(video_center_config);
        video_center->start();
    }
    while(true){
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    }	
	return 0;
}
