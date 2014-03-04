#include "clustercanvas.h"
#include "agentcluster.h"
#include "ui_clustercanvas.h"

#include <QGraphicsEllipseItem>
#include <QThread>

#include <stdio.h>

/**
 * @brief ClusterCanvas::ClusterCanvas Visualization class that takes care of showing ClusterItem
 * and Agent objects on a scaled 2d canvas.
 * @param dataSource Filename of csv file containing cluster data
 * @param iterations Number of iterations to run for.
 * @param parent Parent QObject
 */
ClusterCanvas::ClusterCanvas(std::string dataSource, int iterations, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClusterCanvas)
{
    ui->setupUi(this);

    m_view = ui->graphicsView;
    m_scene = new QGraphicsScene(m_view);
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing, true);

    m_minX = m_maxX = m_minY = m_maxY = -1;
    m_calculatedDataRange = false;

    m_clusterThread = new QThread(this);
    m_cluster = new AgentCluster(iterations, 0);
    m_cluster->moveToThread(m_clusterThread);
    connect(m_clusterThread, SIGNAL(started()), m_cluster, SLOT(start()));
    connect(m_cluster, SIGNAL(update(std::vector<ClusterItem*>*,std::vector<Agent*>*)),
            SLOT(updateDisplay(std::vector<ClusterItem*>*,std::vector<Agent*>*)));
    if (!m_cluster->loadData(dataSource)) {
        printf("Error: unable to open data file: %s\n\n", dataSource.c_str());
        delete m_cluster;
        delete m_clusterThread;
        m_cluster = 0;
        m_clusterThread = 0;
    } else
        printf("...loaded data: %i points\n", (int)m_cluster->dataCount());
}

ClusterCanvas::~ClusterCanvas()
{
    printf("deleting ClusterCanvas...\n");
    if (m_scene)
        m_scene->clear();

    if (m_cluster)
        delete m_cluster;

    m_agentItems.clear();
    m_dataItems.clear();


    delete ui;
}

/**
 * @brief ClusterCanvas::run Start the AgentCluster object.
 */
void ClusterCanvas::run() {
    if (m_cluster && m_clusterThread)
        m_clusterThread->start();
}

/**
 * @brief ClusterCanvas::updateDisplay Display the given ClusterItems and Agents to a QGraphicsView for
 * visual inspection.
 * @param items A vector of ClusterItem objects. This is the dataset that the algorithm clustered.
 * @param agents A vector of Agent objects. These are displayed larger than the ClusterItems.
 */
