#include "MessageCallback.h"


MessageInterface::CallBack::CallBackRequestGet MessageInterface::CallBack::_callBackRequestGet = NULL;
MessageInterface::CallBack::CallBackResponseGet MessageInterface::CallBack::_callBackResponseGet = NULL;
MessageInterface::CallBack::CallBackRequestSet MessageInterface::CallBack::_callBackRequestSet = NULL;
MessageInterface::CallBack::CallBackSubscribe MessageInterface::CallBack::_callBackSubscribe = NULL;
MessageInterface::CallBack::CallBackUnsubscribe MessageInterface::CallBack::_callBackUnsubscribe = NULL;
MessageInterface::CallBack::CallBackNotification MessageInterface::CallBack::_callBackNotification = NULL;
MessageInterface::CallBack::CallBackAcknowledge MessageInterface::CallBack::_callBackAcknowledge = NULL;

void MessageInterface::CallBack::registerRequestGetCallBack(const CallBackRequestGet func)
{
    _callBackRequestGet = func;
}

void MessageInterface::CallBack::invokeRequestGetCallBack(uint32_t seqId, const RequestGetData *dataArr, size_t arrSize)
{
    if (_callBackRequestGet == NULL)
        return;
    _callBackRequestGet(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerResponseGetCallBack(const CallBackResponseGet func)
{
    _callBackResponseGet = func;
}

void MessageInterface::CallBack::invokeResponseGetCallBack(uint32_t seqId, const ResponseGetData *dataArr, size_t arrSize)
{
    if (_callBackResponseGet == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        MessageInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackResponseGet(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerNotificationCallBack(const CallBackNotification func)
{
    _callBackNotification = func;
}

void MessageInterface::CallBack::invokeNotificationCallBack(uint32_t seqId, const NotificationData *dataArr, size_t arrSize)
{
    if (_callBackNotification == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        MessageInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackNotification(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerSubscribeCallBack(const CallBackSubscribe func)
{
    _callBackSubscribe = func;
}

void MessageInterface::CallBack::invokeSubscribeCallBack(uint32_t seqId, const SubscribeData *dataArr, size_t arrSize)
{
    if (_callBackSubscribe == NULL)
        return;
    _callBackSubscribe(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerUnsubscribeCallBack(const CallBackUnsubscribe func)
{
    _callBackUnsubscribe = func;
}

void MessageInterface::CallBack::invokeUnsubscribeCallBack(uint32_t seqId, const UnsubscribeData *dataArr, size_t arrSize)
{
    if (_callBackUnsubscribe == NULL)
        return;
    _callBackUnsubscribe(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerRequestSetCallBack(const CallBackRequestSet func)
{
    _callBackRequestSet = func;
}

void MessageInterface::CallBack::invokeRequestSetCallBack(uint32_t seqId, const RequestSetData *dataArr, size_t arrSize)
{
    if (_callBackRequestSet == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        MessageInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackRequestSet(seqId, dataArr, arrSize);
}

void MessageInterface::CallBack::registerAcknowledgeCallBack(const CallBackAcknowledge func)
{
    _callBackAcknowledge = func;
}

void MessageInterface::CallBack::invokeAcknowledgeCallBack(uint32_t seqId, unsigned char msgId)
{
    if (_callBackAcknowledge == NULL)
        return;
    _callBackAcknowledge(seqId, msgId);
}
