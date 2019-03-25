#include "out_video_stream.h"

out_video_stream::out_video_stream(){
	stream_url = "";
	o_fmt_ctx = NULL;
	i_fmt_ctx = NULL;
	i_fmt_ctx_audio = NULL;
	fps = -1; //帧率的默认值
	is_opened = false;
	dict = NULL;

}

out_video_stream::~out_video_stream(){
	release();
	std::cout<<"~out_video_stream()"<<std::endl;
}

bool out_video_stream::init(const std::string& url, AVFormatContext* i_fmt_ctx, const bool is_file, const int minute, const int fps)
{
	int ret = -1;
	//每个使用FFMpeg SDK的工程都必须调用的函数。进行codec和format的注册，然后才能使用。
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
    av_dict_set(&dict, "rtsp_transport", "tcp", 0);
    av_dict_set(&dict, "buffer_size", "1024000", 0);
    av_dict_set(&dict, "stimeout", "2000000", 0);//设置超时2秒
	
	this->i_fmt_ctx = i_fmt_ctx;
	this->stream_url = url;
	this->is_file = is_file;
	this->fps = fps;
	this->write_time = minute;

	
	AVOutputFormat *ofmt = NULL;
	std::cout << "open save video file: " << stream_url <<std::endl;
	if(is_file){
        avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, stream_url.c_str());
    }else{
        avformat_alloc_output_context2(&o_fmt_ctx, NULL, "mpegts", stream_url.c_str());//UDP
    } 
	if(!&o_fmt_ctx){
		std::cout<<"avformat_alloc_output_context2 ERROR!!!\n"<<"stream_url.c_str() = "<<stream_url.c_str()<<std::endl;
		return false;	
	} 
	ofmt = o_fmt_ctx->oformat;

	//根据视频输入流创建输出流（Create output AVStream according to input AVStream）
	for(uint i = 0; i < (int)i_fmt_ctx->nb_streams; i++)
	{
		if(i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVStream *in_stream = i_fmt_ctx->streams[i];
			AVStream *out_stream = avformat_new_stream(o_fmt_ctx, in_stream->codec->codec);
			out_stream->time_base = in_stream->time_base;
			if (!out_stream) {
	            std::cout<<"Failed allocating output stream"<<std::endl;
	            return false;
	        }
	        //复制AVCodecContext的设置（Copy the settings of AVCodecContext）
	        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
	        if (ret < 0) {
	            std::cout<<"Failed to copy context from input to output stream codec context"<<std::endl;
	            return false;
	        }
	        out_stream->codec->codec_tag = 0;
	        if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	        break;
		}
	}

    std::cout<<"打开输出 "<<std::endl;


	//打印输出流信息
    av_dump_format(o_fmt_ctx, 0, stream_url.c_str(), 1);

    //打开输出URL（Open output URL）
    if (!(ofmt->flags & AVFMT_NOFILE)){
        ret = avio_open(&o_fmt_ctx->pb, stream_url.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0){
            std::cout<<"Could not open output URL: "<<stream_url.c_str()<<std::endl;
            return false;
        }else{
            std::cout<<"打开输出 url: "<<stream_url.c_str()<<std::endl;

		}
    }
    //写文件头（Write file header）
    ret = avformat_write_header(o_fmt_ctx, NULL);
    if (ret < 0) {
        std::cout<<"Error occurred when opening output URL"<<std::endl;
        return false;
    }

    this->is_opened = true;
	return true;
}




bool out_video_stream::release()
{
	//写文件尾（Write file trailer）
    av_write_trailer(o_fmt_ctx);
    avcodec_close(o_fmt_ctx->streams[0]->codec);
    av_freep(&o_fmt_ctx->streams[0]->codec);
    av_freep(&o_fmt_ctx->streams[0]);
    avio_close(o_fmt_ctx->pb);
    av_free(o_fmt_ctx);
    is_opened = false;
	return true;
}


