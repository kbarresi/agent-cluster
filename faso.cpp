#include "faso.h"
#include "def.h"

#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <cmath>
#include <stdio.h>

/**
 * @brief FASO::FASO Implements a generic Foraging Agent Swarm Optimization algorithm on a sample,
 * multimodal optimization profile.
 * @param parent QObject parent.
 */
FASO::FASO(int iterations,
           int instances,
           int swarmSize,
           TestFunction selectedFunction,
           QObject *parent) :
    QObject(parent)
{
    m_testFunction = selectedFunction;
    m_iterations = iterations;
    m_swarmSize = swarmSize;
    m_lowestValue = landscape(0, 0);
    m_instances = instances;
}
FASO::~FASO() {
}

void FASO::start() {
    m_dataMinX = -5;
    m_dataMinY = -5;
    m_dataMaxX = 5;
    m_dataMaxY = 5;

    m_agentSensorRange = 0.5;       //CHECK THIS OUT YO
    m_minRange = m_agentSensorRange * 0.2;
    m_agentStepSize = m_agentSensorRange * STEP_SIZE_TO_SENSOR_RATIO;

    if (m_swarmSize == -1) {
        m_swarmSize = DEFAULT_SWARM_SIZE;
    }

    double* xPositions = (double*) calloc(m_swarmSize * m_instances, sizeof(double));
    double* yPositions = (double*) calloc(m_swarmSize * m_instances, sizeof(double));


    for (int i = 0; i < m_swarmSize; i++) { //create the swarm...
        Agent* agent = new Agent();
        agent->x = randomDouble(m_dataMinX, m_dataMaxX);
        agent->y = randomDouble(m_dataMinY, m_dataMaxY);
        m_agents.push_back(agent);
    }
    printf("Created a swarm containing %i agents...\n", m_swarmSize);

    for (int n = 0; n < m_instances; n++) {


        for (int i = 0; i < m_iterations; i++) {
            updateHappiness();
            updateRanges();
            for (unsigned int j = 0; j < m_agents.size(); j++) {
                Agent* agent= m_agents[j];
                move(agent);
            }

            printf("Finished iteration %i...\n", i);
            if (i % UPDATE_RATE == 0) {
                emit update(&m_agents);
                printf("\tupdating...");
            }
            sleep(MOVEMENT_DELAY);
        }

        for (unsigned int i = 0; i < m_agents.size(); i++) { //save positions...
            Agent *a = m_agents[i];
            int index = (n * m_agents.size()) + i;
            xPositions[index] = a->x;
            yPositions[index] = a->y;
            a->x = randomDouble(m_dataMinX, m_dataMaxX);
            a->y = randomDouble(m_dataMinY, m_dataMaxY);
        }

        printf("Finished instance %i\n", n);
    }

    QFile file("../AgentCluster/test_data/results.csv");
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream(&file);
        for (int i = 0; i < (m_swarmSize * m_instances); i++) {
            stream << xPositions[i] << "," << yPositions[i] << "\n";
        }
        file.close();
    } else {
        printf("POSITIONS\n\n");
        for (int i = 0; i < (m_swarmSize * m_instances); i++)
            printf("%4.2f,%4.2f\n", xPositions[i], yPositions[i]);
    }



    free(xPositions);
    free(yPositions);
    printf("\n\nFinished...\n");
    emit finished();
}





void FASO::updateHappiness() {
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent* agent = m_agents[i];
        agent->happiness = calculateHappiness(agent);
    }
}
double FASO::calculateHappiness(Agent *agent) {
    //h(i) = O(p_i) / (|A(p_i, r_c^i)| + 1)

    double objectiveFunctionValue = objectiveFunction(agent->x, agent->y);
    double neighborScore = CROWDING_ADVERSION_FACTOR * (double)agentsWithinCrowdingRange(agent).size();

    double totalScore = objectiveFunctionValue / (neighborScore + 1.0);
    return totalScore;
}
double FASO::objectiveFunction(double x, double y) {
    double result = landscape(x, y);
    if (result < m_lowestValue) {
        m_lowestValue = result;
        printf("New lowest: %4.2f\n", m_lowestValue);
        updateHappiness();
    }

    return 1.0 / (result - m_lowestValue);
}




