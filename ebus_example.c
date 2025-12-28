#include "ebus.h"

#define LOG_TAG              "ebus_example"
#define LOG_LVL              LOG_LVL_WARNING
#include <ulog.h>


#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       2048
#define THREAD_TIMESLICE        5

#define BASE_EXAMPLE    0
#define ASYNC_EXAMPLE   1

#define NODE1_NAME "Node1"
#define NODE2_NAME "Node2"
#define NODE3_NAME "Node3"
#define NODE4_NAME "Node4"

enum {
    EbusEvtId_Broadcast = 0x8001,
    EbusEvtId_P2p,
    EbusEvtId_Sync,
    EbusEvtId_Async,
    EbusEvtId_Ack,
};

void Node1Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if(evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if(evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg = {0};
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        Ebus_Response(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch(msg->evt_id)
        {
            case EbusEvtId_Broadcast:{

            }
            break;
            case EbusEvtId_P2p:{
                
            }
            break;
        }
    }
}

void Node2Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if(evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if(evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg = {0};
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        Ebus_Response(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch(msg->evt_id)
        {
            case EbusEvtId_Broadcast:{

            }
            break;
            case EbusEvtId_P2p:{
                
            }
            break;
        }
    }
}

void Node3Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if(evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if(evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg = {0};
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        Ebus_Response(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch(msg->evt_id)
        {
            case EbusEvtId_Broadcast:{

            }
            break;
            case EbusEvtId_P2p:{
                
            }
            break;
        }
    }
}

void Node4Cb(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data)
{
    if(evt == eEBusEvtType_IndicationAckCb)
    {
        LOG_I("indication ack cb node name:%s success", node->name);
    }
    else if(evt == eEBusEvtType_IndicationCb)
    {
        sEbusNode_t *ack_node = (sEbusNode_t *)user_data;
        LOG_I("indication cb node name:%s wantack:%s evt:%x", node->name, ack_node->name, msg->evt_id);
        // 创建响应消息
        sEbusMsgItem_t resp_msg = {0};
        resp_msg.evt_id = msg->evt_id;
        resp_msg.len = msg->len;
        rt_memcpy(resp_msg.data, msg->data, resp_msg.len);
        resp_msg.seq_num = msg->seq_num;
        Ebus_Response(node, ack_node, &resp_msg);
    }
    else
    {
        LOG_I("recv node name:%s evt:%x", node->name, msg->evt_id);
        switch(msg->evt_id)
        {
            case EbusEvtId_Broadcast:{

            }
            break;
            case EbusEvtId_P2p:{
                
            }
            break;
        }
    }
}

#if BASE_EXAMPLE


static void thread1_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE1_NAME, Node1Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread2_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE2_NAME, Node2Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread3_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE3_NAME, Node3Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread4_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg = {0};
       
    tx_msg.len = 8;              
    rt_memset(tx_msg.data, 0x55, tx_msg.len);

    sEbusNode_t *node = Ebus_NodeCreate(NODE4_NAME, Node4Cb);
    while (1)
    {
        tx_msg.evt_id = EbusEvtId_P2p;   
        LOG_I("send Ebus_Notification %d ", Ebus_Notification(node, NODE1_NAME, &tx_msg));
        rt_thread_mdelay(1000);
        tx_msg.evt_id = EbusEvtId_Broadcast; 
        LOG_I("send Ebus_Broadcast %d ", Ebus_Broadcast(node, &tx_msg));
        rt_thread_mdelay(1000);
    }
}

/* 已在上方定义 thread1_entry/thread2_entry，移除重复定义以避免重定义错误 */

#elif ASYNC_EXAMPLE 

static void thread1_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE1_NAME, Node1Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread2_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE2_NAME, Node2Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread3_entry(void *parameter)
{
    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE3_NAME, Node3Cb);
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        rt_thread_mdelay(10);
    }
}

static void thread4_entry(void *parameter)
{
    sEbusMsgItem_t tx_msg = {0};
    tx_msg.len = 8;              
    rt_memset(tx_msg.data, 0x55, tx_msg.len);

    sEbusMsgItem_t rx_msg = {0};
    sEbusNode_t *node = Ebus_NodeCreate(NODE4_NAME, Node4Cb);

    uint32_t i = 0;
    while (1)
    {
        eEbusRst_t rst = Ebus_MsgRecv(node, &rx_msg);
        while(rst == eEbusRst_Success)
        {
            node->Evtcb(eEbusEvtType_RecvCb, node, &rx_msg, NULL);
            rst = Ebus_MsgRecv(node, &rx_msg);
        }
        i++;
        if(i%10 == 0)
        {
            tx_msg.evt_id = EbusEvtId_Sync;   
            eEbusRst_t result = Ebus_IndicationAsync(node, NODE1_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE1_NAME);

            tx_msg.evt_id = EbusEvtId_Sync;   
            result = Ebus_IndicationAsync(node, NODE2_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE2_NAME);

            tx_msg.evt_id = EbusEvtId_Sync;   
            result = Ebus_IndicationAsync(node, NODE3_NAME, &tx_msg);
            LOG_I("send indication success send:%s recv:%s", node->name, NODE3_NAME);
        }
        rt_thread_mdelay(10);
    }
}
#endif

void ebus_example(void)
{
    Ebus_Create();

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
    
    // Ebus_Destory();
}
MSH_CMD_EXPORT(ebus_example, ebus example);