bool out_video_stream::get_is_opened()
{
	return is_opened;
}

std::string out_video_stream::get_stream_url()
{
	return stream_url;
}

// 只是输出流的url或者写入时间发生改变后调用该函数
// 当输入流的配置发生改变的时候需要直接调用init函数了，并且需要等待输入流的重新初始化完毕
bool out_video_stream::set_stream_url(const std::string& url)
{
	release();
	return init(url, this->i_fmt_ctx, this->is_file, this->write_time, this->fps);
}

int out_video_stream::get_fps()
{
	return fps;
}

void out_video_stream::set_write_time(const int minute){
	this->write_time = minute;
}

int out_video_stream::get_write_time(){ return write_time; }

bool out_video_stream::write(AVPacket& cur_packet,const std::string& self_info)
{
	int k = 0;
	if(is_file){
		k = av_interleaved_write_frame(o_fmt_ctx,&cur_packet);//一定最后在写到磁盘，因为写后，函数会自动释放空间
		if(k != 0) std::cout<<"push video stream k = "<<k<<std::endl;
	}else{
		// 添加私有流信息
		std::cout<<"添加私有流信息"<<std::endl;
		if("" == self_info){ // 默认""不需要添加SEI信息
			std::cout<<"私有数据为空，不需要添加私有流信息"<<std::endl;
			k = av_interleaved_write_frame(o_fmt_ctx,&cur_packet);
			if(k != 0) std::cout<<"push video stream k = "<<k<<std::endl;
			return (0 == k)?true:false;
		}
		int len_self_info = self_info.size();
		char *content = new char[len_self_info + 1]; //需要释放
		strcpy(content, self_info.c_str());
		uint32_t len_sei_packet = get_sei_packet_size(len_self_info);
		unsigned char *sei_packet = new unsigned char[len_sei_packet]; //需要释放
		fill_sei_packet(sei_packet, true, content, (uint32_t)len_self_info);
		uint8_t *new_data = new uint8_t[cur_packet.size + len_sei_packet];
		memcpy(new_data, sei_packet, len_sei_packet); // 复制SEI内容
		memcpy(new_data+len_sei_packet, cur_packet.data, cur_packet.size); // 复制原packet.data内容
		int cur_packet_size = cur_packet.size + len_sei_packet;
		cur_packet.data = NULL; // packet data will be allocated by the encoder
		cur_packet.data = new_data;
		cur_packet.size = cur_packet_size;
		// 写入视频流
		k = av_interleaved_write_frame(o_fmt_ctx,&cur_packet);
		if(k != 0) std::cout<<"push video stream k = "<<k<<std::endl;
		if(nullptr != content){ // 释放content
			//std::cout<<"delete content"<<std::endl;
			delete[] content;
			content = nullptr;
		}
		if(nullptr != sei_packet){ // 释放sei_packet
			//std::cout<<"delete sei_packet"<<std::endl;
			delete[] sei_packet;
			sei_packet = nullptr;
		}
		if(nullptr != new_data){
			//std::cout<<"delete new_data"<<std::endl;
			delete[] new_data;
			new_data = nullptr;
		}
	}
	
	std::cout<<"k 为 0 则添加私有流信息成功"<<k<<std::endl;

    return (0 == k)?true:false;
}
/*
	self_defined_info  是json格式的string数据，里面包含了检测后的一些信息
	在传输私有流信息的时候为私有信息添加一个字段frame_index 用于辅助同步视频流和私有流
	self_defined_info'format:
	{
		"frame_index":12,
		"object":[
			{
				"class":"car",
				"xmin":123,
				"ymin":153,
				"xmax":155,
				"ymax":456,
				"score":0.86
			},
			{
				"class":"car",
				"xmin":123,
				"ymin":153,
				"xmax":155,
				"ymax":456,
				"score":0.86
			}
		]
	}

*/





