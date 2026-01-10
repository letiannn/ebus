#include "ebus.h"

#define LOG_TAG "ebus_ack_example"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE  5

#define NODE1_NAME "Node1"
#define NODE2_NAME "Node2"
#define NODE3_NAME "Node3"
#define NODE4_NAME "Node4"

static volatile int g_example_running = 1;

typedef enum EbusEvtIdTag
{
    EbusEvtId_Broadcast = 0x8001,
    EbusEvtId_P2p,
    EbusEvtId_Sync,
    EbusEvtId_Async,
    EbusEvtId_Ack,
}EbusEvtId_t;

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
        sEbusMsgItem_t resp_msg = { 0 };
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
        sEbusMsgItem_t resp_msg = { 0 };
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
        sEbusMsgItem_t resp_msg = { 0 };
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
        sEbusMsgItem_t resp_msg = { 0 };
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
    sEbusMsgItem_t rx_msg = { 0 };
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE1_NAME, Node1Cb);
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
    EbusNodeDestory(node);
}

static void thread2_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = { 0 };
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE2_NAME, Node2Cb);
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
    EbusNodeDestory(node);
}

static void thread3_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = { 0 };
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE3_NAME, Node3Cb);
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
    EbusNodeDestory(node);
}

static void thread4_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg = { 0 };
    tx_msg.len = 8;
    rt_memset(tx_msg.data, 0x55, tx_msg.len);

    sEbusMsgItem_t rx_msg = { 0 };
    sEbusNode_t *node = (sEbusNode_t *)EbusNodeCreate(NODE4_NAME, Node4Cb);

    uint32_t i = 0;
    while (++i)
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

        if (i % 10 == 0 && node && node->init)
        {
            tx_msg.evt_id = (uint16_t)EbusEvtId_Sync;
            eEbusRst_t result = EbusIndicationAsync(node, NODE1_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE1_NAME);

            tx_msg.evt_id = (uint16_t)EbusEvtId_Sync;
            result = EbusIndicationAsync(node, NODE2_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE2_NAME);

            tx_msg.evt_id = (uint16_t)EbusEvtId_Sync;
            result = EbusIndicationAsync(node, NODE3_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE3_NAME);
        }
        if(i % 1000 == 0)
        {
            break;
        }
        rt_thread_mdelay(10);
    }

    EbusNodeDestory(node);

    rt_thread_mdelay(100);
    g_example_running = 0;
    rt_thread_mdelay(200);

    EbusDestory();
}

void ebus_ack_example(void)
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

}
MSH_CMD_EXPORT(ebus_ack_example, ebus ack example);
