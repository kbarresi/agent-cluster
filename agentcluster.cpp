#include "agentcluster.h"

#include <iostream>
#include <fstream>

AgentCluster::AgentCluster(AgentClusterVisualizer visualizeFunction)
{
    m_visualizer = visualizeFunction;
}

AgentCluster::~AgentCluster() {

}

/**
 * @brief AgentCluster::loadData Attempts to load the 2-column data from the filename given.
 * @param dataSource The filename of the file to load data from.
 * @return True for successful loading, false if there was an error.
 */
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

/**
 * @brief AgentCluster::start Runs the AgentCluster algorithm.
 * @details It initializes the agent population, and then simply calls the functions built for each
 * of the three main clustering phases. Finally, it calls the AgentClusterVisualizer function in
 * order to visualize the resulting cluster items and the agents.
 * @param iterations Number of iterations the convergence phase should run for.
 */
void AgentCluster::start(int iterations) {

    //Init the population to random positions;
    double minX = m_data[0]->x;
    double minY = m_data[0]->y;
    double maxX = minX;
    double maxY = minY;
    for (int i = 1; i < m_data.size(); i++) {
        ClusterItem* item = m_data[i];
        if (item->x > maxX)
            maxX = item->x;
        else if (item->x < minX)
            minX = item->x;

        if (item->y > maxY)
            maxY = item->y;
        else if (item->y < minY)
            minY = item->y;
    }

    int swarmSize = (int)((double)m_data.size() * SWARM_SIZE_FACTOR);
    for (int i = 0; i < swarmSize; i++) {
        Agent* agent = new Agent();
        agent->x = randomDouble(minX, maxX);
        agent->y = randomDouble(minY, maxY);
        m_agents.push_back(agent);
    }
    printf("Created a swarm containing %i agents...\n", swarmSize);

    //Start each of the three clustering phases, in order
    convergencePhase(iterations);
    consolidationPhase();
    assignmentPhase();


    //Now we want to see the results;
    (m_visualizer)(m_data, m_agents);
}

/**
 * @brief AgentCluster::convergencePhase Runs the convergence phase of the AgentSwarm algorithm
 * for a number of times determined by the iteration paramter.
 * @param iterations The number of iterations to run for.
 */
void AgentCluster::convergencePhase(int iterations) {
    (void)(iterations);
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
 * in the original AgentCluster paper.
 */
void AgentCluster::updateRanges() {
    for (int i = 0; i < m_agents.size(); i++) {
        Agent *agent = m_agents[i];
        (void)(agent);
    }
}

/**
 * @brief AgentCluster::updateHappiness Update the hapiness of all agents, as based on their current
 * position.
 */
void AgentCluster::updateHappiness() {
    for (int i = 0; i < m_agents.size(); i++) {
        Agent* agent = m_agents[i];
        (void)(agent);
    }
}

/**
 * @brief AgentCluster::moveTowards Moves the first agent towards the second one.
 * @details If the new position results in lower happiness than the original position results in
 * lower happiness than the original position, the agent will move back, and attempt to move to a
 * different, random position within the entire data-space. If this new position is better than or
 * equal to the original position in terms of happiness, it will stay there. Otherwise, it will
 * stay in the original position.
 * @param agentOne The agent being moved.
 * @param agentTwo The agent who is being moved towards.
 */
void AgentCluster::moveTowards(Agent *agentOne, Agent *agentTwo) {
    (void)(agentOne);
    (void)(agentTwo);
}


/**
 * @brief AgentCluster::closestAgent Finds the closest Agent, based on Euclidean distance.
 * @param agent The Agent to find the closest neighbor for.
 * @return The closest Agent to the given Agent.
 */
Agent* AgentCluster::closestAgent(Agent *agent) const {
    return agent;
}

/**
 * @brief AgentCluster::nearbyAgents Finds Agents within the given Agent's effective range.
 * @param agent The Agent to find neighbors for.
 * @return A vector of Agent objects within the Agent's effective range.
 */
std::vector<Agent*> AgentCluster::nearbyAgents(Agent *agent) const {
    (void)(agent);
    return std::vector<Agent*>();
}

/**
 * @brief AgentCluster::calculateHappiness Calculates the happiness of the Agent at its given
 * position, with a value between [0, 1].
 * @param agent The Agent to calculate happiness for.
 * @return Happiness value between [0, 1].
 */
double AgentCluster::calculateHappiness(Agent *agent) const {
    (void)(agent);
    return 0;
}
