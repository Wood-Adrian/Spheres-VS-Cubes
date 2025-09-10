#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <ogcsys.h>
#include <gccore.h>

//common vector functions that are just kinda useful

float VectorDistance(guVector vect1, guVector vect2);
guVector DirectionUnitVector(guVector location, guVector destination);
bool VectorSurpassedPoint(guVector position, guVector destination, guVector direction);