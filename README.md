# EBUS (Event Bus)

EBUS 是一个轻量级的事件总线组件，基于 RT-Thread 实时操作系统开发。它提供了节点间的消息通信机制，支持广播、通知、指示-响应等多种通信模式。

## 特性

- **轻量级设计**：占用资源少，适合嵌入式系统
- **多种通信模式**：支持广播、点对点通知、指示-响应等通信模式
- **异步响应机制**：通过回调函数异步处理响应，不阻塞发送线程
- **消息队列隔离**：每个节点独立的消息队列，保证消息处理顺序
- **序列号管理**：全局序列号自动递增，支持消息追踪
- **互斥锁保护**：总线操作和节点响应管理使用互斥锁保护，保证线程安全
- **等待响应管理**：支持多路并行等待响应，自动管理超时

## 目录结构

```
ebus/
├── ebus.h              # EBUS 核心头文件
├── ebus.c              # EBUS 核心实现
├── SConscript          # SCons 构建脚本
└── example/           # 示例代码
    ├── ebus_base_example.c    # 基础通信示例
    └── ebus_ack_example.c   # 异步响应示例
```

## 核心概念

### 消息类型

| 类型 | 说明 | 是否需要响应 |
|------|------|-------------|
| `eEbusMsgType_Broadcast` | 广播消息，发送给所有其他节点 | 否 |
| `eEbusMsgType_Notification` | 点对点通知消息 | 否 |
| `eEbusMsgType_Indication` | 指示消息，需要响应 | 是 |
| `eEbusMsgType_Response` | 响应消息 | 否 |

### 事件类型

| 类型 | 说明 |
|------|------|
| `eEbusEvtType_RecvCb` | 接收普通消息回调 |
| `eEBusEvtType_IndicationCb` | 接收指示消息回调（需发送响应） |
| `eEBusEvtType_IndicationAckCb` | 接收到响应的回调 |

### 节点结构

每个节点包含：
- 消息队列：接收来自其他节点的消息
- 回调函数：处理接收到的消息
- 等待响应列表：管理已发送但未收到响应的请求

## 配置参数

在 `ebus.h` 中可配置的参数：

```c
#define EBUS_NAME_LEN               (32)     // 节点名称最大长度
#define EBUS_MAX_NODE_NUM           (10)     // 最大节点数量
#define EBUS_MAX_MSG_SIZE           (8)      // 单条消息最大数据长度
#define EBUS_MAX_MSG_NUM            (10)     // 每个节点消息队列容量
#define EBUS_NODE_MAX_RESP_WAIT_NUM (10)     // 单个节点最大等待响应数量
#define EBUS_RESPONSE_WAIT_TIME_MS  (1000)   // 响应超时时间（预留）
```

## API 参考

### 总线管理

```c
// 创建事件总线
void EbusCreate(void);

// 销毁事件总线
void EbusDestory(void);
```

### 节点管理

```c
// 创建节点
sEbusNode_t *EbusNodeCreate(char *name, EbusCbPtr EvtCb);

// 销毁节点
void EbusNodeDestory(sEbusNode_t *node);
```

### 消息发送

```c
// 广播消息（发送给所有其他节点）
eEbusRst_t EbusBroadcast(sEbusNode_t *node, sEbusMsgItem_t *msg);

// 点对点通知（无应答）
eEbusRst_t EbusNotification(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg);

// 点对点指示（异步，需要响应）
eEbusRst_t EbusIndicationAsync(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg);

// 发送响应消息
eEbusRst_t EbusResponse(sEbusNode_t *node, sEbusNode_t *ack_node, sEbusMsgItem_t *msg);
```

### 消息接收

```c
// 非阻塞接收消息
eEbusRst_t EbusMsgRecv(sEbusNode_t *node, sEbusMsgItem_t *msg);

// 阻塞接收消息（带超时）
eEbusRst_t EbusMsgWaitRecv(sEbusNode_t *node, sEbusMsgItem_t *msg, uint32_t timeout);
```

### 返回值

| 返回值 | 说明 |
|--------|------|
| `eEbusRst_Success` | 操作成功 |
| `eEbusRst_Fail` | 操作失败 |
| `eEbusRst_Timeout` | 超时 |
| `eEbusRst_NoMemory` | 无可用等待响应槽位 |
| `eEbusRst_ParamErr` | 参数错误 |
| `eEbusRst_NodeNotFound` | 节点未找到 |
| `eEbusRst_OtherEvt` | 其他事件类型（如指示、响应等） |
| `eEbusRst_QueueFull` | 消息队列已满 |

