#include "faso.h"
#include "def.h"

#include <QMutex>
#include <cmath>
#include <stdio.h>

/**
 * @brief FASO::FASO Implements a generic Foraging Agent Swarm Optimization algorithm on a sample,
 * multimodal optimization profile.
 * @param parent QObject parent.
 */
FASO::FASO(int iterations, int swarmSize, QObject *parent) :
    QObject(parent)
{
    m_iterations = iterations;
    m_swarmSize = swarmSize;
    m_lowestValue = landscape(0, 0);
}
FASO::~FASO() {
}

void FASO::start() {
    m_dataMinX = -5;
    m_dataMinY = -5;
    m_dataMaxX = 5;
    m_dataMaxY = 5;

    m_agentSensorRange = 0.25;       //CHECK THIS OUT YO
    m_minRange = m_agentSensorRange * 0.2;
    m_agentStepSize = m_agentSensorRange * STEP_SIZE_TO_SENSOR_RATIO;


    if (m_swarmSize == -1) {
        m_swarmSize = DEFAULT_SWARM_SIZE;
    }
    for (int i = 0; i < m_swarmSize; i++) {
        Agent* agent = new Agent();
        agent->x = randomDouble(m_dataMinX, m_dataMaxX);
        agent->y = randomDouble(m_dataMinY, m_dataMaxY);
        m_agents.push_back(agent);
    }
    printf("Created a swarm containing %i agents...\n", m_swarmSize);


    for (int i = 0; i < m_iterations; i++) {
        updateHappiness();
        updateRanges();
        for (unsigned int j = 0; j < m_agents.size(); j++) {
            Agent* agent= m_agents[j];
            move(agent);
        }

        printf("Finished iteration %i...\n", i);
        emit update(&m_agents);
        printf("\tupdating...");
        sleep(MOVEMENT_DELAY);
    }

    printf("POSITIONS\n\n");
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent* a = m_agents[i];
        printf("%4.2f,%4.2f\n", a->x, a->y);
    }
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

    double objectiveFunctionValue = 1.0 / (landscape(agent->x, agent->y) - m_lowestValue + 1.0);  //we want to minimize, so do 1/function value
    double neighborScore = CROWDING_ADVERSION_FACTOR * (double)agentsWithinCrowdingRange(agent).size();

    double totalScore = objectiveFunctionValue / (neighborScore + 1.0);
    return totalScore;
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
        double norm = sqrt(pow(unitX, 2) + pow(unitY, 2));
        unitX /= norm;
        unitY /= norm;

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

double FASO::landscape(double x, double y) {
    //(((x^4 - 16*x^2 + 5*x) + (y^4 - 16*y^2 + 5*y)) / 2)
    double result = (((pow(x, 4) - 16.0 * pow(x, 2) + 5.0 * x) + (pow(y, 4) - 16.0 * pow(y, 2) + 5.0 * y)) * 0.5);
    if (result < m_lowestValue) {
        m_lowestValue = result;
        updateHappiness();
    }

    return result;
}
double FASO::gradientX(double x, double y) {
    //(x^5/5 - 16x^3/3 + 5x^2/2 + xy(y^3 - 16y + 5)) / 2;
    return ((pow(x, 5)/5.0) - ((16 * pow(x, 3)) / 3.0) + ((5 * pow(x, 2)) / 2.0) + x*y*(pow(y, 3) - (16.0 * y) + 5.0)) * 0.5;
}
double FASO::gradientY(double x, double y) {
    //(x^4y - 16x^2y + 5xy + y^5/5 - 16y^3/3 + 5y^2/2) / 2
    return ((y * pow(x, 4)) - (16.0 * y * pow(x, 2)) + (5 * x * y) + (pow(y, 5)/5.0) - ((16 * pow(y, 3))/3.0) + ((5 * pow(y, 2))/2.0)) * 0.5;
}
