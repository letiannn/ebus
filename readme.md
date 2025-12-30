# ebus 事件总线

## 项目概述

**ebus** 是一个轻量级的事件总线系统，专为 RT-Thread 系统设计，提供节点间的消息通信机制。该系统支持多种消息类型，包括广播、通知、指示和响应，具有异步处理、回调机制和等待响应管理等功能。

## 核心特性

### 1. 多种消息类型
- **广播消息** ([eEbusMsgType_Broadcast](.\ebus.h#L40-L40)): 发送给所有注册节点
- **通知消息** ([eEbusMsgType_Notification](.\ebus.h#L41-L41)): 单向通知，无需应答
- **指示消息** ([eEbusMsgType_Indication](.\ebus.h#L42-L42)): 需要应答的请求消息
- **响应消息** ([eEbusMsgType_Response](.\ebus.h#L43-L43)): 对指示消息的应答

### 2. 异步通信机制
- 支持异步指示消息处理
- 提供回调机制处理接收事件
- 支持响应超时管理

### 3. 线程安全
- 使用互斥量保护共享资源
- 消息队列确保线程安全的消息传递

## 系统架构

### 核心数据结构

| 结构体 | 作用 |
|--------|------|
| [sEbusNode_t](.\ebus.h#L95-L106) | 总线节点数据，包含消息队列、回调函数等 |
| [sEbusMsgItem_t](.\ebus.h#L74-L84) | 消息项结构，定义消息类型、源/目标节点、序列号等 |
| [sEbusWaitResp_t](.\ebus.h#L87-L93) | 等待响应项，管理异步消息的应答状态 |
| [sEbus_t](.\ebus.h#L108-L115) | 总线整体信息，包含节点表、互斥量等 |

### 主要功能函数

#### 总线管理
- [Ebus_Create()](.\ebus.h#L110-L110): 创建事件总线
- [Ebus_Destory()](.\ebus.h#L112-L112): 销毁事件总线

#### 节点管理
- [Ebus_NodeCreate()](.\ebus.h#L114-L114): 创建总线节点
- [Ebus_NodeDestory()](.\ebus.h#L116-L116): 销毁总线节点

#### 消息通信
- [Ebus_MsgRecv()](.\ebus.h#L120-L120): 非阻塞接收消息
- [Ebus_MsgWaitRecv()](.\ebus.h#L118-L118): 阻塞接收消息（支持超时）
- [Ebus_Broadcast()](.\ebus.h#L122-L122): 广播消息
- [Ebus_Notification()](.\ebus.h#L124-L124): 发送通知消息（无需应答）
- [Ebus_IndicationAsync()](.\ebus.h#L126-L126): 发送异步指示消息（需要应答）
- [Ebus_Response()](.\ebus.h#L128-L128): 发送响应消息

## 配置参数

| 宏定义 | 默认值 | 说明 |
|--------|--------|------|
| [EBUS_MAX_NODE_NUM](.\ebus.h#L5-L6) | 10 | 最大节点数量 |
| [EBUS_MAX_MSG_SIZE](.\ebus.h#L6-L7) | 8 | 消息最大长度（字节） |
| [EBUS_MAX_MSG_NUM](.\ebus.h#L7-L8) | 10 | 消息队列数量 |
| [EBUS_NODE_MAX_RESP_WAIT_NUM](.\ebus.h#L8-L9) | 10 | 节点最大等待响应数量 |
| [EBUS_RESPONSE_WAIT_TIME_MS](.\ebus.h#L9-L10) | 1000 | 响应等待时间（毫秒） |

## 使用示例

### 基本用法

```c
// 1. 创建事件总线
Ebus_Create();

// 2. 创建节点并注册回调
sEbusNode_t *node = Ebus_NodeCreate("Node1", Node1Cb);

// 3. 消息接收循环
sEbusMsgItem_t rx_msg = {0};
while (1) {
    eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
    if (rst == eEbusRst_Success) {
        node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
    }
    rt_thread_mdelay(10);
}
```

### 回调函数定义

```c
void Node1Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    switch (evt) {
        case eEBusEvtType_IndicationAckCb:
            // 处理应答回调
            break;
        case eEbusEvtType_IndicationCb:
            // 处理指示回调，发送响应
            Ebus_Response(node, (sEbusNode_t *)user_data, &resp_msg);
            break;
        default:
            // 处理普通接收事件
            break;
    }
}
```

### 消息发送

```c
// 发送通知消息
sEbusMsgItem_t msg = {0};
msg.len = 8;
rt_memset(msg.data, 0x55, msg.len);
msg.evt_id = EbusEvtId_P2p;
Ebus_Notification(node, "TargetNode", &msg);

// 发送广播消息
Ebus_Broadcast(node, &msg);

// 发送异步指示消息
Ebus_IndicationAsync(node, "TargetNode", &msg);
```

## 事件类型

- [eEbusEvtType_RecvCb](.\ebus.h#L51-L51): 普通接收回调
- [eEBusEvtType_IndicationCb](.\ebus.h#L52-L52): 指示消息回调（需要响应）
- [eEBusEvtType_IndicationAckCb](.\ebus.h#L53-L53): 指示应答回调

## 错误码

| 错误码 | 含义 |
|--------|------|
| [eEbusRst_Success](.\ebus.h#L25-L25) | 操作成功 |
| [eEbusRst_Timeout](.\ebus.h#L27-L27) | 操作超时 |
| [eEbusRst_NoMemory](.\ebus.h#L28-L28) | 内存不足 |
| [eEbusRst_ParamErr](.\ebus.h#L29-L29) | 参数错误 |
| [eEbusRst_NodeNotFound](.\ebus.h#L30-L30) | 节点未找到 |
| [eEbusRst_QueueFull](.\ebus.h#L32-L32) | 队列满 |

## 调试功能

- `ebus_show()`: 显示所有节点的等待响应信息
- 日志输出：支持不同级别的日志记录
- FinSH 命令：`ebus_show` 命令可查看总线状态

## 适用场景

- RT-Thread 系统中的模块间通信
- 需要异步消息处理的系统
- 支持多种通信模式的事件驱动架构
- 需要可靠消息传递的嵌入式应用

## 注意事项

1. 在使用前必须先调用 [Ebus_Create()](.\ebus.h#L110-L110) 初始化总线
2. 每个节点必须提供回调函数处理接收到的消息
3. 注意处理各种错误码以确保系统稳定性
4. 合理设置配置参数以适应具体应用场景的资源限制