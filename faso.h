#ifndef FASO_H
#define FASO_H


#include <QObject>
#include <vector>

class Agent;
class FASO : public QObject
{
    Q_OBJECT
public:
    FASO(int iterations, int swarmSize = -1, QObject *parent = 0);
    ~FASO();

public slots:
    void start();

signals:
    void update(std::vector<Agent*>* agents);
    void finished();

private:
    std::vector<Agent*> m_agents;
    int m_swarmSize;
    int m_iterations;

    double m_dataMinX;
    double m_dataMinY;
    double m_dataMaxX;
    double m_dataMaxY;
    double m_agentSensorRange;
    double m_minRange;
    double m_agentStepSize;

    double m_lowestValue;

    void updateHappiness();
    double calculateHappiness(Agent *agent);

    void updateRanges();
    void move(Agent* agent);
    void moveTowards(Agent *agentOne, Agent *agentTwo);
    void moveRandomly(Agent *agent);

    std::vector<Agent*> agentsWithinCrowdingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinForagingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinRange(Agent* agent, double range) const;

    void sleep(int millis);
    double landscape(double x, double y);
    static double gradientX(double x, double y);
    static double gradientY(double x, double y);
};

#endif // FASO_H
