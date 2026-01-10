#include "ebus.h"

#define LOG_TAG "ebus"
#define LOG_LVL LOG_LVL_WARNING
#include <ulog.h>

static sEbus_t g_ebus_ = { 0 };

/**
 * @description: 获取流水号
 * @return {*}
 */
static uint16_t EbusGetSn(void)
{
    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    ++g_ebus_.sn;
    rt_mutex_release(g_ebus_.bus_mutex);
    return g_ebus_.sn;
}

/**
 * @description: 根据名称查找节点
 * @param {char} *name
 * @return {*}
 */
static sEbusNode_t *EbusFindNodeByName(const char *name)
{
    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    for (int i = 0; i < EBUS_MAX_NODE_NUM; i++)
    {
        sEbusNode_t *node = g_ebus_.node_tbl[i];
        if (node != RT_NULL && node->init && rt_strcmp(node->name, name) == 0)
        {
            LOG_D("[Ebus] Found node by name: %s, idx: %d", name, i);
            rt_mutex_release(g_ebus_.bus_mutex);
            return node;
        }
    }
    LOG_D("[Ebus] Node not found by name: %s", name);
    rt_mutex_release(g_ebus_.bus_mutex);
    return RT_NULL;
}

/**
 * @description: 在总线中获取一个空闲的节点
 * @return {*}
 */
static int EbusFindIdleIdx(void)
{
    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    int idx = -1;
    for (int i = 0; i < EBUS_MAX_NODE_NUM; i++)
    {
        if (g_ebus_.node_tbl[i] == RT_NULL)
        {
            idx = i;
            LOG_D("[Ebus] Found idle slot at index: %d", idx);
            break;
        }
    }
    if (idx == -1)
    {
        LOG_W("[Ebus] No idle slots available in bus");
    }
    rt_mutex_release(g_ebus_.bus_mutex);
    return idx;
}

/**
 * @description: 通过节点id地址获取节点
 * @param {sEbusNode_t} *bus
 * @return {*}
 */
static sEbusNode_t *EbusFindNodeByIdx(uint8_t node_idx)
{
    sEbusNode_t *node = RT_NULL;
    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    int idx = 0;
    for (idx = 0; idx < EBUS_MAX_NODE_NUM; idx++)
    {
        if (g_ebus_.node_tbl[idx] != RT_NULL && g_ebus_.node_tbl[idx]->node_idx == node_idx)
        {
            node = g_ebus_.node_tbl[idx];
            LOG_D("[Ebus] Found node by idx: %d, name: %s", node_idx, node->name);
            break;
        }
    }
    if (node == RT_NULL)
    {
        LOG_D("[Ebus] Node not found by idx: %d", node_idx);
    }
    rt_mutex_release(g_ebus_.bus_mutex);
    return node;
}

/**
 * @description: 在节点中查找等待响应的项
 * @param {sEbusNode_t} *node
 * @param {uint16_t} seq_num
 * @param {uint8_t} src_node_idx
 * @return {*} 找到返回索引，未找到返回 -1
 */
static int EbusFindWaitRespItem(sEbusNode_t *node, uint16_t seq_num)
{
    if (node == RT_NULL || !node->init)
    {
        LOG_E("[Ebus] Invalid node when finding wait response item");
        return -1;
    }

    rt_mutex_take(node->resp_mutex, RT_WAITING_FOREVER);
    for (int i = 0; i < EBUS_NODE_MAX_RESP_WAIT_NUM; i++)
    {
        if (node->wait_resp_list[i].state != eEbusMsgState_Idle &&
            node->wait_resp_list[i].seq_num == seq_num)
        {
            LOG_D("[Ebus] Found wait response item: seq=%d, idx=%d, node=%s",
                  seq_num, i, node->name);
            rt_mutex_release(node->resp_mutex);
            return i;
        }
    }
    LOG_D("[Ebus] Wait response item not found: seq=%d, node=%s", seq_num, node->name);
    rt_mutex_release(node->resp_mutex);
    return -1;
}

/**
 * @description: 在节点中分配一个等待响应的项
 * @param {sEbusNode_t} *node
 * @return {*} 成功返回索引，失败返回 -1
 */
