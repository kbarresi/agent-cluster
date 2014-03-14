Foraging Agent Swarm Optimization (FASO)
=============

### Overview

FASO is a multi-agent optimization algorithm inspired by Glowworm Swarm Optimization (GSO) and Bacterial Foraging Optimization (BFO). This repository contains an implementation of a generic FASO algorithm that is used to find minimums of a Styblinski–Tang function. 

It also contains an implementation of Foraging Agent Swarm Clustering (FASC), an application of FASO to optimizing data cluster analysis. Simply provide a dataset to cluster, and you can watch the FASC agents move around to their clusters. FASC has several benefits over other clustering techniques, including:
- No need to specify number of clusters.
- Works equally well for centroid/density based cluster styles.
- Highly parallelizable due to distributed nature.


### Usage

    Usage: AgentSwarm [options] <data.csv>

Options

    -c	Use clustering (FASC) mode. If used, data.csv must be given. Default: false
    -n	Number of iterations to run Default: 100
    -s	Number of agents in swarm. Default: 50
    -i  Number of swarm instances to run. Default: 1
