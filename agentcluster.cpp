#include "agentcluster.h"

#include <iostream>
#include <fstream>

AgentCluster::AgentCluster(AgentClusterVisualizer visualizeFunction)
{
    m_visualizer = visualizeFunction;
}

AgentCluster::~AgentCluster() {

}

bool AgentCluster::loadData(std::string dataSource) {
    std::ifstream file(dataSource);
    if (!file.is_open()) {
        printf("Error: unable to open file: %s\n\n", dataSource);
        return false;
    }

    std::vector<std::string> lineString;

    while (file.good()) {
        std::string line;
        std::getline(file, line, '\n');

        lineString = split(line, ',');
        if (lineString.size() != 2)
            continue;

        std::string xStr = lineString[0];
        std::string yStr = lineString[1];


        ClusterItem* item = new ClusterItem();
        item->x = atof(xStr.c_str());
        item->y = atof(yStr.c_str());
        m_data.push_back(item);

        lineString.clear();
    }

    return true;
}


void AgentCluster::start(int iterations) {
    convergencePhase(iterations);
    consolidationPhase();
    assignmentPhase();

    (m_visualizer)(m_data, m_agents);
}

void AgentCluster::convergencePhase(int iterations) {
    (void)(iterations);
}

void AgentCluster::consolidationPhase() {

}

void AgentCluster::assignmentPhase() {

}
