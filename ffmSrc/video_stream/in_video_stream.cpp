#include "in_video_stream.h"

in_video_stream::in_video_stream(){
	i_fmt_ctx = NULL;
	stream_url = "";
	is_opened = false;
    start_time = 0;
}


in_video_stream::~in_video_stream(){
    release();
	std::cout<<"~in_video_stream()"<<std::endl;
}


bool in_video_stream::init(const std::string& url, const int mat_height_, const int mat_width_)
{
	i_video_stream = NULL;
	video_index = -1;
	is_opened = false;
	stream_url = url;
	dict = NULL;
	frame_index = 0;
	this->mat_height = mat_height_;
	this->mat_width = mat_width_;
    //取得时间
    start_time = av_gettime();

    //每个使用FFMpeg SDK的工程都必须调用的函数。进行codec和format的注册，然后才能使用。
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

    av_dict_set(&dict, "rtsp_transport", "tcp", 0);
    av_dict_set(&dict, "buffer_size", "10240000", 0);
    av_dict_set(&dict, "stimeout", "2000000", 0);//设置超时2秒

    //输入（Input）
    if ((ret = avformat_open_input(&i_fmt_ctx, stream_url.c_str(), 0, 0)) < 0) {
        std::cout<<"Could not open input file."<<std::endl;
        return false;
    }
    if ((ret = avformat_find_stream_info(i_fmt_ctx, 0)) < 0) {
        std::cout<<"Failed to retrieve input stream information"<<std::endl;
        return false;
    }
    for(int i=0; i<(int)i_fmt_ctx->nb_streams; i++) 
        if(i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_index = i;
            break;
        }
	
    if (video_index == -1){
       std::cout<<"Didn't find a video stream: "<< stream_url <<std::endl;
       return false;
    }

	//set the input stream
    i_video_stream = i_fmt_ctx->streams[video_index];  
    //codec Context
    i_cod_ctx = i_video_stream->codec;

#ifdef SOFTWARE_DECODE  // 软解码
    //获取视频编码格式
    i_codec = avcodec_find_decoder(i_cod_ctx->codec_id);//寻找解码器
    if(i_codec == NULL){// find failed!
    	std::cout<<"Cant't find the decoder" <<std::endl;
        return false;
    }
#endif //SOFTWARE_DECODE

#ifdef HARDWARE_DECODE // 硬解码
    switch (i_cod_ctx->codec_id) {
	case AV_CODEC_ID_H264:
		i_codec = avcodec_find_decoder_by_name("h264_cuvid");//硬解码264
		if (i_codec == NULL) {
			printf("Couldn't find Codec1.\n");
			return -1;
		}
		break;
	case AV_CODEC_ID_MPEG4:
		i_codec = avcodec_find_decoder_by_name("mpeg4_cuvid");//硬解码mpeg4
		if (i_codec == NULL) {
			printf("Couldn't find Codec2.\n");
			return -1;
		}
		break;
	case AV_CODEC_ID_HEVC:
		i_codec = avcodec_find_decoder_by_name("hevc_cuvid");//硬解码265
		if (i_codec == NULL) {
			printf("Couldn't find Codec3.\n");
			return -1;
		}
		break;
	case AV_CODEC_ID_MPEG2VIDEO:
		i_codec = avcodec_find_decoder_by_name("mpeg2_cuvid");//硬解码mpeg2
		if (i_codec == NULL) {
			printf("Couldn't find Codec4.\n");
			return -1;
		}
		break;
	default:
		i_codec = avcodec_find_decoder(i_cod_ctx->codec_id);//软解
		if (i_codec == NULL) {
			printf("Couldn't find Codec.\n");
			return -1;
		}
		break;

	}

#endif // HARDWARE_DECODE

    //用一个编码格式打开一个编码文件
    if(avcodec_open2(i_cod_ctx,i_codec,NULL)){//open decoder
    	std::cout<<"Can't open the decoder" <<std::endl;
        return false;
    }
    av_dump_format(i_fmt_ctx, 0, stream_url.c_str(), 0);

#ifdef GET_MAT // 需要解码成Mat
    h264_frame = av_frame_alloc(); //分配帧存储空间
    rgb_frame = av_frame_alloc(); //存储解码后转换的RGB数据

    // 计算为保存BGR所需要的空间，字节数 ，opencv中是按BGR来保存的
    pix_size = avpicture_get_size(AV_PIX_FMT_BGR24,this->mat_width,this->mat_height);
    out_buffer = (uint8_t *)av_malloc(pix_size);
    //已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间:out_buffer, data's value
    avpicture_fill((AVPicture *)rgb_frame,out_buffer,AV_PIX_FMT_BGR24,this->mat_width,this->mat_height);

    //Allocate and return an SwsContext;
    img_convert_ctx = sws_getContext(i_cod_ctx->width,i_cod_ctx->height,i_cod_ctx->pix_fmt,
                                    this->mat_width,this->mat_height,AV_PIX_FMT_BGR24,
                                    SWS_BICUBIC,NULL,NULL,NULL);
#endif // GET_MAT

    is_opened = true;//修改默认值
    return true;
}

bool in_video_stream::release()
{
#ifdef GET_MAT 
	sws_freeContext(img_convert_ctx);
    av_frame_free(&rgb_frame);  
    av_frame_free(&h264_frame);
    av_free(out_buffer);
#endif // GET_MAT
    avcodec_close(i_cod_ctx);  
    avformat_close_input(&i_fmt_ctx);
    is_opened = false;
    return true;  
}

bool in_video_stream::get_is_opened()
{
	return is_opened;
}

