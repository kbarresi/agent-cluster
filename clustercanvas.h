#ifndef CLUSTERCANVAS_H
#define CLUSTERCANVAS_H

#include "def.h"
#include "gui/qgraphicsellipseitemobject.h"
#include "gui/qgraphicslineitemobject.h"

#include <QMainWindow>
#include <QHash>
#include <QGraphicsView>
#include <QGraphicsScene>

class AgentCluster;
namespace Ui {
class ClusterCanvas;
}

class ClusterCanvas : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClusterCanvas(std::string dataSource, int iterations, int swarmSize = -1, QWidget *parent = 0);
    ~ClusterCanvas();

public slots:
    void updateDisplay(std::vector<ClusterItem*>* items, std::vector<Agent*>* agents);
    void run();

private:
    struct AgentVisualizer {
        QGraphicsEllipseItemObject* agent;
        QGraphicsEllipseItemObject* forageRange;
        QGraphicsEllipseItemObject* crowdingRange;
        QGraphicsLineItemObject* trail;

        AgentVisualizer() {
            agent = forageRange = crowdingRange = 0;
            trail = 0;
        }
    };


    Ui::ClusterCanvas *ui;

    AgentCluster* m_cluster;
    QThread* m_clusterThread;

    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

    std::vector<QGraphicsEllipseItemObject*> m_dataItems;
    QHash<Agent*, AgentVisualizer*> m_agentVisualizers;

    double m_minX;
    double m_maxX;
    double m_minY;
    double m_maxY;
    bool m_calculatedDataRange;

};

#endif // CLUSTERCANVAS_H
