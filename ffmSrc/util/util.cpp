#include "util.h"
#include <glog/logging.h>
namespace util_tools
{

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

}