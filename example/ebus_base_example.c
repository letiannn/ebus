#include "ebus.h"

#define LOG_TAG "ebus_base_example"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 3192
#define THREAD_TIMESLICE  5

#define NODE11_NAME "Node11"
#define NODE22_NAME "Node22"
#define NODE33_NAME "Node33"
#define NODE44_NAME "Node44"

static volatile int g_example_running = 1;

enum
{
    EbusEvtId_Broadcast = 0x8001,
    EbusEvtId_P2p,
    EbusEvtId_Sync,
    EbusEvtId_Async,
    EbusEvtId_Ack,
};

static void Node1Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if (evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if (evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg;
        rt_memset(&resp_msg, 0, sizeof(resp_msg));
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        EbusResponse(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch (msg->evt_id)
        {
        case EbusEvtId_Broadcast:
        {
        }
        break;
        case EbusEvtId_P2p:
        {
        }
        break;
        }
    }
}

static void Node2Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if (evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if (evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg;
        rt_memset(&resp_msg, 0, sizeof(resp_msg));
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        EbusResponse(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch (msg->evt_id)
        {
        case EbusEvtId_Broadcast:
        {
        }
        break;
        case EbusEvtId_P2p:
        {
        }
        break;
        }
    }
}

static void Node3Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if (evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if (evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg;
        rt_memset(&resp_msg, 0, sizeof(resp_msg));
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        EbusResponse(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch (msg->evt_id)
        {
        case EbusEvtId_Broadcast:
        {
        }
        break;
        case EbusEvtId_P2p:
        {
        }
        break;
        }
    }
}

static void Node4Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if (evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if (evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg;
        rt_memset(&resp_msg, 0, sizeof(resp_msg));
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        EbusResponse(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch (msg->evt_id)
        {
        case EbusEvtId_Broadcast:
        {
        }
        break;
        case EbusEvtId_P2p:
        {
        }
        break;
        }
    }
}

static void thread1_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg;
    rt_memset(&rx_msg, 0, sizeof(rx_msg));
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE11_NAME, Node1Cb);
    while (g_example_running)
    {
        eEbusRst_t rst = EbusMsgRecv(node, &rx_msg);
        while (rst == eEbusRst_Success && g_example_running)
        {
            if (node && node->init)
            {
                node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            }
            rst = EbusMsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
    if (node && node->init)
    {
        EbusNodeDestory(node);
    }
}

static void thread2_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg;
    rt_memset(&rx_msg, 0, sizeof(rx_msg));
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE22_NAME, Node2Cb);
    while (g_example_running)
    {
        eEbusRst_t rst = EbusMsgRecv(node, &rx_msg);
        while (rst == eEbusRst_Success && g_example_running)
        {
            if (node && node->init)
            {
                node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            }
            rst = EbusMsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
    if (node && node->init)
    {
        EbusNodeDestory(node);
    }
}

static void thread3_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg;
    rt_memset(&rx_msg, 0, sizeof(rx_msg));
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE33_NAME, Node3Cb);
    while (g_example_running)
    {
        eEbusRst_t rst = EbusMsgRecv(node, &rx_msg);
        while (rst == eEbusRst_Success && g_example_running)
        {
            if (node && node->init)
            {
                node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            }
            rst = EbusMsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
    if (node && node->init)
    {
        EbusNodeDestory(node);
    }
}

static void thread4_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg;
    rt_memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.len = 8;
    rt_memset(tx_msg.data, 0x55, tx_msg.len);
    uint32_t i = 0;
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE44_NAME, Node4Cb);
    while (++i)
    {
        rt_thread_mdelay(1000);
        if (node && node->init)
        {
            tx_msg.evt_id = (uint16_t)EbusEvtId_P2p;
            LOG_I("send EbusNotification %d ", EbusNotification(node, NODE11_NAME, &tx_msg));
        }
        rt_thread_mdelay(1000);
        if (node && node->init)
        {
            tx_msg.evt_id = (uint16_t)EbusEvtId_Broadcast;
            LOG_I("send EbusBroadcast %d ", EbusBroadcast(node, &tx_msg));
        }
        if(i > 10)
        {
            break;
        }
    }

    if (node && node->init)
    {
        EbusNodeDestory(node);
    }

    rt_thread_mdelay(100);
    g_example_running = 0;
    rt_thread_mdelay(200);

    EbusDestory();
}

void ebus_base_example(void)
{
    EbusCreate();

    rt_thread_t tid1 = rt_thread_create("thread1",
                                        thread1_entry, RT_NULL,
                                        THREAD_STACK_SIZE,
                                        THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_t tid2 = rt_thread_create("thread2",
                                        thread2_entry, RT_NULL,
                                        THREAD_STACK_SIZE,
                                        THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_t tid3 = rt_thread_create("thread3",
                                        thread3_entry, RT_NULL,
                                        THREAD_STACK_SIZE,
                                        THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_t tid4 = rt_thread_create("thread4",
                                        thread4_entry, RT_NULL,
                                        THREAD_STACK_SIZE,
                                        THREAD_PRIORITY, THREAD_TIMESLICE);

    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);
    if (tid4 != RT_NULL)
        rt_thread_startup(tid4);

    // EbusDestory();
}
MSH_CMD_EXPORT(ebus_base_example, ebus base example);
