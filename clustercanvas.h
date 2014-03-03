#ifndef CLUSTERCANVAS_H
#define CLUSTERCANVAS_H

#include "def.h"

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>

namespace Ui {
class ClusterCanvas;
}

class ClusterCanvas : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClusterCanvas(QWidget *parent = 0);
    ~ClusterCanvas();

    void cluster(std::vector<ClusterItem*> items, std::vector<Agent*> agents);

private:
    Ui::ClusterCanvas *ui;

    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

    std::vector<ClusterItem*> m_items;
    std::vector<Agent*> m_agents;
};

#endif // CLUSTERCANVAS_H
