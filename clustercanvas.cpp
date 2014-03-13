#include "clustercanvas.h"
#include "agentcluster.h"
#include "ui_clustercanvas.h"

#include <QGraphicsEllipseItem>
#include <QPropertyAnimation>
#include <QThread>


#include <stdio.h>

/**
 * @brief ClusterCanvas::ClusterCanvas Visualization class that takes care of showing ClusterItem
 * and Agent objects on a scaled 2d canvas.
 * @param dataSource Filename of csv file containing cluster data
 * @param iterations Number of iterations to run for.
 * @param parent Parent QObject
 */
ClusterCanvas::ClusterCanvas(std::string dataSource, int iterations, int swarmSize, QWidget *parent) :
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
    m_cluster = new AgentCluster(iterations, swarmSize, 0);
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

    double yMidline = m_view->sceneRect().height() / 2;

    if (m_dataItems.size() == 0 && SHOW_DATA) {
        for (unsigned int i = 0; i < items->size(); i++) {
            QGraphicsEllipseItemObject* item = new QGraphicsEllipseItemObject(0);
            ClusterItem* clusterItem = (*items)[i];
            double pX = ((maxWidth - minWidth) * (clusterItem->x - m_minX)) / (rangeX) + minWidth;
            double pY = ((maxHeight - minHeight) * (clusterItem->y - m_minY)) / (rangeY) + minHeight;
            pY -= 2 * (pY - yMidline);

            item->setRect(QRectF(pX, pY, 2.5, 2.5));
            item->setBrush(brush);
            item->setPen(pen);
            m_scene->addItem(item);
            item->show();
            m_dataItems.push_back(item);
        }
    }

    //Go through and draw each agent, and related structures
    for (unsigned int i = 0; i < agents->size(); i++) {
        Agent* agent = (*agents)[i];
        AgentVisualizer* visualizer = 0;
        if (m_agentVisualizers.contains(agent))
            visualizer = m_agentVisualizers.value(agent);
        else {
            visualizer = new AgentVisualizer();
            m_agentVisualizers.insert(agent, visualizer);
        }

        //Draw the agent itself...
        QGraphicsEllipseItemObject* radiusItem = 0;
        brush.setColor(Qt::red);
        if (visualizer->agent)
            radiusItem = visualizer->agent;
        else {
            radiusItem = new QGraphicsEllipseItemObject(0);
            radiusItem->setBrush(brush);
            radiusItem->setPen(pen);
            radiusItem->setRect(0, 0, AGENT_SIZE, AGENT_SIZE);
            m_scene->addItem(radiusItem);
            radiusItem->show();
            visualizer->agent = radiusItem;
        }

        double pX = (((maxWidth - minWidth) * (agent->x - m_minX)) / (rangeX) + minWidth) - (radiusItem->boundingRect().width() / 2.0);
        double pY = (((maxHeight - minHeight) * (agent->y - m_minY)) / (rangeY) + minHeight) - (radiusItem->boundingRect().height() / 2.0);
        pY -= 2 * (pY - yMidline);

        if (SHOW_PATH) {    //And the path, if selected
            QGraphicsLineItemObject* trail = 0;
            if (visualizer->trail)
                trail = visualizer->trail;
            else {
                trail = new QGraphicsLineItemObject(0);
                trail->setPen(pen);
                m_scene->addItem(trail);
                trail->show();
                visualizer->trail = trail;
            }
            trail->setLine(radiusItem->x(), radiusItem->y(), pX, pY);
        }

        if (ANIMATED) {
            QPropertyAnimation* mover = new QPropertyAnimation(radiusItem, "pos");
            mover->setDuration(MOVEMENT_DELAY - 1);
            mover->setStartValue(radiusItem->pos());
            mover->setEndValue(QPointF(pX, pY));
            mover->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            radiusItem->setPos(pX, pY);
        }


        ////////Draw the forage range
        brush.setColor(QColor(0, 0, 0, 50));
        pen.setColor(Qt::transparent);
        pen.setStyle(Qt::DashLine);
        if (SHOW_FORAGE_RANGE) {
            radiusItem = 0;
            if (visualizer->forageRange)
                radiusItem = visualizer->forageRange;
            else {
                radiusItem = new QGraphicsEllipseItemObject(0);
                radiusItem->setBrush(brush);
                radiusItem->setPen(pen);
                m_scene->addItem(radiusItem);
                radiusItem->show();
                visualizer->forageRange = radiusItem;
            }

            double radius = agent->foragingRange;
            double topLeftX = ((maxWidth - minWidth) * ((agent->x - radius) - m_minX)) / (rangeX) + minWidth;
            double topLeftY = ((maxHeight - minHeight) * ((agent->y - radius) - m_minY)) / (rangeY) + minHeight;
            topLeftY -= 2 * (topLeftY - yMidline);

            double bottomRightX = ((maxWidth - minWidth) * ((agent->x + radius) - m_minX)) / (rangeX) + minWidth;
            double bottomRightY = ((maxHeight - minHeight) * ((agent->y + radius) - m_minY)) / (rangeY) + minHeight;
            bottomRightY -= 2 * (bottomRightY - yMidline);

            double ellipseWidth = bottomRightX - topLeftX;
            double ellipseHeight = bottomRightY - topLeftY;

            double startX = visualizer->agent->x() - (ellipseWidth / 2);
            double startY = visualizer->agent->y() - (ellipseHeight / 2);

            radiusItem->setRect(0, 0, ellipseWidth, ellipseHeight);
            if (ANIMATED) {
                QPropertyAnimation* mover = new QPropertyAnimation(radiusItem, "pos");
                mover->setDuration(MOVEMENT_DELAY - 1);

                QPointF startingPoint = QPointF(startX, startY) + QPointF(visualizer->agent->boundingRect().width() / 2, visualizer->agent->boundingRect().height() / 2);
                mover->setStartValue(startingPoint);
                mover->setEndValue(QPointF(topLeftX, topLeftY) + QPointF(0, visualizer->agent->boundingRect().height()));
                mover->start(QAbstractAnimation::DeleteWhenStopped);
            } else {
                radiusItem->setPos(topLeftX, topLeftY);
            }
        }


        pen.setColor(QColor(255, 0, 0, 128));
        brush.setColor(Qt::transparent);
        if (SHOW_CROWDING_RANGE) {
            radiusItem = 0;
            if (visualizer->crowdingRange)
                radiusItem = visualizer->crowdingRange;
            else {
                radiusItem = new QGraphicsEllipseItemObject(0);
                radiusItem->setBrush(brush);
                radiusItem->setPen(pen);
                m_scene->addItem(radiusItem);
                radiusItem->show();
                visualizer->crowdingRange = radiusItem;
            }
            double radius = agent->crowdingRange;
            double topLeftX = ((maxWidth - minWidth) * ((agent->x - radius) - m_minX)) / (rangeX) + minWidth;
            double topLeftY = ((maxHeight - minHeight) * ((agent->y - radius) - m_minY)) / (rangeY) + minHeight;
            topLeftY -= 2 * (topLeftY - yMidline);


            double bottomRightX = ((maxWidth - minWidth) * ((agent->x + radius) - m_minX)) / (rangeX) + minWidth;
            double bottomRightY = ((maxHeight - minHeight) * ((agent->y + radius) - m_minY)) / (rangeY) + minHeight;
            bottomRightY -= 2 * (bottomRightY - yMidline);

            double ellipseWidth = bottomRightX - topLeftX;
            double ellipseHeight = bottomRightY - topLeftY;

            double startX = visualizer->agent->x() - (ellipseWidth / 2);
            double startY = visualizer->agent->y() - (ellipseHeight / 2);

            radiusItem->setRect(0, 0, ellipseWidth, ellipseHeight);

            if (ANIMATED) {
                QPropertyAnimation* mover = new QPropertyAnimation(radiusItem, "pos");
                mover->setDuration(MOVEMENT_DELAY - 1);

                QPointF startingPoint = QPointF(startX, startY) + QPointF(visualizer->agent->boundingRect().width() / 2, visualizer->agent->boundingRect().height() / 2);
                mover->setStartValue(startingPoint);
                mover->setEndValue(QPointF(topLeftX, topLeftY) + QPointF(0, visualizer->agent->boundingRect().height()));
                mover->start(QAbstractAnimation::DeleteWhenStopped);
            } else {
                radiusItem->setPos(topLeftX, topLeftY);
            }

        }
    }
}
