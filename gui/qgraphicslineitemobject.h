#ifndef QGRAPHICSLINEITEMOBJECT_H
#define QGRAPHICSLINEITEMOBJECT_H

#include <QObject>
#include <QGraphicsLineItem>

/**
 * @brief The QGraphicsLineItemObject class Wrapper for QGraphicsLineItem that makes it a QObject.
 */
class QGraphicsLineItemObject : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit QGraphicsLineItemObject(QObject *parent = 0);

signals:

public slots:

};

#endif // QGRAPHICSLINEITEMOBJECT_H
