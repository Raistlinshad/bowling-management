// EventBus.h
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QObject>
#include <QJsonObject>

class EventBus : public QObject
{
    Q_OBJECT

public:
    explicit EventBus(QObject *parent = nullptr);
    bool publish(const QString &channel, const QString &event, const QJsonObject &data);

signals:
    void eventReceived(const QString &channel, const QString &event, const QJsonObject &data);
};

#endif // EVENTBUS_H