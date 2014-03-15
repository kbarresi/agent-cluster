#ifndef DEF_H
#define DEF_H

#include <vector>
#include <sstream>
#include <stdlib.h>
#include <cmath>

#define E 2.718281828459
#define PI 3.14159265

/**
 * @brief The ClusterItem struct Simple structure for holding values pertinent to inputted data.
 */
struct ClusterItem {
    int group;
    double x;
    double y;

    ClusterItem() {
        group = -1;
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

    double happiness;

    double foragingRange;
    double crowdingRange;
    double selectionRange;

    bool visited;
    int cluster;

    Agent() {
        x = y = 0;
        cluster = -1;
        happiness = 0.0;
        foragingRange = selectionRange = crowdingRange = 0.0;
        visited = false;
    }
};

struct Cluster {
    int id;
    std::vector<Agent*> agents;
    std::vector<ClusterItem*> points;

    Cluster() {
        id = 0;
    }
};


//Configuration parameters


/* The swarm size compared to the data set size. For example, a value of 0.5 means that the swarm
 * size will be half of the size of the given data set.
 */
static const double SWARM_SIZE_FACTOR = 1;

/* The ratio of the average datapoint-to-datapoint distance to agent sensor range.
 */
static const double SENSOR_TO_AVG_DIST_RATIO  = 0.5;

/* The ratio of the crowding range to the foraging range.
 */
static const double CROWDING_TO_FORAGE_DIST_RATIO = 0.2;

/* The ratio of agent sensor range to agent step size.
 */
static const double STEP_SIZE_TO_SENSOR_RATIO = 0.2;

/* Beta value used in updating agent effective range
 */
static const double AGENT_BETA = 10;

/* The distance an agent can randomly move across the space, as compared to the Agent's foraging range
 */
static const double RANDOM_MOVE_FACTOR = 1.0;

/* The amount agents dislike being crowded together. This is used as a constant in a decay function,
 * and determines how strongly agent crowding affects happiness.
 */
static const double CROWDING_ADVERSION_FACTOR = 10.0;

static const int CANVAS_SIZE = 800;
static const int AGENT_SIZE = 10;
static const int DATAPOINT_SIZE = 10;
static const int DEFAULT_SWARM_SIZE = 50;
static const int MOVEMENT_DELAY = 100;         //in milliseconds
static const int UPDATE_RATE = 5; //update display every x iterations
static const bool ANIMATED = true;
static const bool SHOW_DATA = true;
static const bool SHOW_FORAGE_RANGE = true;
static const bool SHOW_CROWDING_RANGE = false;
static const bool SHOW_PATH = false;

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


/**
 * @brief pointDistance Euclidean distance between two points.
 * @param x1 X position of object 1
 * @param x2 X position of object 2
 * @param y1 Y position of object 1
 * @param y2 Y position of object 2
 * @return Euclidean distance between the two points.
 */
static double pointDistance(double x1, double x2, double y1, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

inline bool nanTest(double x) { if (x != x) return false; else return true; }

#endif // DEF_H