static int EbusAllocWaitRespItem(sEbusNode_t *node)
{
    if (node == RT_NULL || !node->init)
    {
        LOG_E("[Ebus] Invalid node when allocating wait response item");
        return -1;
    }

    rt_mutex_take(node->resp_mutex, RT_WAITING_FOREVER);
    for (int i = 0; i < EBUS_NODE_MAX_RESP_WAIT_NUM; i++)
    {
        if (node->wait_resp_list[i].state == eEbusMsgState_Idle)
        {
            LOG_D("[Ebus] Allocated wait response item: idx=%d, node=%s", i, node->name);
            rt_mutex_release(node->resp_mutex);
            return i;
        }
    }
    LOG_W("[Ebus] No available wait response slots for node: %s", node->name);
    rt_mutex_release(node->resp_mutex);

    return -1;
}

/**
 * @description: 释放等待响应的项
 * @param {sEbusNode_t} *node
 * @param {int} idx
 * @return {*}
 */
static void EbusFreeWaitRespItem(sEbusNode_t *node, int idx)
{
    if (node == RT_NULL || !node->init || idx < 0 || idx >= EBUS_NODE_MAX_RESP_WAIT_NUM)
    {
        LOG_E("[Ebus] Invalid parameters when freeing wait response item");
        return;
    }

    rt_mutex_take(node->resp_mutex, RT_WAITING_FOREVER);
    LOG_D("[Ebus] Freeing wait response item: node=%s, idx=%d", node->name, idx);
    node->wait_resp_list[idx].seq_num = 0;
    node->wait_resp_list[idx].src_node_idx = 0;
    node->wait_resp_list[idx].dst_node_idx = 0;
    node->wait_resp_list[idx].send_time = 0;
    node->wait_resp_list[idx].state = eEbusMsgState_Idle;
    rt_mutex_release(node->resp_mutex);
}

/**
 * @description: 处理接收到的响应消息
 * @param {sEbusNode_t} *node
 * @param {sEbusMsgItem_t} *msg
 * @return {*}
 */
static void EbusProcessResponse(sEbusNode_t *node, sEbusMsgItem_t *msg)
{
    if (node == RT_NULL || !node->init || msg == RT_NULL || msg->type != eEbusMsgType_Response)
    {
        LOG_E("[Ebus] Invalid parameters when processing response");
        return;
    }

    int idx = EbusFindWaitRespItem(node, msg->seq_num);
    if (idx >= 0)
    {
        LOG_D("[Ebus] Processing response: seq=%d, node=%s, src=%d, dst=%d",
              msg->seq_num, node->name, msg->src_node_idx, msg->dst_node_idx);
        EbusFreeWaitRespItem(node, idx);
    }
    else
    {
        LOG_W("[Ebus] Response not in wait list: seq=%d, node=%s", msg->seq_num, node->name);
    }
}

/**
 * @description: 将总线内的节点初始化
 * @param {uint8_t} idx
 * @param {sEbusNode_t} *bus
 * @return {*}
 */
static void EbusBusInit(uint8_t idx, sEbusNode_t *node)
{
    if (idx >= EBUS_MAX_NODE_NUM || node == RT_NULL)
    {
        LOG_E("[Ebus] Invalid bus init parameters: idx=%d", idx);
        return;
    }

    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    g_ebus_.node_tbl[idx] = node;
    node->node_idx = idx;
    node->init = 1;
    LOG_D("[Ebus] Bus node registered: name=%s, idx=%d", node->name, idx);
    rt_mutex_release(g_ebus_.bus_mutex);
}

/**
 * @description: 将总线内的节点去初始化
 * @param {uint8_t} idx
 * @return {*}
 */
static void EbusBusDeinit(uint8_t idx)
{
    if (idx >= EBUS_MAX_NODE_NUM)
    {
        LOG_E("[Ebus] Invalid bus deinit index: %d", idx);
        return;
    }

    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
    if (g_ebus_.node_tbl[idx] != RT_NULL)
    {
        LOG_D("[Ebus] Bus node unregistered: idx=%d", idx);
        g_ebus_.node_tbl[idx] = RT_NULL;
    }
    rt_mutex_release(g_ebus_.bus_mutex);
}

