library(RColorBrewer)

points = data.matrix(read.table("cluster_results.csv", sep = ","))
clusterNumber = max(points[,3]) + 1


palette(brewer.pal(clusterNumber,"Set2"))
#palette(rainbow(clusterNumber))
plot(points, col = as.factor(points[,3]), xlab = "x", ylab = "y")