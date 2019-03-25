#include <vector>
#include "readstream.h"

std::string read_config(const std::string &confpath)
{
    std::cout<<"打开配置文件";
    std::ifstream in;
    // std::cout<<confpath<<std::endl;
    in.open(confpath);
    if(!in)
    {
        std::cout<<"config file open failed!";
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
    std::cout<<"文件关闭";
    return conf_j;
}

int main()
{
    std::string config_str = read_config("./video_stream.config");
	json config_json = json::parse(config_str);
    cv::Mat image;
    std::string self_data;
    std::vector<json> v_video_center_json = config_json["video_stream_center"];
    std::string video_center_config;
    for(auto iter = v_video_center_json.begin(); iter != v_video_center_json.end(); ++iter){
        video_center_config = (*iter).dump();
        std::cout<< video_center_config << std::endl;
    }
    read_stream *readstream = new read_stream();
    readstream->init(video_center_config);
    readstream->startrun();
    while(true){
        self_data = readstream->run(image);
        std::cout << "私有数据：" << self_data << std::endl;
    }
    
	return 0;
}


