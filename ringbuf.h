/*
 * MIT License
 *
 * Copyright (c) 2024 siyun.chen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _RINGBUF_H
#define _RINGBUF_H

#include <stdint.h>
#include <stdbool.h>

#ifndef ringbuf_malloc
  #include <stdlib.h>
  #define ringbuf_malloc malloc // 定义malloc函数
#endif //ringbuf_malloc

#ifndef ringbuf_free
  #define ringbuf_free   free   // 定义free函数
#endif //ringbuf_free

/**
 * @brief  环形数组对象
 */
typedef struct
{
  uint8_t *arr;   // 环形数组内存指针
  uint16_t size;  // 环形数组大小
  uint16_t used;  // 已使用长度
  uint16_t front; // 读指针，始终代表已使用内存的第一个位置索引
  uint16_t rear;  // 写指针，始终代表未使用内存的第一个位置索引
} ringbuf_t;

/**
 * @brief  创建静态环形数组
 * @param  obj:  环形数组对象
 * @param  arr:  环形数组内存指针
 * @param  size: 环形数组大小
 * @return 是否创建成功
 */
bool ringbuf_init(ringbuf_t *obj, uint8_t *arr, uint16_t size);

/**
 * @brief  删除静态环形数组
 * @param  obj: 环形数组对象
 * @return 无
 */
void ringbuf_deinit(ringbuf_t *obj);

/**
 * @brief  创建动态环形数组
 * @param  size: 环形数组大小
 * @return 环形数组对象指针，创建失败返回NULL
 */
ringbuf_t *ringbuf_create(uint16_t size);

/**
 * @brief  删除动态环形数组
 * @param  obj: 环形数组对象
 * @return 无
 */
void ringbuf_delete(ringbuf_t *obj);

/**
 * @brief  获取环形数组已使用长度
 * @param  obj: 环形数组对象
 * @return 已使用长度
 */
uint16_t ringbuf_getUsedLength(ringbuf_t *obj);

/**
 * @brief  判断环形数组是否为空
 * @param  obj: 环形数组对象
 * @return 环形数组是否为空
 */
bool ringbuf_isEmpty(ringbuf_t *obj);

/**
 * @brief  判断环形数组是否已满
 * @param  obj: 环形数组对象
 * @return 环形数组是否已满
 */
bool ringbuf_isFull(ringbuf_t *obj);

/**
 * @brief  向环形数组写入数据
 * @param  obj:    环形数组对象
 * @param  pdata:  待写入数据的缓冲区指针
 * @param  length: 待写入数据的长度，如果大于剩余空间将丢弃剩余的数据
 * @return 实际写入长度
 */
uint16_t ringbuf_write(ringbuf_t *obj, uint8_t *pdata, uint16_t length);

/**
 * @brief  强制向环形数组写入数据
 * @param  obj    环形数组对象
 * @param  pdata  待写入数据的缓冲区指针
 * @param  length 待写入数据的长度，如果剩余空间不足将覆盖原有数据
 * @return 实际写入长度
 */
uint16_t ringbuf_writeForce(ringbuf_t *obj, uint8_t *pdata, uint16_t length);

/**
 * @brief  从环形数组读出数据
 * @note   读出数据的同时会从环形数组中删除数据。
 * @param  obj    环形数组对象
 * @param  pdata  接收数据的缓冲区指针
 * @param  length 要读取的数据长度，如果length大于已有数据数量将只读出已有数据数量
 * @return 实际读取长度
 */
uint16_t ringbuf_read(ringbuf_t *obj, uint8_t *pdata, uint16_t length);

/**
 * @brief  移除环形数组中指定长度的数据
 * @note   主要用来移除不需要的数据，当参数length为正时从前往后移除（相当于FIFO），若为负时从后
 *         往前移除（相当于LIFO）。
 * @param  obj:    环形数组对象
 * @param  length: 要移除的数据长度
 * @return 实际移除长度
 */
uint16_t ringbuf_remove(ringbuf_t *obj, int length);

/**
 * @brief  批量修改环形数组中指定索引范围内已存在的数据。仅修改数据，不是写入数据。
 * @note   索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能修改已写入的数据，不是写入数据。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的起始索引
 * @param  length 要修改数据的长度
 * @param  pdata  要修改数据的指针
 * @return 修改操作是否成功
 */
bool ringbuf_modify(ringbuf_t *obj, uint16_t index, uint16_t length, uint8_t *pdata);

/**
 * @brief  窥视环形数组中指定索引范围内已存在的数据。仅读取数据，不会移除数据。
 * @note   索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能读取已写入的数据且只读不删。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的起始索引
 * @param  length 要读取的数据长度
 * @param  pdata  用于存储读取数据的缓冲区指针
 * @return 读取操作是否成功
 */
bool ringbuf_peek(ringbuf_t *obj, uint16_t index, uint16_t length, uint8_t *pdata);

/**
 * @brief  更新环形数组读写指针
 * @note   一般用于数据从其它地方直接缓存到环形数组内存(比如DMA)，此时需要手动的同步改变读写指针。
 *         函数会对输入参数合法性进行校验，但由于front、rear与used是直接赋值给环形数组对象，使用
 *         时请务必小心。
 * @param  obj:   环形数组对象
 * @param  front: 新的front指针位置
 * @param  rear:  新的rear指针位置
 * @param  used:  新的已使用长度
 * @return 是否更新成功
 */
bool ringbuf_updatePointer(ringbuf_t *obj, uint16_t front, uint16_t rear, uint16_t used);

/**
 * @brief  向环形数组指定索引处开始填充指定值，最大可填充整个环形数组。(C库 memset函数)
 * @note   注意此函数仅填充数据不会改变环形数组信息(读写指针、used等)。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的索引
 * @param  value  要填充的值
 * @param  length 要填充的长度
 * @return 实际填充长度
 */
uint16_t ringbuf_memset(ringbuf_t *obj, uint16_t index, uint8_t value, uint16_t length);

/**
 * @brief  从环形数组指定索引范围内已存在的数据中获取字符串长度。(C库 strlen函数)
 * @param  obj   环形数组对象
 * @param  index 相对读指针的索引
 * @return 字符串长度
 */
uint16_t ringbuf_strlen(ringbuf_t *obj, uint16_t index);

/**
 * @brief  从环形数组指定索引范围内已存在的数据中查找字符并返回相对索引。(C库 strchar函数)
 * @param  obj   环形数组对象
 * @param  index 相对读指针的索引
 * @param  ch    要查找的字符
 * @return 匹配字符相对读指针的索引，如果找不到则返回 -1
 */
int ringbuf_strchr(ringbuf_t *obj, uint16_t index, uint8_t ch);

/**
 * @brief  从环形数组指定索引范围内已存在的数据中查找子字符串并返回相对索引。(C库 strstr函数)
 * @param  obj       环形数组对象
 * @param  index     相对读指针的索引
 * @param  substring 要查找的子字符串
 * @return 子字符串相对读指针的索引，如果未找到则返回 -1
 */
int ringbuf_strstr(ringbuf_t *obj, uint16_t index, const char *substring);

#endif //_RINGBUF_H
