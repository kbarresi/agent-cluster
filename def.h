#ifndef DEF_H
#define DEF_H

#include <vector>
#include <sstream>

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

typedef void (*ClusterVisualizer)(std::vector<ClusterItem*>);
typedef void (*AgentClusterVisualizer)(std::vector<ClusterItem*>, std::vector<Agent*>);





//UTIL FUNCTIONS
static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}



#endif // DEF_H
