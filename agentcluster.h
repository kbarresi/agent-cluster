#ifndef AGENTCLUSTER_H
#define AGENTCLUSTER_H

#include "def.h"

#include <string>
#include <QObject>

class AgentCluster : public QObject
{
    Q_OBJECT

public:
    AgentCluster(int iterations, int swarmSize = -1, QObject *parent = 0);
    ~AgentCluster();

    bool loadData(std::string dataSource);

    size_t dataCount() const { return m_data.size(); }
    size_t agentCount() const { return m_agents.size(); }

public slots:
    void start();

signals:
    void update(std::vector<ClusterItem*>* items, std::vector<Agent*>* agents);

private:
    int m_iterations;
    int m_swarmSize;

    std::vector<Agent*> m_agents;
    std::vector<ClusterItem*> m_data;


    double m_dataMinX;
    double m_dataMaxX;
    double m_dataMinY;
    double m_dataMaxY;

    double m_agentSensorRange;
    double m_minRange;
    double m_agentStepSize;

    double m_dataConcentrationSlope;
    double m_crowdingConcetrationSlope;

    void convergencePhase();
    void consolidationPhase();
    void assignmentPhase();

    void updateRanges();
    void updateHappiness();
    void moveTowards(Agent* agentOne, Agent* agentTwo);
    void moveRandomly(Agent* agent);

    Agent* bestAgentInRange(Agent* agent) const;

    std::vector<Agent*> agentsWithinCrowdingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinForagingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinRange(Agent* agent, double range) const;

    double calculateHappiness(Agent* agent) const;

    double averageClusterDistance() const;
    inline double pointDistance(double x1, double x2, double y1, double y2) const;

    void sleep(int milliseconds);
    inline bool nanTest(double x) { if (x != x) return false; else return true; }
};

#endif // AGENTCLUSTER_H
