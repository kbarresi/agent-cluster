#ifndef AGENTCLUSTER_H
#define AGENTCLUSTER_H

#include "def.h"

#include <string>

class AgentCluster
{
public:
    AgentCluster(AgentClusterVisualizer visualizeFunction);
    ~AgentCluster();

    bool loadData(std::string dataSource);
    void start(int iterations);

    size_t dataCount() const { return m_data.size(); }
    size_t agentCount() const { return m_agents.size(); }
private:

    AgentClusterVisualizer m_visualizer;


    std::vector<Agent*> m_agents;
    std::vector<ClusterItem*> m_data;



    void convergencePhase(int iterations);
    void consolidationPhase();
    void assignmentPhase();

    void updateRanges();
    void updateHappiness();
    void moveTowards(Agent* agentOne, Agent* agentTwo);


    Agent* closestAgent(Agent* agent) const;
    std::vector<Agent*> nearbyAgents(Agent* agent) const;
    double calculateHappiness(Agent* agent) const;


};

#endif // AGENTCLUSTER_H
