#include "Camera.h"

Camera::Camera(float near, float far, float bottom, float top, int width, int height)
{
	this->near_ = near;
	this->far_ = far;
	this->bottom_ = bottom;
	this->top_ = top;
	this->width_ = width;
	this->height_ = height;
}
