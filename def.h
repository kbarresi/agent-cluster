#ifndef DEF_H
#define DEF_H

#include <vector>
#include <sstream>

/**
 * @brief The ClusterItem struct Simple structure for holding values pertinent to inputted data.
 */
struct ClusterItem {
    int group;
    double x;
    double y;

    ClusterItem() {
        group = 0;
        x = 0;
        y = 0;
    }
};

/**
 * @brief The Agent struct Contains data representing a single agent.
 */
struct Agent {
    double x;
    double y;

    float happiness;

    double effectiveRange;
    double selectionRange;

    Agent() {
        x = y = 0;
        happiness = 0.0;
        effectiveRange = selectionRange = 0.0;
    }
};

typedef void (*AgentClusterVisualizer)(std::vector<ClusterItem*>, std::vector<Agent*>);



//Configuration parameters

/* The swarm size compared to the data set size. For example, a value of 0.5 means that the swarm
 * size will be half of the size of the given data set.
 */
static double SWARM_SIZE_FACTOR = 0.5;






//UTIL FUNCTIONS

/**
 * @brief split Split an std::string into a vector of strings.
 * @param s The string to split.
 * @param delim The delimiting character.
 * @param elems A string vector to write to.
 * @return A vector of strings that the string was split into.
 */
static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
/**
 * @brief split Split an std::string into a vector of strings.
 * @param s The string to split.
 * @param delim The delimiting character.
 * @return A vector of strings that the string was split into.
 */
static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

/**
 * @brief randomDouble Generates a random double within a given range.
 * @param min The bottom of the range.
 * @param max The top of the range.
 * @return The random double.
 */
static double randomDouble(double min, double max) {
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}


#endif // DEF_H
