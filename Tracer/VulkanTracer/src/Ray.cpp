#include "Ray.h"
#include <cstring>

Ray::Ray(double xpos, double ypos, double zpos, double xdir, double ydir, double zdir, double w)
{
	position.x = xpos;
	position.y = ypos;
	position.z = zpos;
	direction.x = xdir;
	direction.y = ydir;
	direction.z = zdir;
	weight = w;
	placeholder = 0;
}
//copies 64 byts of data in the following format: xloc, yloc, zloc, weight, xdir, ydir, zdir, placeholder
Ray::Ray(double* location) {
	memcpy(&position.x, location, 64);
}
Ray::Ray() {}
Ray::~Ray() {

}

std::vector<double> Ray::getRayInformation()
{
	std::vector<double> rayInfo;
	rayInfo.push_back(position.x);
	rayInfo.push_back(position.y);
	rayInfo.push_back(position.z);
	rayInfo.push_back(direction.x);
	rayInfo.push_back(direction.y);
	rayInfo.push_back(direction.z);
	return rayInfo;
}

double Ray::getxDir()
{
	return direction.x;
}
double Ray::getyDir()
{
	return direction.y;
}

double Ray::getzDir()
{
	return direction.z;
}

double Ray::getxPos()
{
	return position.x;
}

double Ray::getyPos()
{
	return position.y;
}

double Ray::getzPos()
{
	return position.z;
}
double Ray::getWeight()
{
	return weight;
}
