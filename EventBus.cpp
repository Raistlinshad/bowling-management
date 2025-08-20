// EventBus.cpp
#include "EventBus.h"

EventBus::EventBus(QObject *parent) : QObject(parent) {}

bool EventBus::publish(const QString &channel, const QString &event, const QJsonObject &data)
{
    emit eventReceived(channel, event, data);
    return true;
}