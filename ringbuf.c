/*
 * MIT License
 *
 * Copyright (c) 2023-2024 csy(siyun.chen@foxmail.com)
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
#include "ringbuf.h"

/**
 * @brief  创建静态环形数组
 * @param  obj:  环形数组对象
 * @param  arr:  环形数组内存指针
 * @param  size: 环形数组大小
 * @return 是否创建成功
 */
bool ringbuf_init(ringbuf_t *obj, uint8_t *arr, uint16_t size)
{
    if ((obj == NULL) || (arr == NULL) || (size <= 0))
    {
        return false;
    }
    obj->arr = arr;
    obj->size = size;
    obj->used = 0;
    obj->front = 0;
    obj->rear = 0;
    return true;
}

/**
 * @brief  删除静态环形数组
 * @param  obj: 环形数组对象
 * @return 无
 */
void ringbuf_deinit(ringbuf_t *obj)
{
    if (obj == NULL)
        return;
    obj->arr = NULL;
    obj->size = 0;
    obj->used = 0;
    obj->front = 0;
    obj->rear = 0;
}

/**
 * @brief  创建动态环形数组
 * @param  size: 环形数组大小
 * @return 环形数组对象指针，创建失败返回NULL
 */
ringbuf_t *ringbuf_create(uint16_t size)
{
    if (size <= 0)
    {
        return NULL;
    }
    ringbuf_t *obj = (ringbuf_t *)ringbuf_malloc(sizeof(ringbuf_t));
    if (obj == NULL)
    {
        return NULL;
    }
    obj->arr = (uint8_t *)ringbuf_malloc(sizeof(uint8_t) * size);
    if (obj->arr == NULL)
    {
        ringbuf_free(obj);
        return NULL;
    }
    obj->size = size;
    obj->used = 0;
    obj->front = 0;
    obj->rear = 0;
    return obj;
}

/**
 * @brief  删除动态环形数组
 * @param  obj: 环形数组对象
 * @return 无
 */
void ringbuf_delete(ringbuf_t *obj)
{
    if (obj == NULL)
        return;
    if (obj->arr != NULL)
        ringbuf_free(obj->arr);
    ringbuf_free(obj);
}

/**
 * @brief  获取环形数组已使用长度
 * @param  obj: 环形数组对象
 * @return 已使用长度
 */
uint16_t ringbuf_getUsedLength(ringbuf_t *obj)
{
    if (obj == NULL)
        return 0;
    return obj->used;
}

/**
 * @brief  判断环形数组是否为空
 * @param  obj: 环形数组对象
 * @return 环形数组是否为空
 */
bool ringbuf_isEmpty(ringbuf_t *obj)
{
    if (obj == NULL)
        return true;
    return obj->used == 0;
}

/**
 * @brief  判断环形数组是否已满
 * @param  obj: 环形数组对象
 * @return 环形数组是否已满
 */
bool ringbuf_isFull(ringbuf_t *obj)
{
    if (obj == NULL)
        return false;
    return obj->used >= obj->size;
}

/**
 * @brief  向环形数组写入数据，支持覆盖已有数据的强制写入模式
 * 
 * 此函数将指定的数据写入环形数组。如果选择了强制写入模式并且缓冲区剩余空间不足，
 * 则会覆盖缓冲区中的旧数据。如果尝试写入的数据量超过了缓冲区的大小，函数将不执行任何操作并返回0。
 * 
 * @note   写入不会超过缓冲区的大小。当强制写入且实际写入量大于未使用的缓冲区空间时，
 *         会更新前端索引以覆盖旧数据，使used字段等于缓冲区大小。
 * @param  obj:    环形数组对象
 * @param  pdata:  待写入数据的缓冲区指针
 * @param  length: 待写入数据的长度
 * @param  force:  是否强制写入
 * @return 实际写入长度
 */