void ClusterCanvas::updateDisplay(std::vector<ClusterItem*>* items, std::vector<Agent*>* agents) {
    if (items->size() == 0)
        return;


    if (!isVisible()) {
        show();
        QRectF boundingArea =QRectF(0, 0, 800, 800);
        m_view->setSceneRect(boundingArea);
    }


    while (m_agentItems.size() != 0) {
        QGraphicsEllipseItem* item = m_agentItems[m_agentItems.size() - 1];
        item->hide();
        m_scene->removeItem(item);
        delete item;
        m_agentItems.pop_back();
    }

    //Shift everything so that (0, 0) is the smallest x/y position. For viewing purposes
    if (!m_calculatedDataRange) {
        m_minX = (*items)[0]->x;
        m_minY = (*items)[0]->y;
        m_maxX = m_minX;
        m_maxY = m_minY;
        for (unsigned int i = 0; i < items->size(); i++) {
            ClusterItem* item = (*items)[i];
            if (item->x < m_minX)
                m_minX = item->x;
            else if (item->x > m_maxX)
                m_maxX = item->x;

            if (item->y < m_minY)
                m_minY = item->y;
            else if (item->y > m_maxY)
                m_maxY = item->y;
        }
        m_calculatedDataRange = true;

        printf("\tupdated data in view...\n");
    } else
        printf("\tnot updating data in view...\n");

    double rangeX = m_maxX - m_minX;
    double rangeY = m_maxY - m_minY;

    qreal minWidth = m_view->mapToScene(0, 0).x();
    qreal minHeight = m_view->mapToScene(0, 0).y();
    qreal maxWidth =m_view->mapToScene(m_view->width(), m_view->width()).x();
    qreal maxHeight = m_view->mapToScene(m_view->height(), m_view->height()).x();


    QBrush brush(Qt::blue);
    QPen pen(Qt::black, 1);

    if (m_dataItems.size() == 0) {
        for (unsigned int i = 0; i < items->size(); i++) {
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
            ClusterItem* clusterItem = (*items)[i];
            double pX = ((maxWidth - minWidth) * (clusterItem->x - m_minX)) / (rangeX) + minWidth;
            double pY = ((maxHeight - minHeight) * (clusterItem->y - m_minY)) / (rangeY) + minHeight;

            item->setRect(QRectF(pX, pY, 2.5, 2.5));
            item->setBrush(brush);
            item->setPen(pen);
            m_scene->addItem(item);
            item->show();
            m_dataItems.push_back(item);
        }
    }


    //Draw the agents...
    brush.setColor(Qt::red);
    for (unsigned int i = 0; i < agents->size(); i++) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
        Agent* agent = (*agents)[i];
        double pX = ((maxWidth - minWidth) * (agent->x - m_minX)) / (rangeX) + minWidth;
        double pY = ((maxHeight - minHeight) * (agent->y - m_minY)) / (rangeY) + minHeight;

        item->setRect(pX, pY, 2, 2);
        item->setBrush(brush);
        item->setPen(pen);
        m_scene->addItem(item);
        item->show();
        m_agentItems.push_back(item);
    }

    //Draw the effective range...
    brush.setColor(Qt::transparent);
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::DashLine);
    for (unsigned int i = 0; i < agents->size(); i++) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
        Agent* agent = (*agents)[i];

        double radius = agent->effectiveRange;
        double topLeftX = ((maxWidth - minWidth) * ((agent->x - radius) - m_minX)) / (rangeX) + minWidth;
        double topLeftY = ((maxHeight - minHeight) * ((agent->y - radius) - m_minY)) / (rangeY) + minHeight;

        double bottomRightX = ((maxWidth - minWidth) * ((agent->x + radius) - m_minX)) / (rangeX) + minWidth;
        double bottomRightY = ((maxHeight - minHeight) * ((agent->y + radius) - m_minY)) / (rangeY) + minHeight;

        QPointF topLeft = QPointF(topLeftX, topLeftY);
        QPointF bottomRight = QPointF(bottomRightX, bottomRightY);
        QRectF ellipseRect(topLeft, bottomRight);


        item->setRect(ellipseRect);
        item->setBrush(brush);
        item->setPen(pen);
        m_scene->addItem(item);
        item->show();
        m_agentItems.push_back(item);
    }

    //Draw personal space....
    pen.setColor(Qt::red);
    for (unsigned int i = 0; i < agents->size(); i++) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
        Agent* agent = (*agents)[i];
        double radius = agent->personalSpace;
        double topLeftX = ((maxWidth - minWidth) * ((agent->x - radius) - m_minX)) / (rangeX) + minWidth;
        double topLeftY = ((maxHeight - minHeight) * ((agent->y - radius) - m_minY)) / (rangeY) + minHeight;

        double bottomRightX = ((maxWidth - minWidth) * ((agent->x + radius) - m_minX)) / (rangeX) + minWidth;
        double bottomRightY = ((maxHeight - minHeight) * ((agent->y + radius) - m_minY)) / (rangeY) + minHeight;

        QPointF topLeft = QPointF(topLeftX, topLeftY);
        QPointF bottomRight = QPointF(bottomRightX, bottomRightY);
        QRectF ellipseRect(topLeft, bottomRight);

        item->setRect(ellipseRect);
        item->setPen(pen);
        m_scene->addItem(item);
        item->show();
        m_agentItems.push_back(item);
    }
}
