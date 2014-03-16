library(RColorBrewer)
library(fpc)

data = read.csv("jain.csv", sep = ",");
clusterNumber = 8
#(cl <- kmeans(data, clusterNumber))

palette(brewer.pal(clusterNumber, "Set2"))
#plot(data, col = cl$cluster)

d <- dbscan(data, 2, showplot = TRUE, MinPts = 5, method = c("hybrid"));
d;               