# 环形数组
## 介绍
环形数组(rangbuf)是一种常见的队列(queue)结构。ringbuf在环形数组的基础上实现了C库部分常用的字符串处理函数。
## 配置
### 动态内存
ringbuf支持动态内存，默认使用malloc与free函数，如有需要可以在`ringbuf.h`中修改配置宏以使用自定义内存分配函数。
```c
#ifndef ringbuf_malloc
	#include <stdlib.h>
	#define ringbuf_malloc malloc // 定义malloc函数
#endif //ringbuf_malloc

#ifndef ringbuf_free
	#define ringbuf_free   free   // 定义free函数
#endif //ringbuf_free
```
## 使用步骤
1. 首先创建动态或者静态环形数组并得到初始化后的环形数组对象。
2. 然后根据需要调用相关API对数据进行增删改查。
3. 最后删除环形数组对象释放空间。
## 环形数组对象
```c
typedef struct
{
    uint8_t *arr;   // 环形数组内存指针
    uint16_t size;  // 环形数组大小
    uint16_t used;  // 已使用长度
    uint16_t front; // 读指针
    uint16_t rear;  // 写指针
} ringbuf_t;
```
## API
### 01. 创建静态环形数组
```c
bool ringbuf_init(ringbuf_t *obj, uint8_t *arr, uint16_t size);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| arr | 环形数组内存指针 |
| size | 环形数组大小 |
| 返回值 | 是否创建成功 |
### 02. 删除静态环形数组
```c
void ringbuf_deinit(ringbuf_t *obj);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| 返回值 | 无 |
### 03. 创建动态环形数组
```c
ringbuf_t* ringbuf_create(uint16_t size);
```
| 参数名 | 介绍 |
| ---- | ---- |
| size | 环形数组大小 |
| 返回值 | 环形数组对象指针，创建失败返回NULL |
### 04. 删除动态环形数组
```c
void ringbuf_delete(ringbuf_t *obj);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| 返回值 | 无 |
### 05. 获取环形数组已使用长度
```c
uint16_t ringbuf_getUsedLength(ringbuf_t *obj);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| 返回值 | 已使用长度 |
### 06. 判断环形数组是否为空
```c
bool ringbuf_isEmpty(ringbuf_t *obj);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| 返回值 | 是否为空 |
### 07. 判断环形数组是否已满
```c
bool ringbuf_isFull(ringbuf_t *obj);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| 返回值 | 是否已满 |
### 08. 向环形数组写入数据，支持覆盖已有数据的强制写入模式
> 此函数将指定的数据写入环形数组。如果选择了强制写入模式并且缓冲区剩余空间不足，则会覆盖缓冲区中的旧数据。如果尝试写入的数据量超过了缓冲区的大小，函数将不执行任何操作并返回0。
```c
uint16_t ringbuf_write(ringbuf_t *obj, uint8_t *pdata, uint16_t length, bool force);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| pdata | 待写入数据的缓冲区指针 |
| length | 待写入数据的长度 |
| force | 是否强制写入 |
| 返回值 | 实际写入长度 |
### 09. 从环形数组读出数据
> 读出数据的同时会从环形数组中删除数据。
```c
uint16_t ringbuf_read(ringbuf_t *obj, uint8_t *pdata, uint16_t length, bool strictRead);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| pdata | 接收数据的缓冲区指针 |
| length | 要读取的数据长度，如果length大于已有数据数量将只读出已有数据数量 |
| strictRead | 是否采用严格读取模式。当设置为true时，如果请求读取的数据长度超过缓冲区中实际数据长度，则不进行读取 |
| 返回值 | 实际读取长度 |
### 10.  移除环形数组中指定长度的数据
> 主要用来移除不需要的数据，当参数length为正时从前往后移除（相当于FIFO），若为负时从后往前移除（相当于LIFO）。
```c
uint16_t ringbuf_remove(ringbuf_t* obj, int length);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| length | 要移除的数据长度 |
| 返回值 | 实际移除长度 |
### 11. 批量修改环形数组中指定索引范围内已存在的数据。仅为修改数据，不是写入数据
> 索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能修改已写入的数据，不是写入数据。
```c
bool ringbuf_modify(ringbuf_t *obj, uint16_t index, uint8_t *pdata, uint16_t length);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的起始索引 |
| pdata | 要修改数据的指针 |
| length | 要修改数据的长度 |
| 返回值 | 修改操作是否成功 |
### 12. 预览环形数组中指定索引范围内已存在的数据。仅读取数据，不会移除数据
> 索引是相对读指针的索引，不是环形数组内存的绝对索引。注意，仅能读取已写入的数据且只读不删。
```c
bool ringbuf_peek(ringbuf_t *obj, uint16_t index, uint8_t *pdata, uint16_t length);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的起始索引 |
| pdata | 用于存储读取数据的缓冲区指针 |
| length | 要读取的数据长度 |
| 返回值 | 指定索引的数据 |
### 13. 更新环形数组读写指针
> 一般用于数据从其它地方直接缓存到环形数组内存(比如DMA)，此时需要手动的同步改变读写指针。函数会对输入参数合法性进行校验，但由于front、rear与used是直接赋值给环形数组对象，使用时请务必小心。
```c
bool ringbuf_updatePointer(ringbuf_t *obj, uint16_t front, uint16_t rear, uint16_t used);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| front | 新的front指针位置 |
| rear | 新的rear指针位置 |
| used | 新的已使用长度 |
| 返回值 | 是否更新成功 |
### 14. 向环形数组指定相对索引开始填充指定值(C库 memset)
>注意此函数仅填充数据不会改变环形数组信息(读写指针、used等)。
```C
uint16_t ringbuf_memset(ringbuf_t *obj, uint16_t index, uint8_t value, uint16_t length);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的索引 |
| value | 要填充的值 |
| length | 要填充的长度 |
| 返回值 | 实际填充长度 |
### 15. 从环形数组指定索引范围内已存在的数据中获取字符串长度(C库 strlen)
```C
uint16_t ringbuf_strlen(ringbuf_t *obj, uint16_t index);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的索引 |
| 返回值 | 字符串长度 |
### 16. 从环形数组指定索引范围内已存在的数据中查找字符并返回相对索引(C库 strchar函数)
```C
int ringbuf_strchr(ringbuf_t *obj, uint16_t index, uint8_t ch);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的索引 |
| ch| 要查找的字符 |
| 返回值 | 匹配字符相对读指针的索引，如果找不到则返回 -1 |
### 17. 从环形数组指定索引范围内已存在的数据中查找子字符串并返回相对索引(C库 strstr函数)
```C
int ringbuf_strstr(ringbuf_t *obj, uint16_t index, const char *substring);
```
| 参数名 | 介绍 |
| ---- | ---- |
| obj | 环形数组对象 |
| index | 相对读指针的索引 |
| substring| 要查找的子字符串 |
| 返回值 | 匹配字符相对读指针的索引，如果找不到则返回 -1 |
## 许可
MIT开源协议
