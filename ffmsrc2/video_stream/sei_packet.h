#ifndef SEI_PACKET_H
#define SEI_PACKET_H
#include <stdint.h>
#include <iostream>

/**
 * @brief      { 大端格式转小端格式 }
 *
 * @param[in]  value  大端格式数值
 *
 * @return     { 小端格式数值 }
 */
uint32_t reversebytes(uint32_t value);

/**
 * @brief      获取不含start_code的NAL的长度
 *
 * @param[in]  content  不加上UUID的纯自定义数据的长度
 *
 * @return     不含start_code的NAL的长度
 */
uint32_t get_sei_nalu_size(uint32_t content);

/**
 * @brief      返回一个包含start_code的完成NAL包长度
 *
 * @param[in]  size  不含start_code的NAL的长度
 *
 * @return     完整的SEI数据NAL包
 */
uint32_t get_sei_packet_size(uint32_t size);

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
int fill_sei_packet(unsigned char * packet, bool isAnnexb, const char * content, uint32_t size);

/**
 * @brief      Gets the sei buffer.
 *
 * @param      data    去除了start code和 NAL header的数据（明确是SEI的NAL数据）
 * @param[in]  size    所有NAL整体的长度
 * @param      buffer  提取的未注册数据数据
 * @param      count   最终得到未注册数据数据长度
 *
 * @return     The sei buffer.
 */
int get_sei_buffer(unsigned char * data, uint32_t size, char * buffer, int *count);


/**
 * @brief      Gets the sei content.
 *
 * @param      packet  未去除start code的NAL数据
 * @param[in]  size    所有NAL整体的长度
 * @param      buffer  提取的未注册数据数据
 * @param      count   最终得到未注册数据数据长度（输入的数值应该为申请buffer的大小）
 *
 * @return     The sei content.
 */
int get_sei_content(unsigned char * packet, uint32_t size, char * buffer, int *count);


#endif // SEI_PACKET


// 可供参考的网站
// SEI讲解：
// https://blog.csdn.net/y601500359/article/details/80943990
// 代码：
// https://blog.csdn.net/ab7936573/article/details/74135909#commentBox