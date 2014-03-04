#include "agentcluster.h"

#include <iostream>
#include <fstream>
#include <QMutex>

#include <math.h>

AgentCluster::AgentCluster(int iterations, QObject *parent)
    : QObject(parent)
{
    m_iterations = iterations;
    m_agentSensorRange = 0;
    m_agentBeta = 0;
    m_agentStepSize = 0;
    m_dataMinX = 0;
    m_dataMaxX = 0;
    m_dataMinY = 0;
    m_dataMaxY = 0;
}

AgentCluster::~AgentCluster() {

}

/**
 * @brief AgentCluster::loadData Attempts to load the 2-column data from the filename given.
 * @param dataSource The filename of the file to load data from.
 * @return True for successful loading, false if there was an error.
 */
bool AgentCluster::loadData(std::string dataSource) {
    std::ifstream file(dataSource.c_str());
    if (!file.is_open()) {
        printf("Error: unable to open file: %s\n\n", dataSource.c_str());
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

/**
 * @brief AgentCluster::start Runs the AgentCluster algorithm.
 * @details It initializes the agent population, and then simply calls the functions built for each
 * of the three main clustering phases.
 */
void AgentCluster::start() {

    //Init the population to random positions;
    m_dataMinX = m_data[0]->x;
    m_dataMinY = m_data[0]->y;
    m_dataMaxX = m_dataMinX;
    m_dataMaxY = m_dataMinY;
    for (unsigned int i = 1; i < m_data.size(); i++) {
        ClusterItem* item = m_data[i];
        if (item->x > m_dataMaxX)
            m_dataMaxX = item->x;
        else if (item->x < m_dataMinX)
            m_dataMinX = item->x;

        if (item->y > m_dataMaxY)
            m_dataMaxY = item->y;
        else if (item->y < m_dataMinY)
            m_dataMinY = item->y;
    }

    int swarmSize = (int)((double)m_data.size() * SWARM_SIZE_FACTOR);
    for (int i = 0; i < swarmSize; i++) {
        Agent* agent = new Agent();
        agent->x = randomDouble(m_dataMinX, m_dataMaxX);
        agent->y = randomDouble(m_dataMinY, m_dataMaxY);
        m_agents.push_back(agent);
    }
    printf("Created a swarm containing %i agents...\n", swarmSize);

    m_agentSensorRange = averageClusterDistance() / 5.0;
    m_agentBeta = 10;
    m_agentStepSize = m_agentSensorRange / 5.0;
    m_dataConcentrationSlope = (double)1 / m_data.size();
    m_crowdingConcetrationSlope = (double)(-1) / m_agents.size();

    for (unsigned int i = 0; i < m_agents.size(); i++)
        m_agents[i]->effectiveRange = m_agentSensorRange / 2.0;



    //Start each of the three clustering phases, in order
    convergencePhase();
    consolidationPhase();
    assignmentPhase();


    //Now we want to see the results;
    emit update(&m_data, &m_agents);
}

/**
 * @brief AgentCluster::convergencePhase Runs the convergence phase of the AgentSwarm algorithm
 * for a number of times determined by the iteration paramter.
 */
void AgentCluster::convergencePhase() {
    for (int i = 0; i < m_iterations; i++) {
        updateHappiness();
        updateRanges();
        for (unsigned int j = 0; j < m_agents.size(); j++) {
            Agent* agentOne = m_agents[j];
            Agent* agentTwo = bestAgentInRange(agentOne);   //best agent in range.
            if (agentTwo) //we have an agent to move towards
                moveTowards(agentOne, agentTwo);
            else    //otherwise, move randomly.
                moveRandomly(agentOne);
        }

        printf("Finished iteration %i...\n", i);
        if (i % 1 == 0) {
            emit update(&m_data, &m_agents);
            printf("\t...updated display\n");
            sleep(1000);
        }
    }
}

/**
 * @brief AgentCluster::consolidationPhase Runs the consolidation phase of the AgentSwarm algorithm.
 */
void AgentCluster::consolidationPhase() {

}

/**
 * @brief AgentCluster::assignmentPhase Runs the assignment phase of the AgentSwarm algorithm.
 */
void AgentCluster::assignmentPhase() {

}


/**
 * @brief AgentCluster::updateRanges Update the effective and selection ranges of all agents.
 * @details We follow a basic formula for the effective and selection ranges of agents as described
 * in the original AgentCluster paper. That is, the effective range Re(i) is:
 *      Re(i) = (sensorRange *  Pi * sensorRange^2) / (1 + beta * neighbors)
 */
void AgentCluster::updateRanges() {
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent *agent = m_agents[i];
        int neighborCount = (int) agentsWithinPersonalSpace(agent).size();

        double dT = (double)neighborCount / ((double)PI * (double)pow(m_agentSensorRange, 2));
        double rD = (double)m_agentSensorRange / ((double)1.0 + ((double)m_agentBeta * dT));
        double personalSpace = rD / (double)5.0;

        agent->effectiveRange = rD;
        agent->personalSpace = personalSpace;
    }
}

/**
 * @brief AgentCluster::updateHappiness Update the hapiness of all agents, as based on their current
 * position.
 */
void AgentCluster::updateHappiness() {
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent* agent = m_agents[i];
        agent->happiness = calculateHappiness(agent);
    }
}