/**
 * @description: 消息发送
 * @param {sEbusNode_t} *node
 * @param {sEbusMsgItem_t} *msg
 * @return {*}
 */
static eEbusRst_t EbusMsgSend(sEbusNode_t *node, sEbusMsgItem_t *msg_item)
{
    if (node == RT_NULL || !node->init || msg_item == RT_NULL)
    {
        LOG_E("[Ebus] Message send parameter error: node=%p, msg=%p", node, msg_item);
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Sending message: type=%d, src=%d, dst=%d, seq=%d",
          msg_item->type, msg_item->src_node_idx, msg_item->dst_node_idx, msg_item->seq_num);

    switch (msg_item->type)
    {
    case eEbusMsgType_Broadcast:
    {
            /* 广播消息：发送给所有已注册的节点（除了自己） */
        int send_count = 0;
        rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);
        for (int i = 0; i < EBUS_MAX_NODE_NUM; i++)
        {
            sEbusNode_t *target_node = g_ebus_.node_tbl[i];
            if (target_node != RT_NULL && target_node->init && target_node != node)
            {
                rt_err_t result = rt_mq_send(target_node->msg_queue, msg_item, sizeof(sEbusMsgItem_t));
                if (result == RT_EOK)
                {
                    send_count++;
                    LOG_D("[Ebus] Broadcast sent to node: %s", target_node->name);
                }
                else if (result == -RT_EFULL)
                {
                    LOG_W("[Ebus] Node %s message queue full, drop broadcast", target_node->name);
                }
                else
                {
                    LOG_E("[Ebus] Send broadcast to node %s failed: %d", target_node->name, result);
                }
            }
        }
        rt_mutex_release(g_ebus_.bus_mutex);
        LOG_D("[Ebus] Broadcast completed: src=%d, seq=%d, sent_to=%d nodes",
              msg_item->src_node_idx, msg_item->seq_num, send_count);
    }
    break;

    case eEbusMsgType_Response:
    case eEbusMsgType_Notification:
    case eEbusMsgType_Indication:
    {
        sEbusNode_t *target_node = (sEbusNode_t *)EbusFindNodeByIdx(msg_item->dst_node_idx);
        if (target_node != RT_NULL)
        {
            rt_err_t result = rt_mq_send(target_node->msg_queue, msg_item, sizeof(sEbusMsgItem_t));
            if (result == RT_EOK)
            {
                LOG_D("[Ebus] Message sent: type=%d, src=%d->%s, dst=%d->%s, seq=%x, evt=%x, len=%d",
                      msg_item->type,
                      msg_item->src_node_idx, node->name,
                      msg_item->dst_node_idx, target_node->name,
                      msg_item->seq_num, msg_item->evt_id, msg_item->len);
            }
            else if (result == -RT_EFULL)
            {
                LOG_E("[Ebus] Target node %s message queue full, drop message", target_node->name);
                return eEbusRst_QueueFull;
            }
            else
            {
                LOG_E("[Ebus] Send to node %s failed: %d", target_node->name, result);
                return eEbusRst_Fail;
            }
        }
        else
        {
            LOG_E("[Ebus] Target node not found: idx=%d", msg_item->dst_node_idx);
            return eEbusRst_NodeNotFound;
        }
    }
    break;

    default:
        LOG_E("[Ebus] Unknown message type: %d", msg_item->type);
        return eEbusRst_ParamErr;
    }
    return eEbusRst_Success;
}

/**
 * @description: 总线创建
 * @return {*}
 */
void EbusCreate(void)
{
    if (g_ebus_.init)
    {
        LOG_W("[Ebus] Bus already initialized");
        return;
    }

    LOG_D("[Ebus] Creating ebus...");
    rt_memset(&g_ebus_, 0x00, sizeof(g_ebus_));
    g_ebus_.init = 1;
    g_ebus_.node_len = 0;
    g_ebus_.bus_mutex = rt_mutex_create("ebusmtx", RT_IPC_FLAG_FIFO);
    if (g_ebus_.bus_mutex == RT_NULL)
    {
        LOG_E("[Ebus] Failed to create bus mutex");
        return;
    }
    LOG_D("[Ebus] Ebus created successfully");
}

