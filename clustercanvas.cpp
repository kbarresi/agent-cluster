#include "clustercanvas.h"
#include "agentcluster.h"
#include "ui_clustercanvas.h"

#include <QGraphicsEllipseItem>
#include <QThread>

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
        printf("Error: unable to open data file: %s\n\n", dataSource);
        delete m_cluster;
        delete m_clusterThread;
        m_cluster = 0;
        m_clusterThread = 0;
    } else
        printf("...loaded data: %i points\n", m_cluster->dataCount());
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
        QRectF boundingArea = QRectF(0, 0, 800, 600);
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
        for (int i = 0; i < items->size(); i++) {
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

    for (int i = 0; i < items->size(); i++) {
        (*items)[i]->x = ((maxWidth - minWidth) * (items->at(i)->x - m_minX)) / (rangeX) + minWidth;
        (*items)[i]->y = ((maxHeight - minHeight) * (items->at(i)->y - m_minY)) / (rangeY) + minHeight;
    }
    for (int i = 0; i < agents->size(); i++) {
        Agent* agent = (*agents)[i];
        if (agent->x < m_minX)
            agent->x = m_minX;
        else if (agent->x > m_maxX)
            agent->x = m_maxX;
        if (agent->y < m_minY)
            agent->y = m_minY;
        else if (agent->y > m_maxY)
            agent->y = m_maxY;

        agent->x = (((maxWidth - minWidth) * (agent->x - m_minX)) / (rangeX)) + minWidth;
        agent->y = (((maxHeight - minHeight) * (agent->y - m_minY)) / (rangeY)) + minHeight;
    }

    QBrush brush(Qt::blue);
    QPen pen(Qt::black, 1);

    if (m_dataItems.size() == 0) {
        for (int i = 0; i < items->size(); i++) {
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
            ClusterItem* clusterItem = (*items)[i];

            item->setRect(QRectF(clusterItem->x, clusterItem->y, 2.5, 2.5));
            item->setBrush(brush);
            item->setPen(pen);
            m_scene->addItem(item);
            item->show();
            m_dataItems.push_back(item);
        }
    }


    brush.setColor(Qt::red);
    for (int i = 0; i < agents->size(); i++) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(0);
        Agent* agent = (*agents)[i];
        item->setRect(agent->x, agent->y, 5, 5);
        item->setBrush(brush);
        item->setPen(pen);
        m_scene->addItem(item);
        item->show();
        m_agentItems.push_back(item);
    }

}
