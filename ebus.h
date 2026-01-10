#ifndef _EBUS_H_
#define _EBUS_H_

#include "rtthread.h"

#define EBUS_NAME_LEN               (32)    //ebus名称长度
#define EBUS_MAX_NODE_NUM           (10)    //ebus节点数量
#define EBUS_MAX_MSG_SIZE           (8)     //消息最大长度
#define EBUS_MAX_MSG_NUM            (10)    //消息数量
#define EBUS_NODE_MAX_RESP_WAIT_NUM (10)    //节点最大的等待回应数量
#define EBUS_RESPONSE_WAIT_TIME_MS  (1000)  //节点最大等待时间

/*** 
 * @description: 指示消息状态
 * @return {*}
 */
typedef enum eEbusMsgStateTag
{
    eEbusMsgState_Idle,     //消息空闲
    eEbusMsgState_Sented,   //消息已经发送
    eEbusMsgState_Recved,   //指示已经被接收
} eEbusMsgState_t;

/**
 * @description: 错误类型
 */
typedef enum eEbusRstTag
{
    eEbusRst_Success = 0,
    eEbusRst_Fail,
    eEbusRst_Timeout,
    eEbusRst_NoMemory,
    eEbusRst_ParamErr,
    eEbusRst_NodeNotFound,
    eEbusRst_OtherEvt,
    eEbusRst_QueueFull,
} eEbusRst_t;

/**
 * @description: 消息类型
 * @return {*}
 */
typedef enum eEbusMsgTypeTag
{
    eEbusMsgType_Broadcast = 0,             //广播
    eEbusMsgType_Notification,              //通知 无应答
    eEbusMsgType_Indication,                //指示 需应答
    eEbusMsgType_Response,                  //响应
} eEbusMsgType_t;

/*** 
 * @description: 事件类型
 * @return {*}
 */
typedef enum eEbusEvtTypeTag
{
    eEbusEvtType_RecvCb = 0,                //接收回调
    eEBusEvtType_IndicationCb,              //接收指示回调
    eEBusEvtType_IndicationAckCb,           //指示应答回调
} eEbusEvtType_t;

typedef struct sEbusNodeTag sEbusNode_t;
typedef struct sEbusMsgItemTag sEbusMsgItem_t;
typedef void (*EbusCbPtr)(eEbusEvtType_t evt, sEbusNode_t *node, sEbusMsgItem_t *msg, void *user_data);

/**
 * @description: 总线消息信息
 * @return {*}
 */
struct sEbusMsgItemTag
{
    eEbusMsgType_t type;                   //消息类型
    uint8_t src_node_idx;           //事件源id
    uint8_t dst_node_idx;           //事件目标id
    rt_tick_t timestamp;              //时间戳
    uint16_t seq_num;                //序列号
    uint16_t evt_id;                 //事件id
    uint8_t len;                    //数据长度
    uint8_t data[EBUS_MAX_MSG_SIZE];//数据指针
};

/**
 * @description: 等待响应的请求项
 */
typedef struct sEbusWaitRespTag
{
    uint16_t seq_num;        // 序列号
    uint8_t src_node_idx;   // 源节点ID
    uint8_t dst_node_idx;   // 目标节点ID
    rt_tick_t send_time;      // 发送时间
    eEbusMsgState_t state;          // 状态
} sEbusWaitResp_t;

/**
 * @description: 总线节点数据
 */
struct sEbusNodeTag
{
    uint8_t init;                   //是否初始化
    char name[EBUS_NAME_LEN];    //总线名称
    uint8_t node_idx;               //总线id
    rt_mq_t msg_queue;              //消息队列
    EbusCbPtr Evtcb;                  //回调接口
    sEbusWaitResp_t wait_resp_list[EBUS_NODE_MAX_RESP_WAIT_NUM];
    rt_mutex_t resp_mutex;             //响应管理互斥锁
};

/**
 * @description: 总线整体信息
 */
typedef struct sEbusTag
{
    uint8_t init;                           //是否初始化
    rt_mutex_t bus_mutex;                      //总线互斥量
    uint16_t sn;                             //总线序列号
    uint8_t node_len;                       //总线数量
    sEbusNode_t *node_tbl[EBUS_MAX_NODE_NUM];   //总线表
} sEbus_t;

void EbusCreate(void);

void EbusDestory(void);

sEbusNode_t *EbusNodeCreate(char *name, EbusCbPtr EvtCb);

void EbusNodeDestory(sEbusNode_t *node);

eEbusRst_t EbusMsgWaitRecv(sEbusNode_t *node, sEbusMsgItem_t *msg, uint32_t timeout);

eEbusRst_t EbusMsgRecv(sEbusNode_t *node, sEbusMsgItem_t *msg);

eEbusRst_t EbusBroadcast(sEbusNode_t *node, sEbusMsgItem_t *msg);

eEbusRst_t EbusNotification(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg);

eEbusRst_t EbusIndicationAsync(sEbusNode_t *node, char *dst_node_name, sEbusMsgItem_t *msg);

eEbusRst_t EbusResponse(sEbusNode_t *node, sEbusNode_t *ack_node, sEbusMsgItem_t *msg);

#endif