/**
 * @brief AgentCluster::moveTowards Moves the first agent towards the second one.
 * @param agentOne The agent being moved.
 * @param agentTwo The agent who is being moved towards.
 */
void AgentCluster::moveTowards(Agent *agentOne, Agent *agentTwo) {
    double deltaX = agentTwo->x - agentOne->x;
    double deltaY = agentTwo->y - agentOne->y;
    double agentDistance = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    if (agentDistance == 0)
        return;

    double movementDistance = std::min(randomDouble(0.0, m_agentStepSize), agentDistance);

    double newX = ((deltaX / agentDistance) * movementDistance) + agentOne->x;
    double newY = ((deltaY / agentDistance) * movementDistance) + agentOne->y;
    Q_ASSERT_X(nanTest(newX), "Failed NaN", __FUNCTION__);
    Q_ASSERT_X(nanTest(newY), "Failed NaN", __FUNCTION__);

    agentOne->x = newX;
    agentOne->y = newY;
    agentOne->happiness = calculateHappiness(agentOne);
}

/**
 * @brief AgentCluster::moveRandomly Moves the agent randomly across the search space, a distance
 * related to the Agent's effective range. If the newly selected position has a lower hapiness
 * level than the original, the agent is moved back to the original spot.
 * @param agent Agent to move.
 */
void AgentCluster::moveRandomly(Agent *agent) {
    double initialX = agent->x;
    double initialY = agent->y;
    double initialHappiness = agent->happiness;


    double distance =  randomDouble(0.0, agent->effectiveRange * RANDOM_MOVE_FACTOR + 1);
    double xComponent = (double)rand() / (double)RAND_MAX * distance;
    if (rand() % 2)
        xComponent *= -1.0;
    double yComponent = (double)rand() / (double)RAND_MAX * distance;
    if (rand() % 2)
        yComponent *= -1.0;

    double pX = initialX + xComponent;
    double pY = initialY + yComponent;

    if (pX < m_dataMinX)
        pX = m_dataMinX;
    else if (pX > m_dataMaxX)
        pX = m_dataMaxX;
    if (pY < m_dataMinY)
        pY = m_dataMinY;
    else if (pY > m_dataMaxY)
        pY = m_dataMaxY;

    Q_ASSERT_X(nanTest(pX), "Failed NaN", __FUNCTION__);
    Q_ASSERT_X(nanTest(pY), "Failed NaN", __FUNCTION__);
    agent->x = pX;
    agent->y = pY;
    double newHappiness = calculateHappiness(agent);
    if (newHappiness >= initialHappiness) { //did we find a better position?
        agent->happiness = newHappiness;
        return;
    }

    //otherwise, move back...
    agent->x = initialX;
    agent->y = initialY;
}

/**
 * @brief AgentCluster::bestAgentInRange Finds the best Agent within the effective range, based on
 * Euclidean distance. If there are no agents within the given Agent's effective range, 0 is
 * returned.
 * @param agent The Agent to find the closest neighbor for.
 * @return The best Agent within range, or 0 if no Agents are found.
 */
