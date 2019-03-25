#include "sei_packet.h"  
#include <stdio.h>  
#include <string.h>  
  
#define min(X,Y) ((X) < (Y) ? (X) : (Y))  
  
#define UUID_SIZE 16  
  
//FFMPEG uuid  
//static unsigned char uuid[] = { 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef };  
//self UUID  
// unsigned char (0,255)
static unsigned char uuid[] = { 0x54, 0x80, 0x83, 0x97, 0xf0, 0x23, 0x47, 0x4b, 0xb7, 0xf7, 0x4f, 0x32, 0xb5, 0x4e, 0x06, 0xac };  
  
//开始码  
static unsigned char start_code[] = {0x00,0x00,0x00,0x01};  
  

// 大端模式==>小端模式
uint32_t reversebytes(uint32_t value) {  
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |  
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;  
}  

// 获取不含start_code的NAL的长度
// typedef unsigned int  uint32_t;
uint32_t get_sei_nalu_size(uint32_t content)  // content指的是不加上UUID的纯自定义数据的长度
{  
    //SEI payload size  
    uint32_t sei_payload_size = content + UUID_SIZE;  
    // NALU + payload类型 + 数据长度 + 数据  (与代码顺序一一对应)
    uint32_t sei_size = 1 + 1 + (sei_payload_size / 0xFF + (sei_payload_size % 0xFF != 0 ? 1 : 0)) + sei_payload_size;  
    //截止码  
    uint32_t tail_size = 2;  
    if (sei_size % 2 == 1)  
    {  
        tail_size -= 1;  
    }  
    sei_size += tail_size;  
  
    return sei_size;  
}  

// 返回一个包含start_code的完成NAL包长度
uint32_t get_sei_packet_size(uint32_t size)  
{  
    return get_sei_nalu_size(size) + 4; // 4指的是start_code的长度 
}  
  

/**
 * @brief      { 得到一个完整SEI数据包 }
 *
 * @param      packet    最终得到的完整SEI数据包
 * @param[in]  isAnnexb  h264的一种封装格式
 * @param[in]  content   用户未注册的数据
 * @param[in]  size      用户未注册的数据的长度
 *
 * @return     { 函数操作是否成功 }
 */
int fill_sei_packet(unsigned char * packet,bool isAnnexb, const char * content, uint32_t size)  
{  
    // size指的是不加上UUID的纯自定义数据的长度
    unsigned char * data = (unsigned char*)packet;  
    unsigned int nalu_size = (unsigned int)get_sei_nalu_size(size);  
    uint32_t sei_size = nalu_size;  // 备份？
    //大端转小端  
    nalu_size = reversebytes(nalu_size);  
  
    //NALU开始码  
    unsigned int * size_ptr = &nalu_size;  
    if (isAnnexb)  
    {  
        memcpy(data, start_code, sizeof(unsigned int));  
    }  
    else  
    {  
        memcpy(data, size_ptr, sizeof(unsigned int));  
    }  
    data += sizeof(unsigned int); //将data的地址向后移动到存放NAL header的位置  
  
    unsigned char * sei = data;  
    //NAL header  
    *data++ = 6; //SEI  
    //sei payload type  
    *data++ = 5; //unregister （表示SEI里面存放的是自定义数据）
    size_t sei_payload_size = size + UUID_SIZE;  

    //设定SEI负载数据长度  
    while (true)  
    {  
        *data++ = (sei_payload_size >= 0xFF ? 0xFF : (char)sei_payload_size);  
        if (sei_payload_size < 0xFF) break;  
        sei_payload_size -= 0xFF;  
    }  
  
    //UUID  
    memcpy(data, uuid, UUID_SIZE);  
    data += UUID_SIZE;  
    //数据  
    memcpy(data, content, size);  
    data += size;  
  
    //tail 截止对齐码  
    if (sei + sei_size - data == 1)  
    {  
        *data = 0x80;  
    }  
    else if (sei + sei_size - data == 2)  
    {  
        *data++ = 0x00;  
        *data++ = 0x80;  
    }  
  
    return true;  
}  


/**
 * @brief      Gets the sei buffer.
 *
 * @param      data    去除了start code和 NAL header的数据（明确是SEI的NAL数据）
 * @param[in]  size    ？？？
 * @param      buffer  提取的未注册数据数据
 * @param      count   最终得到未注册数据数据长度
 *
 * @return     The sei buffer.
 */
