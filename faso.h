#ifndef FASO_H
#define FASO_H


#include <QObject>
#include <vector>

enum TestFunction { Styblinski, Ackley };

class Agent;
class FASO : public QObject
{
    Q_OBJECT
public:

    FASO(int iterations,
         int instances = 1,
         int swarmSize = -1,
         TestFunction selectedFunction = Styblinski,
         QObject *parent = 0);
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
    int m_instances;

    TestFunction m_testFunction;
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
    double objectiveFunction(double x, double y);

    void updateRanges();
    void move(Agent* agent);
    void moveTowards(Agent *agentOne, Agent *agentTwo);
    void moveRandomly(Agent *agent);

    std::vector<Agent*> agentsWithinCrowdingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinForagingRange(Agent* agent) const;
    std::vector<Agent*> agentsWithinRange(Agent* agent, double range) const;

    void sleep(int millis);
    double landscape(double x, double y) const;
    double gradientX(double x, double y) const ;
    double gradientY(double x, double y) const;

    static double styblinksi(double x, double y);
    static double styblinksiGradientX(double x);
    static double styblinksiGradientY(double y);

    static double ackley(double x, double y);
    static double ackleyGradientX(double x, double y);
    static double ackleyGradientY(double x, double y);
};

#endif // FASO_H
