library(mclust)

data = read.csv("agreggation.csv", sep = ",")
model = Mclust(data)

surfacePlot(data, parameters = model$parameters, type = "contour", 
            what = "density", transformation = "none", drawlabels = FALSE,
            nlevels = 15)
