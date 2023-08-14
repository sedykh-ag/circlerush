#pragma once

#include <stdlib.h>

double randrange(double xmin, double xmax) {
  return ((double)rand() / RAND_MAX) * (xmax - xmin) + xmin;
}