void FASO::updateRanges() {
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent *agent = m_agents[i];
        double positionGoodness = 1.0 / landscape(agent->x, agent->y);

        double r_f = m_minRange + ((m_agentSensorRange - m_minRange) / (1.0 + AGENT_BETA * positionGoodness));

        agent->foragingRange = (r_f + agent->foragingRange) * 0.5;
        agent->crowdingRange = agent->foragingRange * CROWDING_TO_FORAGE_DIST_RATIO;
    }
}
void FASO::move(Agent *agent) {
    std::vector<Agent*> neighbors =  agentsWithinForagingRange(agent);    //bestAgentInRange(agent);   //best agent in range.
    if (neighbors.size() != 0) {    //has neighbors
        Agent* bestNeighbor = neighbors[0];
        for (unsigned int i = 1; i < neighbors.size(); i++) {
            Agent* candidate = neighbors[i];
            if (candidate->happiness > bestNeighbor->happiness)
                bestNeighbor = candidate;
        }
        if (bestNeighbor->happiness > agent->happiness) {   //found a better neighbor we should move towards
            moveTowards(agent, bestNeighbor);
        } else {    //otherwise, just move randomly :(
            moveRandomly(agent);
        }
    } else {    //all alone...move in direction of gradient

        double unitX = gradientX(agent->x, agent->y);
        double unitY = gradientY(agent->x, agent->y);
        if (unitX == 0.0 && unitY == 0.0) { //already at a max/min? move randomly
            moveRandomly(agent);
            return;
        }

        double norm = sqrt(pow(unitX, 2) + pow(unitY, 2));
        unitX /= -norm;     //we want to go down the slope...
        unitY /= -norm;

        double magnitude = randomDouble(0.1, 1);
        double newX = agent->x + (unitX * magnitude);
        double newY = agent->y + (unitY * magnitude);

        Q_ASSERT_X(nanTest(newX), "Failed NaN", __FUNCTION__);
        Q_ASSERT_X(nanTest(newY), "Failed NaN", __FUNCTION__);

        agent->x = newX;
        agent->y = newY;
    }
}
void FASO::moveTowards(Agent *agentOne, Agent *agentTwo) {
    double crowdingFactor = (agentOne->crowdingRange + agentTwo->crowdingRange) * 0.5;
    double agentDistance = pointDistance(agentOne->x, agentTwo->x, agentOne->y, agentTwo->y) - crowdingFactor;

    if (agentDistance == 0)
        return;

    double moveMagnitude = std::min(randomDouble(0, agentOne->foragingRange), agentDistance) * randomDouble(0, 0.9);
    double unitVectorX = (agentTwo->x - agentOne->x) / agentDistance;
    double unitVectorY = (agentTwo->y - agentOne->y) / agentDistance;

    double newX = agentOne->x + (moveMagnitude * unitVectorX);
    double newY = agentOne->y + (moveMagnitude * unitVectorY);

    Q_ASSERT_X(nanTest(newX), "Failed NaN", __FUNCTION__);
    Q_ASSERT_X(nanTest(newY), "Failed NaN", __FUNCTION__);

    agentOne->x = newX;
    agentOne->y = newY;
    agentOne->happiness = calculateHappiness(agentOne);
}
void FASO::moveRandomly(Agent *agent) {
    double initialX = agent->x;
    double initialY = agent->y;
    double initialHappiness = agent->happiness;

    double moveMagnitude =  randomDouble(0.0, agent->foragingRange * RANDOM_MOVE_FACTOR + 1);
    double moveDirection = randomDouble(0, 360) * (PI / 180.0);

    double posX = initialX + (cos(moveDirection) * moveMagnitude);
    double posY = initialY + (sin(moveDirection) * moveMagnitude);

    Q_ASSERT_X(nanTest(posX), "Failed NaN", __FUNCTION__);
    Q_ASSERT_X(nanTest(posY), "Failed NaN", __FUNCTION__);

    agent->x = posX;
    agent->y = posY;
    double newHappiness = calculateHappiness(agent);
    if (newHappiness >= initialHappiness) { //did we find a better position?
        agent->happiness = newHappiness;
        return;
    }

    //otherwise, move back...
    agent->x = initialX;
    agent->y = initialY;
}