/**
 * @description: 总线销毁
 * @return {*}
 */
void EbusDestory(void)
{
    if (!g_ebus_.init)
    {
        LOG_W("[Ebus] Bus not initialized, skip destroy");
        return;
    }

    LOG_D("[Ebus] Destroying ebus...");

    if (g_ebus_.bus_mutex)
    {
        rt_mutex_delete(g_ebus_.bus_mutex);
        g_ebus_.bus_mutex = RT_NULL;
        LOG_D("[Ebus] Bus mutex deleted");
    }

    g_ebus_.init = 0;
    rt_memset(&g_ebus_, 0x00, sizeof(g_ebus_));
    LOG_D("[Ebus] Ebus destroyed successfully");
}

/**
 * @description: 总线内节点创建
 * @param {char} *name
 * @param {AsyncIndicationCb} ackcb
 * @param {void} *user_data
 * @return {*}
 */
sEbusNode_t *EbusNodeCreate(char *name, EbusCbPtr EvtCb)
{
    if (name == RT_NULL || EvtCb == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for node creation");
        return RT_NULL;
    }

    LOG_D("[Ebus] Creating node: %s", name);

    sEbusNode_t *node = (sEbusNode_t *)rt_malloc(sizeof(sEbusNode_t));
    if (node == RT_NULL)
    {
        LOG_E("[Ebus] Failed to allocate memory for node: %s", name);
        return RT_NULL;
    }
    rt_memset(node, 0, sizeof(sEbusNode_t));

    rt_strncpy(node->name, name, EBUS_NAME_LEN - 1);
    node->name[EBUS_NAME_LEN - 1] = '\0';
    node->Evtcb = EvtCb;

    // 初始化等待响应列表
    for (int i = 0; i < EBUS_NODE_MAX_RESP_WAIT_NUM; i++)
    {
        node->wait_resp_list[i].state = eEbusMsgState_Idle;
    }

    // 创建响应互斥量
    node->resp_mutex = rt_mutex_create("respmtx", RT_IPC_FLAG_FIFO);
    if (node->resp_mutex == RT_NULL)
    {
        LOG_E("[Ebus] Failed to create response mutex for node: %s", name);
        rt_free(node);
        return RT_NULL;
    }

    // 创建消息队列
    char mq_name[EBUS_NAME_LEN] = { 0 };
    rt_snprintf(mq_name, EBUS_NAME_LEN, "%s_mq", name);
    node->msg_queue = rt_mq_create(mq_name, sizeof(sEbusMsgItem_t), EBUS_MAX_MSG_NUM, RT_IPC_FLAG_FIFO);
    if (node->msg_queue == RT_NULL)
    {
        LOG_E("[Ebus] Failed to create message queue for node: %s", name);
        rt_mutex_delete(node->resp_mutex);
        rt_free(node);
        return RT_NULL;
    }

    // 查找空闲槽位
    int idle_idx = EbusFindIdleIdx();
    if (idle_idx < 0)
    {
        LOG_E("[Ebus] No available slots for node: %s", name);
        rt_mq_delete(node->msg_queue);
        rt_mutex_delete(node->resp_mutex);
        rt_free(node);
        return RT_NULL;
    }

    // 注册到总线
    EbusBusInit((uint8_t)idle_idx, node);

    // 检查是否初始化成功
    if (!node->init)
    {
        LOG_E("[Ebus] Failed to initialize node: %s", name);
        EbusBusDeinit(idle_idx);
        rt_mq_delete(node->msg_queue);
        rt_mutex_delete(node->resp_mutex);
        rt_free(node);
        return RT_NULL;
    }

    LOG_D("[Ebus] Node created successfully: name=%s, idx=%d", node->name, node->node_idx);
    return node;
}

/**
 * @description: 总线内节点销毁
 * @param {sEbusNode_t} *node
 * @return {*}
 */