uint16_t ringbuf_write(ringbuf_t *obj, uint8_t *pdata, uint16_t length, bool force)
{
    if (obj == NULL || pdata == NULL || length == 0 || obj->size == 0)
        return 0;

    uint16_t space = force ? obj->size : obj->size - obj->used;
    if (length > space)
        return 0;

    uint16_t written = length < space ? length : space;
    for (uint16_t i = 0; i < written; ++i)
    {
        obj->arr[obj->rear] = pdata[i];
        obj->rear = (obj->rear + 1) % obj->size;
    }

    if (force && written > obj->size - obj->used)
    {
        uint16_t cover_len = written - (obj->size - obj->used);
        obj->front = (obj->front + cover_len) % obj->size;
        obj->used = obj->size;
    }
    else
    {
        obj->used += written;
    }

    return written;
}

/**
 * @brief  从环形数组读出数据
 * @note   读出数据的同时会从环形数组中删除数据。
 * @param  obj        环形数组对象
 * @param  pdata      接收数据的缓冲区指针
 * @param  length     要读取的数据长度
 * @param  strictRead 是否采用严格读取模式。当设置为true时，如果请求读取的数据长度超过缓冲区中实际数据长度，则不进行读取
 * @return 实际读取长度
 */
uint16_t ringbuf_read(ringbuf_t *obj, uint8_t *pdata, uint16_t length, bool strictRead)
{
    if (obj == NULL || pdata == NULL || length == 0 || obj->used == 0)
        return 0;

    if (strictRead && length > obj->used)
        return 0;

    uint16_t readLength = length > obj->used ? obj->used : length;

    for (uint16_t i = 0; i < readLength; i++)
    {
        pdata[i] = obj->arr[obj->front];
        obj->front = (obj->front + 1) % obj->size;
    }

    obj->used -= readLength;

    return readLength;
}

/**
 * @brief  移除环形数组中指定长度的数据
 * @note   主要用来移除不需要的数据，当参数length为正时从前往后移除（相当于FIFO），若为负时从后
 *         往前移除（相当于LIFO）。
 * @param  obj:    环形数组对象
 * @param  length: 要移除的数据长度
 * @return 实际移除长度
 */
uint16_t ringbuf_remove(ringbuf_t *obj, int length)
{
    if (obj == NULL || length == 0 || obj->used == 0)
        return 0;

    if (abs(length) >= obj->used)
    {
        length = obj->used;
        obj->front = 0;
        obj->rear = 0;
        obj->used = 0;
        return length;
    }

    if (length > 0)
    {
        obj->front = (obj->front + length) % obj->size;
        obj->used -= length;
        return length;
    }
    else
    {
        obj->rear = (obj->rear + obj->size + length) % obj->size;
        obj->used += length;
        return -length;
    }
}

/**
 * @brief  批量修改环形数组中指定索引范围内已存在的数据。仅为修改数据，不是写入数据。
 * @note   索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能修改已写入的数据，不是写入数据。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的起始索引
 * @param  pdata  要修改数据的指针
 * @param  length 要修改数据的长度
 * @return 修改操作是否成功
 */
bool ringbuf_modify(ringbuf_t *obj, uint16_t index, uint8_t *pdata, uint16_t length)
{
    if (obj == NULL || pdata == NULL || index >= obj->used || length > (obj->used - index) || obj->size == 0)
        return false;

    uint16_t start_pos = (obj->front + index) % obj->size;
    for (uint16_t i = 0; i < length; ++i)
    {
        obj->arr[(start_pos + i) % obj->size] = pdata[i];
    }
    return true;
}

/**
 * @brief  预览环形数组中指定索引范围内已存在的数据。仅读取数据，不会移除数据。
 * @note   索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能读取已写入的数据且只读不删。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的起始索引
 * @param  pdata  用于存储读取数据的缓冲区指针
 * @param  length 要读取的数据长度
 * @return 读取操作是否成功
 */