//RANGE FUNCTIONS
std::vector<Agent*> FASO::agentsWithinCrowdingRange(Agent *agent) const {
    return agentsWithinRange(agent, agent->crowdingRange);
}
std::vector<Agent*> FASO::agentsWithinForagingRange(Agent *agent) const {
    return agentsWithinRange(agent, agent->foragingRange);
}
std::vector<Agent*> FASO::agentsWithinRange(Agent *agent, double range) const {
    std::vector<Agent*> closeAgents;
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent* testAgent = m_agents[i];
        if (testAgent == agent)
            continue;
        double distance = sqrt(pow(testAgent->x - agent->x, 2) + pow(testAgent->y - agent->y, 2));
        if (distance <= range)
            closeAgents.push_back(testAgent);
    }
    return closeAgents;
}



void FASO::sleep(int milliseconds) {
    QMutex mut;
    mut.lock();
    mut.tryLock(milliseconds);
    mut.unlock();
}

double FASO::landscape(double x, double y) const {
    if (m_testFunction == Styblinski)
        return styblinksi(x, y);
    else if (m_testFunction == Ackley)
        return ackley(x, y);
    else
        return styblinksi(x, y);
}
double FASO::gradientX(double x, double y) const {
    if (m_testFunction == Styblinski)
        return styblinksiGradientX(x);
    else if (m_testFunction == Ackley)
        return ackleyGradientX(x, y);
    else
        return styblinksiGradientX(x);
}
double FASO::gradientY(double x, double y) const {
    if (m_testFunction == Styblinski)
        return styblinksiGradientY(y);
    else if (m_testFunction == Ackley)
        return ackleyGradientY(x, y);
    else
        return styblinksiGradientY(y);
}


double FASO::styblinksi(double x, double y) {
    return (((pow(x, 4) - 16.0 * pow(x, 2) + 5.0 * x) + (pow(y, 4) - 16.0 * pow(y, 2) + 5.0 * y)) * 0.5);
}
double FASO::styblinksiGradientX(double x) {
    //dz/dx = 2x^3 -16x +5/2
    return (2.0 * pow(x, 3)) - (16.0 * x) + (2.5);
}
double FASO::styblinksiGradientY(double y) {
    //2y^3 - 16y +5/2
    return (2.0 * pow(y, 3)) - (16.0 * y) + (2.5);
}

double FASO::ackley(double x, double y) {
    // (-20 * exp(-0.2 * sqrt(0.5 * (x^2 + y^2))) - exp(0.5*(cos(2 * pi * x) + cos(2 * pi * y))) + 20 + exp(1))
    return (-20.0 * pow(E, (-0.2 * sqrt(0.5 * (pow(x, 2) + pow(y, 2))))) - pow(E, 0.5 * (cos(2.0 * PI * x) + cos(2.0 * PI * y))) + 20.0 + E);
}
double FASO::ackleyGradientX(double x, double y) {
    //dz/dx = ((2.82843 * x * exp(-0.141421 * sqrt(x^2 + y^2))/(sqrt(x^2 + y^2))) + pi*sin(2*pi*x)*exp(0.5(cos(2*pi*x) + cos(2*pi*y)))
    double x2 = pow(x, 2);
    double y2 = pow(y, 2);
    return ((2.82843 * x * exp(-0.141421 * sqrt(x2 + y2)))/(sqrt(x2 + y2))) + PI*sin(2*PI*x)*exp(0.5 * (cos(2*PI*x) + cos(2*PI*y)));
}
double FASO::ackleyGradientY(double x, double y) {
    //dz/dy = ((2.82843 * y * exp(-0.141421 * sqrt(x^2 + y^2))/(sqrt(x^2 + y^2))) + pi*sin(2*pi*y)*exp(0.5(cos(2*pi*x) + cos(2*pi*y)))
    double x2 = pow(x, 2);
    double y2 = pow(y, 2);
    return ((2.82843 * y * exp(-0.141421 * sqrt(x2 + y2)))/(sqrt(x2 + y2))) + PI*sin(2*PI*y)*exp(0.5 * (cos(2*PI*x) + cos(2*PI*y)));
}
