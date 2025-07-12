# 调试协议 🚀

调试协议是一个基于C语言的轻量级串口通信协议工具包，专为支持串口通信的嵌入式系统设计。它提供了一种简单高效的方式来处理串口调试和数据交换，支持慢速和快速数据处理模式。🛠️

## 功能 ✨

- **灵活的协议结构**：支持命令和变量格式，实现多样化的通信。
  - 命令格式：`@ 0x01 [MACHINE CODE] [Content] \r\n` 📡
  - 变量格式：`@ 0x02 [MACHINE CODE] [VAR_Name]:[Data]\r\n` 📊
- **两种处理模式**：
  - **慢速模式**：适用于命令和变量的详细解析和处理，适合与上位机通信调试。🐢
  - **快速模式**：优化高速度数据传输，减少开销，适用于与其他设备（如Openmv）的快速串口通信。⚡
- **变量和命令管理**：
  - 使用 `Val_Create` 创建和管理变量（支持浮点型、整型等类型）。📋
  - 使用 `CMD_Create` 注册带有回调函数的自定义命令。🔧
- **平台无关**：兼容任何支持串口通信的嵌入式平台（如STM32、Arduino等）。🌐
- **轻量高效**：专为资源受限环境设计，内存占用低（协议缓冲区默认100字节，可自定义）。💾

## 安装 📦

1. **克隆仓库**：
   ```bash
   git clone https://github.com/RyanYuang/Debug_Protocol.git
   ```

2. **加入项目**：
   - 将 `Debug_Protocol.c` 和 `Debug_Protocol.h` 复制到您的项目目录中。📂
   - 确保您的开发环境中包含 `HeadFiles.h`（或任何其他必需的依赖项）。

3. **使用工具链构建**：
   - 该代码库使用 Keil V5 IDE 开发，但与其它 C 编译器（如 GCC for embedded systems）兼容。🔨
   - 将源文件添加到您的项目中并进行编译。

## 使用 🖥️

### 1. 初始化协议
创建一个 `Protocol_t` 对象，并使用您所需的模式（`SLOW_TYPE` 或 `FAST_TYPE`）进行初始化。

```c
#include "Debug_Protocol.h"
#include "PerformanceTest.h"

Protocol_t USART1_Protocol;

void main() {
    Protocol_Init(&USART1_Protocol, SLOW_TYPE); // 🚀 初始化
}
```

### 2. 处理串口数据
在串口中断处理程序中调用 `Rec_Proc` 来处理接收到的数据。

```c
void USART1_IRQHandler(void) {
    uint8_t received_data[1];
    uint8_t received_data[0] = /* 您的串口接收函数 */;
    Rec_Proc(&USART1_Protocol, received_data); // 📥 接收数据
}
```

### 3. 处理协议数据
在主循环中（而不是中断中）调用 `Protocol` 来处理缓冲区中的数据。

```c
void main() {
    Protocol_Init(&USART1_Protocol, SLOW_TYPE);
    while (1) {
        Protocol(&USART1_Protocol, SLOW_TYPE); // 🔄 处理数据
    }
}
```

### 4. 创建变量
注册由协议管理的变量。

```c
float speed_f = 0.0;
int   speed_i = 0;
Val_Create("speed_f", &speed_f); // 📝 注册变量
Val_Create("speed_i", &speed_i); // 📝 注册变量
```

### 5. 创建命令
定义带有回调函数的自定义命令。

```c
void reply_function() {
    // 您的命令逻辑
    printf("OK\r\n"); // ✅ 回复
}

CMD_Create("Reply", &reply_function); // 🔗 链接命令
```
### 6.修改Machine Code
```
<Debug_Protocol.h>
#define MACHINE_ADDR 0x20 (可以改为你需要的机器码，但是不要与宏定义中的数值相同即可)
```
### 示例数据格式
#### Slow Mode
- MACHINE CODE为 0x20
- **命令**：`发送端`发送 `@ 0x01 0x20 Reply \r` 以触发 `reply_function`。🎯
- **变量**：`发送端`发送 `@ 0x02 0x20 Speed:123.45 \r` 以将 `speed` 变量设置为 `123.45`。📈
#### Fast Mode
- MACHINE CODE为 0x20
- **数据**：`发送端`发送 `@ 32,10\r`，以发送第一个数据32，第二个数据10，假设我们使用`USART1_Protocol`结构体来接收数据，那么传输过来的数据在`USART1_Protocol.Data_Buffer.Data_Arry`中，如果想获取数据，那么可以使用`USART1_Protocol.Data_Buffer.Data_Arry[0]`来获取第一个数据32，`USART1_Protocol.Data_Buffer.Data_Arry[1]`来获取第二个数据10。<br>
- 如果是`整形数据`，需要使用函数`strtol`来进行数据转换，函数原型为：`long int strtol(const char *str, char **endptr, int base);`，第一个参数为待转换的字符串，第二个参数为指向字符指针的指针，第三个参数为进制数，返回转换后的长整型数。
- 如果是`浮点型数据`，需要使用函数`strtof`来进行数据转换，函数原型为：`float strtof(const char *str, char **endptr);`，第一个参数为待转换的字符串，第二个参数为指向字符指针的指针，返回转换后的浮点型数。



## 注意事项 📌
- **中断安全**：确保在串口中断中调用 `Rec_Proc`，而在主循环中调用 `Protocol`，以避免性能问题。⚠️
- **调试**：取消注释调试打印语句（例如，`printf("Get Head\r\n");`）以进行故障排除。🐞
- **依赖项**：代码假定存在包含标准 C 头文件（如 `stdio.h`、`string.h`）的 `HeadFiles.h`。请确保这些文件可用或根据需要调整。

## 贡献 🤝
欢迎贡献！请按照以下步骤操作：
1. 叉取仓库。🍴
2. 创建新分支（`git checkout -b feature/your-feature`）。
3. 提交您的更改（`git commit -m 'Add your feature'`）。✍️
4. 推送到分支（`git push origin feature/your-feature`）。🚀
5. 发起拉取请求。📬

修改代码时，请按照 MIT 许可证条款保留原始版权声明。



## 许可证 📜
本项目采用 MIT 许可证授权。您可以自由使用、复制、修改、合并、发布、分发、授权和/或销售软件副本，但需遵守以下条件：

- 上述版权声明和许可声明应包含在软件的所有副本或主要部分中。

请参阅 [LICENSE](LICENSE) 文件了解完整详情。

## 联系 📧
如有问题或建议，请在 [GitHub 仓库](https://github.com/RyanYuang/Debug-Protocol) 上提出问题，或联系作者 Ryan Yuang
Email: RyanYuang@outlook.com