void EbusNodeDestory(sEbusNode_t *node)
{
    if (node == RT_NULL)
    {
        LOG_E("[Ebus] Attempt to destroy null node");
        return;
    }

    if (!node->init)
    {
        LOG_W("[Ebus] Node not initialized: %s", node->name);
        return;
    }

    LOG_D("[Ebus] Destroying node: %s, idx=%d", node->name, node->node_idx);

    // 从总线注销
    EbusBusDeinit(node->node_idx);

    // 删除消息队列和互斥量
    rt_mq_delete(node->msg_queue);
    LOG_D("[Ebus] Message queue deleted for node: %s", node->name);

    rt_mutex_delete(node->resp_mutex);
    LOG_D("[Ebus] Response mutex deleted for node: %s", node->name);

    // 清理节点数据
    node->node_idx = 0;
    node->init = 0;
    rt_free(node);

    LOG_D("[Ebus] Node destroyed successfully: %s", node->name);
}

/**
 * @description: 接收
 * @param {sEbusNode_t} *node
 * @param {sEbusMsgItem_t} *msg
 * @param {uint32_t} timeout
 * @return {*}
 */
eEbusRst_t EbusMsgWaitRecv(sEbusNode_t *node, sEbusMsgItem_t *msg, uint32_t timeout)
{
    if (node == RT_NULL || !node->init || msg == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for message receive");
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Waiting for message: node=%s, timeout=%d", node->name, timeout);

    rt_ssize_t len = rt_mq_recv(node->msg_queue, msg, sizeof(sEbusMsgItem_t), timeout);
    if (len > 0)
    {
        LOG_D("[Ebus] Message received: node=%s, type=%d, seq=%d, src=%d, dst=%d",
              node->name, msg->type, msg->seq_num, msg->src_node_idx, msg->dst_node_idx);

        if (msg->type == eEbusMsgType_Indication && node->Evtcb != RT_NULL)
        {
            LOG_D("[Ebus] Processing indication: seq=%d, src=%d, dst=%d",
                  msg->seq_num, msg->src_node_idx, msg->dst_node_idx);
            sEbusNode_t *ack_node = (sEbusNode_t *)EbusFindNodeByIdx(msg->src_node_idx);
            if (ack_node != RT_NULL)
            {
                node->Evtcb(eEBusEvtType_IndicationCb, node, msg, ack_node);
                LOG_D("[Ebus] Indication callback invoked");
            }
            else
            {
                LOG_W("[Ebus] Source node not found for indication: idx=%d", msg->src_node_idx);
            }
            return eEbusRst_OtherEvt;
        }
        else if (msg->type == eEbusMsgType_Response && node->Evtcb != RT_NULL)
        {
            LOG_D("[Ebus] Processing response: seq=%d, src=%d, dst=%d",
                  msg->seq_num, msg->src_node_idx, msg->dst_node_idx);
            node->Evtcb(eEBusEvtType_IndicationAckCb, node, msg, RT_NULL);
            EbusProcessResponse(node, msg);
            return eEbusRst_OtherEvt;
        }
        return eEbusRst_Success;
    }
    else if (len == -RT_ETIMEOUT)
    {
        LOG_D("[Ebus] Message receive timeout: node=%s", node->name);
        return eEbusRst_Timeout;
    }
    else
    {
        LOG_E("[Ebus] Message receive failed: node=%s, err=%d", node->name, len);
        return eEbusRst_Fail;
    }
}

/**
 * @description: 接收
 * @param {sEbusNode_t} *node
 * @param {sEbusMsgItem_t} *msg
 * @return {*}
 */
eEbusRst_t EbusMsgRecv(sEbusNode_t *node, sEbusMsgItem_t *msg)
{
    LOG_D("[Ebus] Try receive message: node=%s", node->name);
    return EbusMsgWaitRecv(node, msg, 0);
}

/**
 * @description: 消息广播
 * @param {sEbusNode_t} *node
 * @param {char} dst_node_name
 * @param {sEbusMsgItem_t} *msg
 * @return {*}
 */
eEbusRst_t EbusBroadcast(sEbusNode_t *node, sEbusMsgItem_t *msg)
{
    if (node == RT_NULL || !node->init || msg == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for broadcast");
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Broadcasting: from node=%s, evt=%x", node->name, msg->evt_id);

    msg->type = eEbusMsgType_Broadcast;
    msg->src_node_idx = node->node_idx;
    msg->dst_node_idx = 0xFF;
    msg->seq_num = EbusGetSn();
    msg->timestamp = rt_tick_get();

    return EbusMsgSend(node, msg);
}

/**
 * @description: 消息通知无应答
 * @param {sEbusNode_t} *node
 * @param {char} dst_node_name
 * @param {sEbusMsgItem_t} *msg
 * @return {*}
 */
eEbusRst_t EbusNotification(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg)
{
    if (node == RT_NULL || msg == RT_NULL || dst_node_name == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for notification");
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Sending notification: from=%s, to=%s, evt=%x",
          node->name, dst_node_name, msg->evt_id);

    sEbusNode_t *dst_node = EbusFindNodeByName(dst_node_name);
    if (dst_node == RT_NULL)
    {
        LOG_E("[Ebus] Target node not found for notification: %s", dst_node_name);
        return eEbusRst_NodeNotFound;
    }

    msg->type = eEbusMsgType_Notification;
    msg->src_node_idx = node->node_idx;
    msg->dst_node_idx = dst_node->node_idx;
    msg->seq_num = EbusGetSn();
    msg->timestamp = rt_tick_get();

    return EbusMsgSend(node, msg);
}

/**
 * @description: 异步Indication，通过回调通知结果
 * @param {sEbusNode_t} *node 发送节点
 * @param {char} *dst_node_name 目标节点名称
 * @param {sEbusMsgItem_t} *msg 发送的消息
 * @return {*} 执行结果
 */
eEbusRst_t EbusIndicationAsync(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg)
{
    if (node == RT_NULL || msg == RT_NULL || dst_node_name == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for async indication");
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Sending async indication: from=%s, to=%s, evt=%x",
          node->name, dst_node_name, msg->evt_id);

    sEbusNode_t *dst_node = EbusFindNodeByName(dst_node_name);
    if (dst_node == RT_NULL)
    {
        LOG_E("[Ebus] Target node not found for async indication: %s", dst_node_name);
        return eEbusRst_NodeNotFound;
    }

    // 分配等待响应项
    int wait_idx = EbusAllocWaitRespItem(node);
    if (wait_idx < 0)
    {
        LOG_E("[Ebus] No space for wait response: node=%s", node->name);
        return eEbusRst_NoMemory;
    }

    // 设置消息参数
    msg->type = eEbusMsgType_Indication;
    msg->src_node_idx = node->node_idx;
    msg->dst_node_idx = dst_node->node_idx;
    msg->seq_num = EbusGetSn();
    msg->timestamp = rt_tick_get();

    LOG_D("[Ebus] Async indication configured: seq=%d, wait_idx=%d", msg->seq_num, wait_idx);

    // 配置等待项
    rt_mutex_take(node->resp_mutex, RT_WAITING_FOREVER);
    sEbusWaitResp_t *wait_item = &node->wait_resp_list[wait_idx];
    wait_item->seq_num = msg->seq_num;
    wait_item->src_node_idx = node->node_idx;
    wait_item->dst_node_idx = dst_node->node_idx;
    wait_item->send_time = rt_tick_get();
    wait_item->state = eEbusMsgState_Sented;
    rt_mutex_release(node->resp_mutex);

    // 发送消息
    eEbusRst_t send_result = EbusMsgSend(node, msg);
    if (send_result != eEbusRst_Success)
    {
        LOG_E("[Ebus] Async indication send failed: result=%d", send_result);
        EbusFreeWaitRespItem(node, wait_idx);
    }
    else
    {
        LOG_D("[Ebus] Async indication sent: seq=%d, from=%s to %s",
              msg->seq_num, node->name, dst_node->name);
    }

    return send_result;
}

/**
 * @description: 响应消息发送
 * @param {sEbusNode_t} *node 发送节点（响应方）
 * @param {char} *dst_node_name 目标节点名称（请求方）
 * @param {sEbusMsgItem_t} *msg 响应消息
 * @return {*} 执行结果
 */
eEbusRst_t EbusResponse(sEbusNode_t *node, sEbusNode_t *ack_node, sEbusMsgItem_t *msg)
{
    if (node == RT_NULL || msg == RT_NULL || ack_node == RT_NULL)
    {
        LOG_E("[Ebus] Invalid parameters for response");
        return eEbusRst_ParamErr;
    }

    LOG_D("[Ebus] Sending response: from=%s, to=%s, seq=%d",
          node->name, ack_node->name, msg->seq_num);

    // 设置响应消息参数
    msg->type = eEbusMsgType_Response;
    msg->src_node_idx = node->node_idx;
    msg->dst_node_idx = ack_node->node_idx;
    msg->timestamp = rt_tick_get();

    // 更新等待响应项状态
    int idx = EbusFindWaitRespItem(ack_node, msg->seq_num);
    if (idx >= 0)
    {
        rt_mutex_take(ack_node->resp_mutex, RT_WAITING_FOREVER);
        sEbusWaitResp_t *wait_item = &ack_node->wait_resp_list[idx];
        if (wait_item->state == eEbusMsgState_Sented)
        {
            wait_item->state = eEbusMsgState_Recved;
            LOG_D("[Ebus] Wait item state updated to Recved: seq=%d", msg->seq_num);
        }
        else
        {
            LOG_W("[Ebus] Unexpected wait item state: seq=%d, state=%d",
                  msg->seq_num, wait_item->state);
        }
        rt_mutex_release(ack_node->resp_mutex);
    }
    else
    {
        LOG_W("[Ebus] Wait item not found for response: seq=%d, ack_node=%s",
              msg->seq_num, ack_node->name);
    }

    return EbusMsgSend(node, msg);
}

/* -------------------------------------------------------------------------- */
/*                                    finsh                                   */
/* -------------------------------------------------------------------------- */
/**
 * @description: 显示总线内所有节点的等待响应信息
 * @return {*}
 */
void ebus_show(void)
{
    if (!g_ebus_.init)
    {
        rt_kprintf("Ebus not initialized!\n");
        return;
    }

    uint32_t current_tick = rt_tick_get();
    rt_kprintf("Ebus Wait Response Info - Current tick: %d\n", current_tick);

    rt_mutex_take(g_ebus_.bus_mutex, RT_WAITING_FOREVER);

    for (int node_idx = 0; node_idx < EBUS_MAX_NODE_NUM; node_idx++)
    {
        sEbusNode_t *node = g_ebus_.node_tbl[node_idx];
        if (node == RT_NULL || !node->init)
        {
            continue;
        }

        rt_mutex_take(node->resp_mutex, RT_WAITING_FOREVER);

        rt_kprintf("\nNode: %s (ID:%d)\n", node->name, node->node_idx);

        int active_count = 0;
        for (int slot_idx = 0; slot_idx < EBUS_NODE_MAX_RESP_WAIT_NUM; slot_idx++)
        {
            sEbusWaitResp_t *item = &node->wait_resp_list[slot_idx];

            if (item->state != eEbusMsgState_Idle)
            {
                active_count++;
                uint32_t wait_time = current_tick - item->send_time;
                uint32_t wait_ms = wait_time * (1000 / RT_TICK_PER_SECOND);

                const char *state_str;
                switch (item->state)
                {
                case eEbusMsgState_Sented:
                    state_str = "SENTED";
                    break;
                case eEbusMsgState_Recved:
                    state_str = "RECVED";
                    break;
                default:
                    state_str = "IDLE";
                    break;
                }

                rt_kprintf("  Slot[%d]: Seq=0x%04X, State=%s, Src=%d->Dst=%d, SendTime=%d, Wait=%dms\n",
                           slot_idx,
                           item->seq_num,
                           state_str,
                           item->src_node_idx,
                           item->dst_node_idx,
                           item->send_time,
                           wait_ms);
            }
        }

        if (active_count == 0)
        {
            rt_kprintf("  No active wait responses\n");
        }
        else
        {
            rt_kprintf("  Active slots: %d/%d\n", active_count, EBUS_NODE_MAX_RESP_WAIT_NUM);
        }

        rt_mutex_release(node->resp_mutex);
    }

    rt_mutex_release(g_ebus_.bus_mutex);
    rt_kprintf("End of ebus wait response info\n");
}
MSH_CMD_EXPORT(ebus_show, show all ebus wait response info);

