#include <QApplication>

#include "agentcluster.h"
#include "clustercanvas.h"
#include "def.h"

#include <time.h>

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

    ClusterCanvas* canvas = new ClusterCanvas(dataFile, iterations);
    canvas->run();

    return a.exec();
}


/**
 * @brief printUsage Prints program usage to stdout
 */
void printUsage() {
    printf("Usage: AgentCluster [iterations] <data.csv>\n");
    printf("\nWhere <data.csv> is a comma-separated list of input data, with two ");
    printf("columns, each representing an x/y position. The values will be graphically ");
    printf("clustered using the AgentCluster algorithm.\n\n");
}