bool ringbuf_peek(ringbuf_t *obj, uint16_t index, uint8_t *pdata, uint16_t length)
{
    if (obj == NULL || pdata == NULL || index >= obj->used || length > (obj->used - index) || obj->size == 0)
        return false;

    uint16_t start_pos = (obj->front + index) % obj->size;
    for (uint16_t i = 0; i < length; ++i)
    {
        pdata[i] = obj->arr[(start_pos + i) % obj->size];
    }

    return true;
}

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
bool ringbuf_updatePointer(ringbuf_t *obj, uint16_t front, uint16_t rear, uint16_t used)
{
    if (obj == NULL || used > obj->size)
        return false;

    front = front % obj->size;
    rear = rear % obj->size;
    used = used % obj->size;

    uint16_t len = (front <= rear) ? rear - front : obj->size - front + rear;

    if (len != used && !((len == 0) && used == obj->size))
        return false;

    obj->front = front;
    obj->rear = rear;
    obj->used = used;
    return true;
}

/**
 * @brief  向环形数组指定索引处开始填充指定值，最大可填充整个环形数组。(C库 memset函数)
 * @note   注意此函数仅填充数据不会改变环形数组信息(读写指针、used等)。
 * @param  obj    环形数组对象
 * @param  index  相对读指针的索引
 * @param  value  要填充的值
 * @param  length 要填充的长度
 * @return 实际填充长度
 */
uint16_t ringbuf_memset(ringbuf_t *obj, uint16_t index, uint8_t value, uint16_t length)
{
    if (obj == NULL || length == 0 || index >= obj->size)
        return 0;

    uint16_t start = (obj->front + index) % obj->size;
    length = (length <= obj->size) ? length : obj->size;

    for (uint16_t i = 0; i < length; i++)
    {
        obj->arr[start] = value;
        start = (start + 1) % obj->size;
    }

    return length;
}

/**
 * @brief  从环形数组指定索引范围内已存在的数据中获取字符串长度。(C库 strlen函数)
 * @param  obj   环形数组对象
 * @param  index 相对读指针的索引
 * @return 字符串长度
 */
uint16_t ringbuf_strlen(ringbuf_t *obj, uint16_t index)
{
    if (obj == NULL || index >= obj->used)
        return 0;

    uint16_t start = (obj->front + index) % obj->size;
    uint16_t count = 0;

    while (count < obj->used && obj->arr[start] != '\0')
    {
        count++;
        start = (start + 1) % obj->size;
    }

    return count;
}

/**
 * @brief  从环形数组指定索引范围内已存在的数据中查找字符并返回相对索引。(C库 strchar函数)
 * @param  obj   环形数组对象
 * @param  index 相对读指针的索引
 * @param  ch    要查找的字符
 * @return 匹配字符相对读指针的索引，如果找不到则返回 -1
 */
int ringbuf_strchr(ringbuf_t *obj, uint16_t index, uint8_t ch)
{
    if (obj == NULL || index >= obj->used)
        return -1;

    uint16_t start = (obj->front + index) % obj->size;
    uint16_t count = 0;
    while (count < obj->used)
    {
        if (obj->arr[start] == ch)
            return (index + count) % obj->used;

        count++;
        start = (start + 1) % obj->size;
    }

    return -1;
}

/**
 * @brief  从环形数组指定索引范围内已存在的数据中查找子字符串并返回相对索引。(C库 strstr函数)
 * @param  obj       环形数组对象
 * @param  index     相对读指针的索引
 * @param  substring 要查找的子字符串
 * @return 子字符串相对读指针的索引，如果未找到则返回 -1
 */
int ringbuf_strstr(ringbuf_t *obj, uint16_t index, const char *substring)
{
    if (obj == NULL || index >= obj->used || substring == NULL)
        return -1;

    uint16_t substring_len = 0;
    while(substring[substring_len] != '\0')
    {
        substring_len++;
    }

    uint16_t start = (obj->front + index) % obj->size;
    uint16_t count = 0;
    while (index + count + substring_len <= obj->used)
    {
        uint16_t i;
        for (i = 0; i < substring_len; i++)
        {
            if (obj->arr[(start + i) % obj->size] != substring[i])
                break;
        }

        if (i == substring_len)
            return (index + count) % obj->used;

        count++;
        start = (start + 1) % obj->size;
    }

    return -1;
}