int get_sei_buffer(unsigned char * data, uint32_t size, char * buffer, int *count)  
{  
    unsigned char * sei = data;  
    int sei_type = 0;  
    unsigned sei_size = 0;  
    // SEI payload type  
    do {  
        sei_type += *sei;  
    } while (*sei++ == 255);  // ？？？
    // 数据长度  
    do {  
        sei_size += *sei;  
    } while (*sei++ == 255);  // ？？？
  
    // 检查UUID  
    // 此时的sei_size为UUID+未注册数据的长度
    if (sei_size >= UUID_SIZE && sei_size <= (data + size - sei) &&  
        sei_type == 5 && memcmp(sei, uuid, UUID_SIZE) == 0)  
    {  
        sei += UUID_SIZE;  // 将地址移动到未注册数据首地址
        sei_size -= UUID_SIZE;  // 未注册数据长度
  
        if (buffer != NULL && count != NULL)  
        {  
            if (*count > (int)sei_size)  
            {  
                memcpy(buffer, sei, sei_size);  
            }  
        }  
        if (count != NULL)  
        {  
            *count = sei_size;  
        }  
        return sei_size;  
    }  
    return -1;  
}  


/**
 * @brief      Gets the sei content.
 *
 * @param      packet  未去除start code的NAL数据
 * @param[in]  size    NAL整体的长度
 * @param      buffer  提取的未注册数据数据
 * @param      count   最终得到未注册数据数据长度
 *
 * @return     The sei content.
 */
int get_sei_content(unsigned char * packet, uint32_t size,char * buffer,int *count)  
{  
    if(NULL == packet || 0 == size) return -1;
    unsigned char ANNEXB_CODE_LOW[] = { 0x00,0x00,0x00,0x01 };  
    unsigned char ANNEXB_CODE[] = { 0x00,0x00,0x00,0x01 };  
  
    unsigned char *data = packet;  // data为packet指针
    bool isAnnexb = false;
    // 判断是否为Annexb封装模式  
    if ((size > 3 && memcmp(data, ANNEXB_CODE_LOW,3) == 0) ||  
        (size > 4 && memcmp(data, ANNEXB_CODE,4) == 0)  
        )  
    {  
        isAnnexb = true;
    }else{
        return -1;
    }

    //暂时只处理MP4封装,annexb暂未处理  ？？？
    if (isAnnexb)  
    {  
        while(data < packet + size - 1){  
            if ((packet + size - data) > 4 && data[0] == 0x00 && data[1] == 0x00)  
            {   // 可能是含有start code的数据
                int startCodeSize = 2;  
                if(data[2] == 0x01){  
                    startCodeSize = 3;
                    //std::cout<<"startCodeSize = 3"<<std::endl;  
                }  
                else if(data[2] == 0x00 && data[3] == 0x01){  
                    startCodeSize = 4;
                    //std::cout<<"startCodeSize = 4"<<std::endl;  
                }
                
                if (startCodeSize == 3 || startCodeSize == 4){  
                    if ((packet + size - data) > (startCodeSize + 1) &&   
                        (data[startCodeSize] & 0x1F) == 6)  
                    {  
                        //SEI  
                        unsigned char * sei = data + startCodeSize + 1;  
                        // sei表示当前的地址
                        int ret = get_sei_buffer(sei, (packet + size - sei), buffer, count);
                        if (ret != -1){   
                            return ret;  
                        }else{ // SEI数据被人为的放在最前面，要是第一个没有取得到就直接返回失败   
                            return ret;  
                        } 
                    }

                }

                if((packet + size - data) > (startCodeSize + 1)){
                    data += startCodeSize + 1;
                }else if(data + 1 < packet + size){
                    ++data;
                }else{
                    return -1;
                }  

            }else{  // 不是含有start code的数据
                if(data + 1 < packet + size)
                    ++data;
                else return -1;  
            }  
        }  
    }  
    else{  
        //当前NALU  
        while(data < packet + size - 1){  
            //MP4格式起始码/长度  
            unsigned int *length = (unsigned int *)data;  
            int nalu_size = (int)reversebytes(*length);  
            //NALU header  
            if ((*(data + 4) & 0x1F) == 6)  
            {  
                //SEI
                unsigned char * sei = data + 4 + 1;  
                int ret = get_sei_buffer(sei, min(nalu_size,(packet + size - sei)),buffer,count);  
                if (ret != -1)  
                {  
                    return ret;  
                }  
            }  
            data += 4 + nalu_size;  
        }  
    }  
    return -1;  
}  


// h264有两种封装，
// 一种是annexb模式，传统模式，有startcode，SPS和PPS是在ES中
// 一种是mp4模式，一般mp4 mkv会有，没有startcode，SPS和PPS以及其它信息被封装在container中，每一个frame前面是这个frame的长度