std::string in_video_stream::get_stream_url()
{
	return stream_url;
}

bool in_video_stream::set_stream_url(const std::string& url, const int mat_height_, const int mat_width_)
{
	release();
	return init(url, mat_height_, mat_width_);
}

bool in_video_stream::read(cv::Mat& image, AVPacket& packet)
{
    //start_time = av_gettime();
	int ret = 0;
	bool valid = false;
    int count_errs = 0;
    const int max_number_of_attempts = 1 << 9;
    int got_picture = 0;
    while(!valid)
    {
    	//av_packet_unref(&packet); // set packet.data=NULL; packet.size=NULL;
    	ret = av_read_frame(i_fmt_ctx, &packet);
    	if(ret < 0) return false;
        if(packet.stream_index != this->video_index){
        	av_packet_unref(&packet);//release the none video's packet
            if (++count_errs > max_number_of_attempts)
                return false;
            continue;
      	}

      	//FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if(packet.pts == AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1 = i_fmt_ctx->streams[video_index]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration = (double)AV_TIME_BASE/av_q2d(i_fmt_ctx->streams[video_index]->r_frame_rate);
            //Parameters
            packet.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            packet.dts = packet.pts;
            packet.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }


#ifdef INPUT_VIDEO_FILE
        AVRational time_base = i_fmt_ctx->streams[video_index]->time_base;
        AVRational time_base_q = {1,AV_TIME_BASE};
        // 计算视频播放时间
        int64_t pts_time = av_rescale_q(packet.dts, time_base, time_base_q);
        // 计算实际视频的播放时间
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time) {
            // 睡眠一段时间（目的是让当前视频记录的播放时间与实际时间同步）
            av_usleep((unsigned int)(pts_time - now_time));
        }
#endif //INPUT_VIDEO_FILE

        packet.pos = -1;
        valid = true;

#ifdef GET_MAT // 需要解码成图片
        ret = avcodec_decode_video2(i_cod_ctx,h264_frame,&got_picture,&packet);
      	if(ret <0){
        	std::cout<<"Decode Error."<< stream_url <<std::endl;
        	return false;
      	}
      	if(got_picture){     
        	valid = true;            
      	}else{
          count_errs++;
          if (count_errs > max_number_of_attempts)
            return false;
        }
#endif // GET_MAT
        
    }

#ifdef GET_MAT // 需要解码成图片
    if(valid){
        //转换像素的函数 
        sws_scale(img_convert_ctx,(const uint8_t* const *)h264_frame->data,h264_frame->linesize,0,
                    i_cod_ctx->height,rgb_frame->data,rgb_frame->linesize);
        // 循环使用已开启的空间
        //image = mat_cache[frame_index%(RAW_SIZE*MULTIPLE)];
        memcpy(image.data,out_buffer,pix_size);
        if(packet.stream_index == video_index){
            frame_index++;
        }
    }
#endif // GET_MAT

    return valid;
}


bool in_video_stream::get_sei_data(const AVPacket& packet, std::string &self_data)
{
    char buffer[10240] = {0};
    int len_buffer = 10240;
    int ret = -1;
    try{
        ret = get_sei_content(packet.data,packet.size,buffer,&len_buffer);
		
        std::cout<<"进来了get_sei_data："<<ret<<std::endl;

    }catch(...){
        return false;
    }
    
    if(-1 != ret){
        self_data = buffer;
        return true;
    }
    return false;
}


int in_video_stream::get_fps()
{
	double  frame_rate = 0;
    AVStream *stream = this->i_video_stream;
    if(stream->r_frame_rate.den > 0){
        frame_rate = stream->r_frame_rate.num/stream->r_frame_rate.den;
    }else if(stream->codec->framerate.den > 0){
        frame_rate = stream->codec->framerate.num/stream->codec->framerate.den;
    }
    return (int)frame_rate;
}

AVFormatContext* in_video_stream::get_in_video_stream_info()
{
	return this->i_fmt_ctx;
}

#ifdef GET_MAT // 需要解码
void in_video_stream::set_mat_size(const int mat_height_, const int mat_width_)
{
	// 更新参数
	this->mat_height = mat_height_;
	this->mat_width = mat_width_;
	
	// 释放相关的内存
	sws_freeContext(img_convert_ctx);
    av_frame_free(&rgb_frame);  
    av_frame_free(&h264_frame);
    av_free(out_buffer);

    // 重新分配内存
    h264_frame = av_frame_alloc(); //分配帧存储空间
    rgb_frame = av_frame_alloc(); //存储解码后转换的RGB数据
    // 计算为保存BGR所需要的空间，字节数 ，opencv中是按BGR来保存的
    pix_size = avpicture_get_size(AV_PIX_FMT_BGR24,this->mat_width,this->mat_height);
    out_buffer = (uint8_t *)av_malloc(pix_size);
    //已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间:out_buffer, data's value
    avpicture_fill((AVPicture *)rgb_frame,out_buffer,AV_PIX_FMT_BGR24,this->mat_width,this->mat_height);
    //Allocate and return an SwsContext;
    img_convert_ctx = sws_getContext(i_cod_ctx->width,i_cod_ctx->height,i_cod_ctx->pix_fmt,
                                    this->mat_width,this->mat_height,AV_PIX_FMT_BGR24,
                                    SWS_BICUBIC,NULL,NULL,NULL);

}

void in_video_stream::get_mat_size(int &mat_height_,int &mat_width_)
{
	mat_height_ = this->mat_height;
	mat_width_ = this->mat_width;
}
#endif // GET_MAT