ackleyFunction = function(x, y) {
  return (-20 * exp(-0.2 * sqrt(0.5 * (x^2 + y^2))) - exp(0.5*(cos(2 * pi * x) + cos(2 * pi * y))) + 20 + exp(1))
}

sixHumpCamelFunction = function(x, y) {
  return ((4-2.1*x^2+x^4/3)*x^2+x*y+(-4+4*y^2)*y^2)  
}

styblinksiFunction = function(x, y) {
  return ( ((x^4 - 16*x^2 + 5*x) + (y^4 - 16*y^2 + 5*y)) / 2   )
}
upperBoundX = 5;
upperBoundY = 5;
lowerBoundX = -5;
lowerBoundY = -5;

samples = 250;

x = seq(from = lowerBoundX, to = upperBoundX, length.out = samples)
y = seq(from = lowerBoundY, to = upperBoundY, length.out = samples)
z = matrix(0, samples, samples)
for (i in 0:samples) {
  for (j in 0:samples) {
    z[i,j] = styblinksiFunction(x[i], y[j])
  }
}

filled.contour(x, y, z, color.palette = heat.colors)