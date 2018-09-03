#pragma once

#ifndef OPENGL_H
#define OPENGL_H

#include "image_parameters.h"

int initialiseWindow(image_parameters* image, bool *windowOpen);
void terminateWindow();

#endif //OPENGL_H