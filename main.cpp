#include <QApplication>
#include <QThread>

#include "agentcluster.h"
#include "clustercanvas.h"
#include "faso.h"
#include "def.h"

#include <time.h>
#include <stdio.h>

void printUsage();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::srand(time(NULL));

    QStringList args = a.arguments();
    if (args.size() < 2) {
        printUsage();
        return 0;
    }

    int iterations = 100;
    int instances = 1;
    int swarmSize = -1;

    if (args.contains("-n")) {
        int iterationsIndex =args.indexOf("-n") + 1;
        if (iterationsIndex >= args.size()) {
            printf("Error: iterations not specified\n\n");
            return 1;
        }

        iterations = args.at(iterationsIndex).toInt();
        printf("User set iterations: %i\n", iterations);
    }
    if (args.contains("-s")) {
        int swarmSizeIndex = args.indexOf("-s") + 1;
        if (swarmSizeIndex >= args.size()) {
            printf("Error: swarm size not specified\n\n");
            return 1;
        }
        swarmSize = args.at(swarmSizeIndex).toInt();
        printf("User set swarm size: %i\n", swarmSize);
    }
    if (args.contains("-i")) {
        int instanceIndex = args.indexOf("-i") + 1;
        if (instanceIndex >= args.size()) {
            printf("Error: instance count not specified\n\n");
            return 1;
        }
        instances = args.at(instanceIndex).toInt();
        printf("User set instance count: %i\n", instances);
    }

    ClusterCanvas* canvas = new ClusterCanvas();
    QThread *workThread = new QThread();

    if (args.contains("-c")) {  //we're using it to cluster...
        std::string dataFile = args.last().toStdString();
        AgentCluster *cluster = new AgentCluster(iterations, swarmSize, 0);
        cluster->moveToThread(workThread);
        QObject::connect(workThread, SIGNAL(started()), cluster, SLOT(start()));
        //QObject::connect(cluster, SIGNAL(update(std::vector<ClusterItem*>*,std::vector<Agent*>*)), canvas, SLOT(updateDisplay(std::vector<ClusterItem*>*,std::vector<Agent*>*)));
        QObject::connect(cluster, SIGNAL(setClusters(std::vector<Cluster*>*)), canvas, SLOT(setClusters(std::vector<Cluster*>*)));
        if (!cluster->loadData(dataFile)) {
            printf("Error: unable to open data file: %s\n\n", dataFile.c_str());
            return -1;
        } else
            printf("...loaded data: %i points\n", (int)cluster->dataCount());

        workThread->start();
    } else {    //otherwise, use a generic optimization function.
        TestFunction type = Ackley;
        FASO* faso = new FASO(iterations, instances, swarmSize, type);
        canvas->setFunction(type);
        faso->moveToThread(workThread);
        QObject::connect(workThread, SIGNAL(started()), faso, SLOT(start()));
        QObject::connect(faso, SIGNAL(update(std::vector<Agent*>*)), canvas, SLOT(updateDisplay(std::vector<Agent*>*)));
        workThread->start();
    }

    return a.exec();
}


/**
 * @brief printUsage Prints program usage to stdout
 */
void printUsage() {
    printf("Usage: AgentCluster [options] <data.csv>\n");
    printf("\nWhere <data.csv> is a comma-separated list of input data, with two ");
    printf("columns, each representing an x/y position. The values will be graphically ");
    printf("clustered using the AgentCluster algorithm.\n\n");
    printf("Options:\n");
    printf("\t-c\tUse clustering (FASC) mode. By default, uses generic FASO mode\n");
    printf("\t-n\tNumber of iterations to run\n");
    printf("\t-s\tNumber of agents in swarm\n");
    printf("\t-i\tNumber of swarm instances whose results should be averaged together");
    printf("\n\n\n");
}
