#include "agentcluster.h"

#include <iostream>
#include <fstream>
#include <QMutex>

#include <math.h>

AgentCluster::AgentCluster(int iterations, int swarmSize, QObject *parent)
    : QObject(parent)
{
    m_iterations = iterations;
    m_agentSensorRange = 0;
    m_agentStepSize = 0;
    m_dataMinX = 0;
    m_dataMaxX = 0;
    m_dataMinY = 0;
    m_dataMaxY = 0;
    m_minRange = 0;

    if (swarmSize <= 0)
        m_swarmSize = -1;
    else
        m_swarmSize = swarmSize;
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

    if (m_swarmSize == -1) {
        m_swarmSize = (int)((double)m_data.size() * SWARM_SIZE_FACTOR);
    }
    for (int i = 0; i < m_swarmSize; i++) {
        Agent* agent = new Agent();
        agent->x = randomDouble(m_dataMinX, m_dataMaxX);
        agent->y = randomDouble(m_dataMinY, m_dataMaxY);
        m_agents.push_back(agent);
    }
    printf("Created a swarm containing %i agents...\n", m_swarmSize);

    m_agentSensorRange = averageClusterDistance() * SENSOR_TO_AVG_DIST_RATIO;
    m_minRange = m_agentSensorRange * 0.2;

    m_agentStepSize = m_agentSensorRange * STEP_SIZE_TO_SENSOR_RATIO;
    m_dataConcentrationSlope = -(m_agentSensorRange - m_minRange);
    m_crowdingConcetrationSlope = (double)(-1) / m_agents.size();

    for (unsigned int i = 0; i < m_agents.size(); i++)
        m_agents[i]->foragingRange = m_agentSensorRange / 2.0;



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
            Agent* agent= m_agents[j];
            move(agent);
        }

        printf("Finished iteration %i...\n", i);
        if (i % 1 == 0) {
            emit update(&m_data, &m_agents);
            printf("\t...updated display\n");
            sleep(MOVEMENT_DELAY);
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
 * in the original AgentCluster paper. That is, the foraging range r_f is:
 *      r_f = alpha + (r_s - alpha)/(1 + beta * neighborCount)
 */
void AgentCluster::updateRanges() {
    for (unsigned int i = 0; i < m_agents.size(); i++) {
        Agent *agent = m_agents[i];


        int dataCount = (int) dataWithinForagingRange(agent).size();

        double r_f = m_minRange + ((m_agentSensorRange - m_minRange) / (1.0 + AGENT_BETA * (double)dataCount));

        agent->foragingRange = (r_f + agent->foragingRange) * 0.5;
        agent->crowdingRange = agent->foragingRange * CROWDING_TO_FORAGE_DIST_RATIO;
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
 * @brief AgentCluster::move Takes care of moving an Agent to its next location.
 * @param agent Agent to move.
 */
void AgentCluster::move(Agent *agent) {
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
    } else {    //all alone...
        std::vector<ClusterItem*> items = dataWithinForagingRange(agent);

        if (items.size() != 0) {    //alone, but with data?
            //Find the average position vector and move in that direction
            double avgX = 0;
            double avgY = 0;
            for (unsigned int i = 0; i < items.size(); i++) {
                ClusterItem* item = items[i];
                avgX += item->x - agent->x;
                avgY += item->y - agent->y;
            }
            avgX /= (double)items.size();
            avgY /= (double)items.size();
            double magnitude = randomDouble(0.1, 1);

            agent->x += avgX * magnitude;
            agent->y += avgY * magnitude;
        } else {                    //alone AND no data?
            moveRandomly(agent);
        }
    }
}

/**
 * @brief AgentCluster::moveTowards Moves the first agent towards the second one.
 * @param agentOne The agent being moved.
 * @param agentTwo The agent who is being moved towards.
 */
void AgentCluster::moveTowards(Agent *agentOne, Agent *agentTwo) {
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

    if (newX > m_dataMaxX)
        newX = m_dataMaxX;
    else if (newX < m_dataMinX)
        newX = m_dataMinX;
    if (newY > m_dataMaxY)
        newY = m_dataMaxY;
    else if (newY < m_dataMinY)
        newY = m_dataMinY;


    agentOne->x = newX;
    agentOne->y = newY;
    agentOne->happiness = calculateHappiness(agentOne);
}

/**
 * @brief AgentCluster::moveRandomly Moves the agent randomly across the search space, a distance
 * related to the Agent's foraging range. If the newly selected position has a lower hapiness
 * level than the original, the agent is moved back to the original spot.
 * @param agent Agent to move.
 */
void AgentCluster::moveRandomly(Agent *agent) {
    double initialX = agent->x;
    double initialY = agent->y;
    double initialHappiness = agent->happiness;

    double moveMagnitude =  randomDouble(0.0, agent->foragingRange * RANDOM_MOVE_FACTOR + 1);
    double moveDirection = randomDouble(0, 360) * (PI / 180.0);

    double posX = initialX + (cos(moveDirection) * moveMagnitude);
    double posY = initialY + (sin(moveDirection) * moveMagnitude);

    Q_ASSERT_X(nanTest(posX), "Failed NaN", __FUNCTION__);
    Q_ASSERT_X(nanTest(posY), "Failed NaN", __FUNCTION__);

    if (posX > m_dataMaxX)
        posX = m_dataMaxX;
    else if (posX < m_dataMinX)
        posX = m_dataMinX;
    if (posY > m_dataMaxY)
        posY = m_dataMaxY;
    else if (posY < m_dataMinY)
        posY = m_dataMinY;

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

/**
 * @brief AgentCluster::bestAgentInRange Finds the best Agent within the foraging range, based on
 * Euclidean distance. If there are no agents within the given Agent's foraging range, 0 is
 * returned.
 * @param agent The Agent to find the closest neighbor for.
 * @return The best Agent within range, or 0 if no Agents are found.
 */
Agent* AgentCluster::bestAgentInRange(Agent *agent) const {
    std::vector<Agent*> agents = agentsWithinForagingRange(agent);
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
 * @brief AgentCluster::dataWithinForagingRange Finds data points within the give Agent's foraging
 * range.
 * @param agent The Agent to find data points for.
 * @return A vector of ClusterItem objects within the Agent's foraging range.
 */
std::vector<ClusterItem*> AgentCluster::dataWithinForagingRange(Agent *agent) const {
    std::vector<ClusterItem*> items;
    for (unsigned int i = 0; i < m_data.size(); i++) {
        ClusterItem* item = m_data[i];
        double distance = pointDistance(agent->x, item->x, agent->y, item->y);
        if (distance <= agent->foragingRange)
            items.push_back(item);
    }
    return items;
}

/**
 * @brief AgentCluster::agentsWithinCrowdingRange Finds Agents within the given Agent's personal
 * range.
 * @param agent The Agent to find neighbors for.
 * @return A vector of Agent objects within the Agent's crowding range.
 */
std::vector<Agent*> AgentCluster::agentsWithinCrowdingRange(Agent *agent) const {
    return agentsWithinRange(agent, agent->crowdingRange);
}

/**
 * @brief AgentCluster::agentsWithinForagingRange Finds Agent objects within the given Agent's
 * foraging range.
 * @param agent The Agent to find neighbors for.
 * @return A vector of Agent objects within the foraging range.
 */
std::vector<Agent*> AgentCluster::agentsWithinForagingRange(Agent *agent) const {
    return agentsWithinRange(agent, agent->foragingRange);
}

/**
 * @brief AgentCluster::agentsWithinRange Finds Agent objects within the given range of the
 * Agent.
 * @param agent Agent to search around.
 * @param range Range of search.
 * @return A vector of Agent objects within the given range.
 */
std::vector<Agent*> AgentCluster::agentsWithinRange(Agent *agent, double range) const {
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
/**
 * @brief AgentCluster::calculateHappiness Calculates the happiness of the Agent at its given
 * position, with a value between [0, 1].
 * @details The happiness of Agent i is related to both the number of neighboring agents, and the
 * local data point concentration.
 * @param agent The Agent to calculate happiness for.
 * @return Happiness value between [0, 1].
 * @warning Uses a very basic linear function system. Needs to be updated for logistic style scaling
 * and to include the crowding/data concentration weights.
 */
double AgentCluster::calculateHappiness(Agent *agent) const {
    //h(i) = O(p_i) / (pi * r_f^2 *|A(p_i, r_c^i)| + 1)

    //For our clustering algorithm, the objective function is the percentage of data points located
    //within the foraging range of the agent.
    double objectiveFunctionValue = (double)dataWithinForagingRange(agent).size() / (double)m_data.size();

    double neighborScore = CROWDING_ADVERSION_FACTOR * (double)agentsWithinCrowdingRange(agent).size();
    double totalScore = objectiveFunctionValue / (double)((neighborScore * PI * pow(agent->foragingRange, 2)) + 1.0);
    //double totalScore = objectiveFunctionValue / (double)(neighborScore + 1.0);

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
