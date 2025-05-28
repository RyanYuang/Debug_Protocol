# 调试协议 🚀

调试协议是一个基于C语言的轻量级串口通信协议工具包，专为支持串口通信的嵌入式系统设计。它提供了一种简单高效的方式来处理串口调试和数据交换，支持慢速和快速数据处理模式。🛠️

## 功能 ✨

- **灵活的协议结构**：支持命令和变量格式，实现多样化的通信。
  - 命令格式：`@ 0x01 0x20 [Content] \r\n` 📡
  - 变量格式：`@ 0x02 0x20 [VAR_Name]:[Data]\r\n` 📊
- **两种处理模式**：
  - **慢速模式**：适用于命令和变量的详细解析和处理,适用于与上位机进行通信的调试。🐢
  - **快速模式**：优化了高速度数据传输，减少开销，适用于与其他设备的快速串口通信如Openmv。⚡
- **变量和命令管理**：
  - 使用 `Val_Create` 创建和管理变量。📋
  - 使用 `CMD_Create` 注册带有回调函数的自定义命令。🔧
- **平台无关**：兼容任何支持串口通信的嵌入式平台。🌐
- **轻量高效**：专为资源受限环境设计。💾

## 安装 📦

1. **克隆仓库**：
   ```bash
   git clone https://github.com/RyanYuang/Debug-Protocol.git
   ```

2. **加入项目**：
   - 将 `Common_Communicate_Protocol.c` 和 `Common_Communicate_Protocol.h` 复制到您的项目目录中。📂
   - 确保您的开发环境中包含 `HeadFiles.h`（或任何其他必需的依赖项）。

3. **使用工具链构建**：
   - 该代码库使用 Keil V5 IDE 开发，但与其它 C 编译器（如 GCC for embedded systems）兼容。🔨
   - 将源文件添加到您的项目中并进行编译。

## 使用 🖥️

### 1. 初始化协议
创建一个 `Protocol_t` 对象，并使用您所需的模式（`SLOW_TYPE` 或 `FAST_TYPE`）进行初始化。

```c
#include "Common_Communicate_Protocol.h"

Protocol_t USART1_Protocol;

void main() {
    Protocol_Init(&USART1_Protocol, SLOW_TYPE); // 🚀 初始化
}
```

### 2. 处理串口数据
在串口中断处理程序中调用 `Rec_Proc` 来处理接收到的数据。

```c
void USART1_IRQHandler(void) {
    uint8_t received_data = /* 您的串口接收函数 */;
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
float speed = 0.0;
Val_Create("Speed", &speed); // 📝 注册变量
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

### 示例数据格式
- **命令**：发送 `@ 0x01 0x20 Reply \r\n` 以触发 `reply_function`。🎯
- **变量**：发送 `@ 0x02 0x20 Speed:123.45 \r\n` 以将 `speed` 变量设置为 `123.45`。📈

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
