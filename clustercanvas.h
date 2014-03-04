#ifndef CLUSTERCANVAS_H
#define CLUSTERCANVAS_H

#include "def.h"

#include <QMainWindow>
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
    explicit ClusterCanvas(std::string dataSource, int iterations, QWidget *parent = 0);
    ~ClusterCanvas();

public slots:
    void updateDisplay(std::vector<ClusterItem*>* items, std::vector<Agent*>* agents);
    void run();

private:
    Ui::ClusterCanvas *ui;

    AgentCluster* m_cluster;
    QThread* m_clusterThread;

    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

    std::vector<QGraphicsEllipseItem*> m_dataItems;
    std::vector<QGraphicsEllipseItem*> m_agentItems;

    double m_minX;
    double m_maxX;
    double m_minY;
    double m_maxY;
    bool m_calculatedDataRange;
};

#endif // CLUSTERCANVAS_H