## 使用示例

### 基础通信示例

```c
#include "ebus.h"

#define NODE1_NAME "Node1"
#define NODE2_NAME "Node2"

// 节点1的回调函数
static void Node1Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    LOG_I("Node1 received: evt_id=0x%04X, len=%d", msg->evt_id, msg->len);
}

// 接收线程
static void recv_thread_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = { 0 };
    sEbusNode_t *node = EbusNodeCreate(NODE1_NAME, Node1Cb);
    while (1)
    {
        eEbusRst_t rst = EbusMsgRecv(node, &rx_msg);
        while (rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = EbusMsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

// 发送线程
static void send_thread_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg = { 0 };
    tx_msg.len = 8;
    tx_msg.evt_id = 0x8001;
    rt_memset(tx_msg.data, 0x55, tx_msg.len);
    
    sEbusNode_t *node = EbusNodeCreate(NODE2_NAME, NULL);
    
    while (1)
    {
        // 发送点对点通知
        EbusNotification(node, NODE1_NAME, &tx_msg);
        rt_thread_mdelay(1000);
    }
}

void ebus_base_example(void)
{
    EbusCreate();
    
    rt_thread_t tid1 = rt_thread_create("recv", recv_thread_entry, RT_NULL, 2048, 25, 5);
    rt_thread_t tid2 = rt_thread_create("send", send_thread_entry, RT_NULL, 2048, 25, 5);
    
    if (tid1) rt_thread_startup(tid1);
    if (tid2) rt_thread_startup(tid2);
}
MSH_CMD_EXPORT(ebus_base_example, ebus base example);
```

### 异步响应示例

```c
#include "ebus.h"

// 接收指示时的回调
static void NodeCb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if (evt == eEBusEvtType_IndicationCb)
    {
        // 收到指示消息，发送响应
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        sEbusMsgItem_t resp_msg = { 0 };
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        EbusResponse(node, ack_node, &resp_msg);
    }
    else if (evt == eEBusEvtType_IndicationAckCb)
    {
        // 收到响应
        LOG_I("Response received for seq_num=%d", msg->seq_num);
    }
}

// 发送指示并异步等待响应
static void send_thread_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg = { 0 };
    tx_msg.len = 8;
    tx_msg.evt_id = 0x8002;
    rt_memset(tx_msg.data, 0xAA, tx_msg.len);
    
    sEbusNode_t *node = EbusNodeCreate("Node1", NodeCb);
    
    while (1)
    {
        // 发送指示消息，异步等待响应
        EbusIndicationAsync(node, "Node2", &tx_msg);
        rt_thread_mdelay(1000);
    }
}
```

### 查看等待响应状态

在 FinSH 控制台执行：

```
ebus_show
```

输出示例：
```
Ebus Wait Response Info - Current tick: 1000

Node: Node1 (ID:0)
  Slot[0]: Seq=0x0001, State=SENTED, Src=0->Dst=1, SendTime=900, Wait=100ms
  Slot[1]: Seq=0x0002, State=RECVED, Src=0->Dst=2, SendTime=800, Wait=200ms
  Active slots: 2/10

End of ebus wait response info
```

## 通信流程

### 广播流程
```
发送节点 ──┬──> 节点1
          ├──> 节点2
          ├──> 节点3
          └──> ...
```

### 通知流程（无应答）
```
发送节点 ──> 接收节点
```

### 指示-响应流程
```
发送节点 ──[Indication]──> 接收节点
                    └──[Response]──> 发送节点
```

## 编译配置

在 `SConscript` 中配置组件编译：

```python
from building import *

cwd = GetCurrentDir()
src = Glob('*.c')
src += Glob('example/*.c')

group = DefineGroup('ebus', src, depend = [''])
Return('group')
```

## 依赖

- RT-Thread 实时操作系统
- rt_mq (消息队列)
- rt_mutex (互斥锁)
- ulog 组件（用于日志输出，可选）

## 注意事项

1. **节点名称唯一性**：确保每个节点的名称唯一
2. **消息大小限制**：单条消息数据长度受 `EBUS_MAX_MSG_SIZE` 限制
3. **队列容量**：消息队列满时发送会返回 `eEbusRst_QueueFull`
4. **等待响应数量**：每个节点最多支持 `EBUS_NODE_MAX_RESP_WAIT_NUM` 个并发等待响应
5. **线程安全**：节点回调函数中尽量减少耗时操作
6. **资源释放**：使用完毕后需要调用 `EbusNodeDestory()` 销毁节点，调用 `EbusDestory()` 销毁总线

