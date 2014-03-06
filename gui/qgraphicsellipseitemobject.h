#ifndef QGRAPHICSELLIPSEITEMOBJECT_H
#define QGRAPHICSELLIPSEITEMOBJECT_H

#include <QObject>
#include <QGraphicsEllipseItem>

/**
 * @brief The QGraphicsEllipseItemObject class Wrapper for QGraphicsEllipseItem that makes it a QObject.
 */
class QGraphicsEllipseItemObject : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit QGraphicsEllipseItemObject(QObject *parent = 0);

signals:

public slots:

};

#endif // QGRAPHICSELLIPSEITEMOBJECT_H
