#ifndef CLUSTERCANVAS_H
#define CLUSTERCANVAS_H

#include "def.h"
#include "faso.h"
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
    explicit ClusterCanvas(QWidget *parent = 0);
    ~ClusterCanvas();
    void setFunction(TestFunction function) { m_function = function; }

public slots:
    void updateDisplay(std::vector<ClusterItem*>* items, std::vector<Agent*>* agents);
    void updateDisplay(std::vector<Agent*>* agents);
    void setClusters(std::vector<Cluster*>* clusters);

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
        ~AgentVisualizer() {
            if (agent) {
                agent->hide();
                delete agent;
            }
            if (forageRange) {
                forageRange->hide();
                delete forageRange;
            }
            if (crowdingRange) {
                crowdingRange->hide();
                delete crowdingRange;
            }
            if (trail) {
                trail->hide();
                delete trail;
            }
        }
    };

    TestFunction m_function;

    Ui::ClusterCanvas *ui;

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
