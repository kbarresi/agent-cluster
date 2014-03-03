#include <QApplication>

#include "agentcluster.h"
#include "clustercanvas.h"
#include "def.h"

void printUsage();
void showCluster(std::vector<ClusterItem*> items, std::vector<Agent*> agents);
ClusterCanvas* canvas = 0;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList args = a.arguments();
    if (args.size() < 2) {
        printUsage();
        return 0;
    }

    int iterations = 100;
    if (args.contains("-n")) {
        int iterationsIndex =args.indexOf("-n") + 1;
        if (iterationsIndex >= args.size()) {
            printf("Error: iterations not specified\n\n");
            return 1;
        }

        iterations = args.at(iterationsIndex).toInt();
        printf("User set iterations: %i\n", iterations);
    }
    std::string dataFile = args.last().toStdString();

    canvas = new ClusterCanvas(0);

    AgentCluster* agentCluster = new AgentCluster(&showCluster);
    if (!agentCluster->loadData(dataFile)) {
        printf("Error: unable to open data file: %s\n\n", dataFile);
        return 1;
    } else
        printf("...loaded data: %i points\n", agentCluster->dataCount());



    agentCluster->start(iterations);
    return a.exec();
}

void showCluster(std::vector<ClusterItem*> items, std::vector<Agent*> agents) {
    if (!canvas)
        return;
    canvas->cluster(items, agents);
}

void printUsage() {
    printf("Usage: AgentCluster [iterations] <data.csv>\n");
    printf("\nWhere <data.csv> is a comma-separated list of input data, with two ");
    printf("columns, each representing an x/y position. The values will be graphically ");
    printf("clustered using the AgentCluster algorithm.\n\n");
}
