xPos = numeric()
yPos = numeric()

lowerX = 100
lowerY = 100
xBound = 600
yBound = 600

clusters = sample(5:10, 1)
for (i in 1:clusters) {
	centerX = sample(lowerX+50:xBound, 1)
	centerY = sample(lowerY+50:yBound, 1)
	pointCount = sample(10:25, 1)
	xPos = append(xPos, rnorm(n=pointCount, mean=centerX, sd=25))
	yPos = append(yPos, rnorm(n=pointCount, mean=centerY, sd=25))
}

pointList <- list()
pointList = append(pointList, xPos)
pointList = append(pointList, yPos)

pointMatrix = matrix(pointList, ncol=2)

plot(xPos, yPos)
write.csv(pointMatrix, "artificialData.csv", row.names=FALSE)
