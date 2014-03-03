#include "clustercanvas.h"
#include "ui_clustercanvas.h"

#include <QGraphicsEllipseItem>

ClusterCanvas::ClusterCanvas(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClusterCanvas)
{
    ui->setupUi(this);

    m_view = ui->graphicsView;
    m_scene = new QGraphicsScene(m_view);
    m_view->setScene(m_scene);
}

ClusterCanvas::~ClusterCanvas()
{
    for (size_t i = m_items.size() - 1; i >= 0; i--) {
        ClusterItem* item = m_items[i];
        delete item;
        m_items.pop_back();
    }

    for (size_t i = m_agents.size() - 1; i >= 0; i--) {
        Agent* agent = m_agents[i];
        delete agent;
        m_agents.pop_back();
    }

    delete ui;
}


void ClusterCanvas::cluster(std::vector<ClusterItem*> items, std::vector<Agent*> agents) {
    if (items.size() == 0)
        return;

    m_scene->clear();
    show();

    m_items = items;
    m_agents = agents;

    //Shift everything so that (0, 0) is the smallest x/y position. For viewing purposes
    double minX = m_items[0]->x;
    double minY = m_items[0]->y;
    double maxX = minX;
    double maxY = minY;
    for (int i = 0; i < m_items.size(); i++) {
        ClusterItem* item = m_items[i];
        if (item->x < minX)
            minX = item->x;
        else if (item->x > maxX)
            maxX = item->x;

        if (item->y < minY)
            minY = item->y;
        else if (item->y > maxY)
            maxY = item->y;
    }
    double rangeX = maxX - minX;
    double rangeY = maxY - minY;

    qreal maxWidth = m_view->width();
    qreal maxHeight = m_view->height();

    for (int i = 0; i < m_items.size(); i++) {
        m_items[i]->x = ((maxWidth * (m_items[i]->x - minX)) / (rangeX)) + maxWidth;
        m_items[i]->y = ((maxHeight * (m_items[i]->y - minY)) / (rangeY)) + maxHeight;
    }

    QBrush brush(Qt::blue);
    QPen pen(Qt::black, 1);

    for (int i = 0; i < m_items.size(); i++) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
        ClusterItem* clusterItem = m_items.at(i);

        item->setRect(QRectF(clusterItem->x, clusterItem->y, 2.5, 2.5));
        item->setBrush(brush);
        item->setPen(pen);
        m_scene->addItem(item);
        item->show();
    }
}