Agent* AgentCluster::bestAgentInRange(Agent *agent) const {
    std::vector<Agent*> agents = agentsWithinPersonalSpace(agent);
    double bestHappiness = 0;
    Agent* bestAgent = 0;
    for (unsigned int i = 0; i < agents.size(); i++) {
        Agent* testAgent = agents[i];
        if (testAgent->happiness > bestHappiness) {
            bestHappiness = testAgent->happiness;
            bestAgent = testAgent;
        }
    }
    return bestAgent;
}

/**
 * @brief AgentCluster::agentsWithinEffectiveRange Finds Agents within the given Agent's effective
 * range.
 * @param agent The Agent to find neighbors for.
 * @return A vector of Agent objects within the Agent's effective range.
 */
std::vector<Agent*> AgentCluster::agentsWithinPersonalSpace(Agent *agent) const {
    std::vector<Agent*> closeAgents;
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent* testAgent = m_agents[i];
        if (testAgent == agent)
            continue;
        double distance = sqrt(pow(testAgent->x - agent->x, 2) + pow(testAgent->y - agent->y, 2));
        if (distance <= agent->personalSpace)
            closeAgents.push_back(testAgent);
    }
    return closeAgents;
}



/**
 * @brief AgentCluster::calculateHappiness Calculates the happiness of the Agent at its given
 * position, with a value between [0, 1].
 * @details The happiness of Agent i is related to both the crowding factor Ca(i), and the data
 * point concentration Cd(i).
 * @param agent The Agent to calculate happiness for.
 * @return Happiness value between [0, 1].
 * @warning Doesn't work -- in development
 */
double AgentCluster::calculateHappiness(Agent *agent) const {
    int neighboringAgents = (int) agentsWithinPersonalSpace(agent).size();
    double crowdingFactor = m_crowdingConcetrationSlope * (double)neighboringAgents + (double)1;

    int neighboringData = 0;
    for (unsigned int i = 0; i < m_data.size(); i++) {
        ClusterItem* item = m_data[i];
        double distance = pointDistance(agent->x, item->x, agent->y, item->y);
        if (distance <= agent->effectiveRange)
            neighboringData++;
    }

    double dataConcentration = m_dataConcentrationSlope * (double)(neighboringData);
    double totalScore = dataConcentration - crowdingFactor; //ranges from [-1, 1]. Normalize to [0, 1];
    totalScore = (totalScore + (double)1.0) / (double)2.0;

    return totalScore;
}


/**
 * @brief AgentCluster::averageClusterDistance Calculates the average distance between all data
 * points in the supplied data.
 * @return
 */
double AgentCluster::averageClusterDistance() const {
    if (m_data.size() == 0)
        return 1.0;

    int count = 0;
    double averageDistance = 0;
    for (unsigned int i = 0; i < m_data.size(); i++) {
        ClusterItem* itemOne = m_data[i];
        for (unsigned int j = 0; j < m_data.size(); j++) {
            if (i == j)
                continue;
            ClusterItem* itemTwo = m_data[j];
            averageDistance += sqrt(pow(itemTwo->x - itemOne->x, 2) + pow(itemTwo->y - itemOne->y, 2));
            count++;
        }
    }

    if (count == 0)
        return 1.0;

    averageDistance /= count;
    return averageDistance;
}

/**
 * @brief AgentCluster::pointDistance Euclidean distance between two points.
 * @param x1 X position of object 1
 * @param x2 X position of object 2
 * @param y1 Y position of object 1
 * @param y2 Y position of object 2
 * @return Euclidean distance between the two points.
 */
double AgentCluster::pointDistance(double x1, double x2, double y1, double y2) const {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/**
 * @brief AgentCluster::sleep Pauses the current thread for a designated amount of time.
 * @param milliseconds Milliseconds to pause for.
 */
void AgentCluster::sleep(int milliseconds) {
    QMutex mut;
    mut.lock();
    mut.tryLock(milliseconds);
    mut.unlock();
}
