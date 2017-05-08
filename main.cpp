// Matthew Chiborak
//250748631
//mchibora@uwo.ca
//CS3388 Assignment 4
//Create a scene with three spheres bouncing on a plane that has been drawn with ray casting

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else 
#include <GL/glut.h>
#endif

//Equation for a sphere
//(x-x_0)^2+(y-y_0)^2+(z-z_0)^2=R^2. 

//Spheres bounce min and max heights. When hit or go over this point, switch directions
#define MAX_HEIGHT 5.0
#define MIN_HEIGHT 0.0

//Coordinates for the screen size to be used for ray casting
#define WINDOW_POSITION_X 0.0
#define WINDOW_POSITION_Y 0.0
#define SCREEN_WIDTH 500.0
#define SCREEN_HEIGHT 500.0
#define NUMBER_OF_SPHERES 3

//Camera Values (at.x, at.y, at.z, lookat.x, lookat.y, lookat.z, up.x, up.y, up.z)
//Simulated Camera Values
float cameraValues[] = { 0.0, 3.0, -10.0, 0, 0, 0, 0, 1, 0 };
float distanceFromCamToView = -5.0;

//The camera values for the real camera
float realCameraValues[] = { 0, 0, 1, 0, 0, 0, 0, 1, 0 };

//View max and min cordinates
float uMin = -2.5;
float uMax = 2.5;
float vMin = 0.0;
float vMax = 5.0;

//Light source in the scene
float lightSource[] = { -7.0, 5.0, -10.0 };

//Shading factors
float diffuse = 0.3;
float ambient = 0.2;
float specular = 0.5;
float reflectiveness = 5.0;
float lightIntensity = 1.0;

//Variables for the ground
float groundColour[3] = { 1.0, 1.0, 1.0 };

//Error value for fixing precision issues with shadows
float shadowError = 0.01;

//Class for sphere
class Sphere
{
private:
	float radius;
	float speed;
	float colour[3];
	int direction;
	float position[3];

public:
	Sphere()
	{
		this->radius = 1.0;
		this->position[0] = 0.0;
		this->position[1] = 0.0;
		this->position[2] = 0.0;
		this->speed = 0.0;
		this->colour[0] = 1.0;
		this->colour[1] = 1.0;
		this->colour[2] = 1.0;
		direction = 1;
	}

	Sphere(float radius, float x, float y, float z, float speed, float colour1, float colour2, float colour3)
	{
		this->radius = radius;
		this->position[0] = x;
		this->position[1] = y;
		this->position[2] = z;
		this->speed = speed;
		this->colour[0] = colour1;
		this->colour[1] = colour2;
		this->colour[2] = colour3;
		direction = 1;
	};

	//Increase the spheres position by its speed value in the direction of its direction value
	void move()
	{
		position[1] += (direction * speed);

		//Check if need switch directions
		if (direction > 0 && position[1] + radius >= MAX_HEIGHT)
		{
			direction *= -1;
		}
		if (direction < 0 && position[1] - radius <= MIN_HEIGHT)
		{
			direction *= -1;
		}
	}

	float getXPos()
	{
		return position[0];
	}
	float getYPos()
	{
		return position[1];
	}
	float getZPos()
	{
		return position[2];
	}
	float getRadius()
	{
		return radius;
	}
	float getRed()
	{
		return colour[0];
	}
	float getGreen()
	{
		return colour[1];
	}
	float getBlue()
	{
		return colour[2];
	}
};

//Create the 3 sphere objects
Sphere mySpheres[NUMBER_OF_SPHERES] = { Sphere(0.66, -1.5, 2.0, -3.0, 0.3, 0.0, 1.0, 0.0), Sphere(1.00, 0.0, 2.0, 0.0, 0.2, 1.0, 0.0, 0.0), Sphere(1.33, 1.5, 2.0, 3.0, 0.1, 0.0, 0.0, 1.0) };

void drawWithRayCasting()
{
	//Ray from the eye through the pixil in the viewport
	float ray[3];
	float underSquareRoot[3]; //The ab and c under the square root in the equation

	//w = -(lookat - eye) / ||g||
	float g[3];
	g[0] = -1 * (cameraValues[3] - cameraValues[0]);
	g[1] = -1 * (cameraValues[4] - cameraValues[1]);
	g[2] = -1 * (cameraValues[5] - cameraValues[2]);
	float gMag = sqrt(g[0] * g[0] + g[1] * g[1] + g[2] * g[2]);

	float w[3];
	w[0] = g[0] / gMag;
	w[1] = g[1] / gMag;
	w[2] = g[2] / gMag;

	//Convert to viewing to find the viewing plane
	//u = t x w / ||t x w||
	float u[3];
	u[0] = (cameraValues[7] * w[2] - cameraValues[8] * w[1]);
	u[1] = (cameraValues[8] * w[0] - cameraValues[6] * w[2]);
	u[2] = (cameraValues[6] * w[1] - cameraValues[7] * w[0]);
	float uMag = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	u[0] = u[0] / uMag;
	u[1] = u[1] / uMag;
	u[2] = u[2] / uMag;

	//v = w x u
	float v[3];
	v[0] = w[1] * u[2] - w[2] * u[1];
	v[1] = w[2] * u[0] - w[0] * u[2];
	v[2] = w[0] * u[1] - w[1] * u[0];

	//V2W -> u[x] v[x] w[x] e[x]   * point
	//		u[y] v[y] w[y] e[y]
	//		u[z] v[z] w[z] e[z]
	//		0 0 0 1

	//For Every Pixil
	for (float y = 0; y < SCREEN_HEIGHT; y++)
	{
		for (float x = 0; x < SCREEN_WIDTH; x++)
		{
			//Pixil in world Space
			float pixilView[3];
			float pixilWorld[3];
			//Note depth in pixil world will always be zero
			//Also take from the center of the pixil
			
			//ray p(t) = e + td

			//Viewport to view
			pixilView[0] = ((x + 0.5) - 0) * ((uMax - (uMin)) / (SCREEN_WIDTH - (-0))) + (uMin);
			pixilView[1] = ((y + 0.5) - 0) * ((vMax - (vMin)) / (SCREEN_HEIGHT - (-0))) + (vMin);
			pixilView[2] = distanceFromCamToView; 

			//View to world
			pixilWorld[0] = u[0] * pixilView[0] + v[0] * pixilView[1] + w[0] * pixilView[2] + cameraValues[0];
			pixilWorld[1] = u[1] * pixilView[0] + v[1] * pixilView[1] + w[1] * pixilView[2] + cameraValues[1];
			pixilWorld[2] = u[2] * pixilView[0] + v[2] * pixilView[1] + w[2] * pixilView[2] + cameraValues[2];

			//Values for keeping track of the closest sphere
			float closestT = 1;
			int closestSphere = -1;
			float closestPosition[3];

			//Construct a ray from the eye through viewport (at 0,0)
			ray[0] = pixilWorld[0] - cameraValues[0];
			ray[1] = pixilWorld[1] - cameraValues[1];
			ray[2] = pixilWorld[2];

			//Values for the ab and c variables in the equation for finding how many points a ray interects a sphere
			//a = ||d|| ^ 2
			//b = ed
			//c = ||e||^2 - 1

			//For every object in the scene
			for (int sphereIndex = 0; sphereIndex < NUMBER_OF_SPHERES; sphereIndex++)
			{
				//Find intersecction with the ray
				//Find the ab and c values using the ray and the sphere
				underSquareRoot[0] = ray[0] * ray[0] + ray[1] * ray[1] + ray[2] * ray[2];
				underSquareRoot[1] = 2 * ray[0] * (cameraValues[0] - mySpheres[sphereIndex].getXPos()) + 2 * ray[1] * (cameraValues[1] - mySpheres[sphereIndex].getYPos()) + 2 * ray[2] * (cameraValues[2] - mySpheres[sphereIndex].getZPos());
				underSquareRoot[2] = mySpheres[sphereIndex].getXPos() * mySpheres[sphereIndex].getXPos() + mySpheres[sphereIndex].getYPos() * mySpheres[sphereIndex].getYPos() + mySpheres[sphereIndex].getZPos() * mySpheres[sphereIndex].getZPos() + cameraValues[0] * cameraValues[0] + cameraValues[1] * cameraValues[1] + cameraValues[2] * cameraValues[2] + -2 * (mySpheres[sphereIndex].getXPos() * cameraValues[0] + mySpheres[sphereIndex].getYPos() * cameraValues[1] + mySpheres[sphereIndex].getZPos() * cameraValues[2]) - mySpheres[sphereIndex].getRadius() * mySpheres[sphereIndex].getRadius();

				float underSquareRootValue = underSquareRoot[1] * underSquareRoot[1] - 4 * underSquareRoot[0] * underSquareRoot[2];

				//If > 0, intersect at 2 points. If == 0, grazes a point. If <0, completely misses
				if (underSquareRootValue > 0)
				{
					//Ray intersects sphere in two points
					//+/- for the two points. The closer point (the one closer to the camera), will be the negative square root
					float tValue = (underSquareRoot[1] - sqrt(underSquareRoot[1] * underSquareRoot[1] - 4 * underSquareRoot[0] * underSquareRoot[2])) / (2 * underSquareRoot[0]);

					//Keep closest sphere to the camera, meaning the lowest t value
					if (closestSphere == -1 || tValue < closestT)
					{
						closestT = tValue;
						closestSphere = sphereIndex;
						closestPosition[0] = cameraValues[0] + tValue * -1 * ray[0];
						closestPosition[1] = cameraValues[1] + tValue * -1 * ray[1];
						closestPosition[2] = cameraValues[2] + tValue * -1 * ray[2];
					}
				}
			}

			//Only draw the closest sphere because any other spheres will be obsured
			if (closestSphere >= 0)
			{
				//Normal. just consider the point thats hit as a vector (for a generic sphere) So minus its xyz cordnates to get back to the origin. Also normalize the vector
				float normal[3];
				normal[0] = (closestPosition[0] - mySpheres[closestSphere].getXPos()) / mySpheres[closestSphere].getRadius();
				normal[1] = (closestPosition[1] - mySpheres[closestSphere].getYPos()) / mySpheres[closestSphere].getRadius();
				normal[2] = (closestPosition[2] - mySpheres[closestSphere].getZPos()) / mySpheres[closestSphere].getRadius();
				
				float normalMag = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
				normal[0] = normal[0] / normalMag;
				normal[1] = normal[1] / normalMag;
				normal[2] = normal[2] / normalMag;

				//Light unit vector from the point to the light
				float lightVector[3];
				lightVector[0] = lightSource[0] - closestPosition[0];
				lightVector[1] = lightSource[1] - closestPosition[1];
				lightVector[2] = lightSource[2] - closestPosition[2];

				float lightMag = sqrt(lightVector[0] * lightVector[0] + lightVector[1] * lightVector[1] + lightVector[2] * lightVector[2]);
				lightVector[0] = lightVector[0] / lightMag;
				lightVector[1] = lightVector[1] / lightMag;
				lightVector[2] = lightVector[2] / lightMag;

				//Find the light intensity (normals dot product with the light direction)
				float intensity = normal[0] * lightVector[0] + normal[1] * lightVector[1] + normal[2] * lightVector[2];

				//Add the specular phong highlights
				// R = -L + 2(N dot L) * N
				float R[3];
				R[0] = -1 * lightVector[0] + 2 * intensity * normal[0];
				R[1] = -1 * lightVector[1] + 2 * intensity * normal[1];
				R[2] = -1 * lightVector[2] + 2 * intensity * normal[2];

				//I = KI(R dot V)^n 
				float viewMag = sqrt(ray[0] * ray[0] + ray[1] * ray[1] + ray[2] * ray[2]);
				float view[3];
				view[0] = ray[0] / viewMag;
				view[1] = ray[1] / viewMag;
				view[2] = ray[2] / viewMag;

				float specularIntensity = specular * pow((R[0] * view[0] + R[1] * view[1] + R[2] * view[2]), reflectiveness);

				//See if the object is obsurded so its in shadow
				bool inShadow = false;

				//Form a ray from the point to the light source
				float shadowRay[3];
				shadowRay[0] = lightSource[0] - closestPosition[0];
				shadowRay[1] = lightSource[1] - closestPosition[1];
				shadowRay[2] = lightSource[2] - closestPosition[2];

				//To account for shadow precision error
				closestPosition[0] = closestPosition[0] - shadowError * shadowRay[0];
				closestPosition[1] = closestPosition[1] - shadowError * shadowRay[1];
				closestPosition[2] = closestPosition[2] - shadowError * shadowRay[2];

				float groundUnderSquareRoot[3];

				//Check if intercept a sphere. If so, only draw the ambient lighted colour. If not, ambient, specular, and diffuse. 
				//Check all the objects
				for (int sphereIndex = 0; sphereIndex < NUMBER_OF_SPHERES; sphereIndex++)
				{
					//Stop colliding with itself
					if (sphereIndex == closestSphere)
					{
						continue;
					}

					//Calculate the square root value
					groundUnderSquareRoot[0] = shadowRay[0] * shadowRay[0] + shadowRay[1] * shadowRay[1] + shadowRay[2] * shadowRay[2];
					groundUnderSquareRoot[1] = 2 * shadowRay[0] * (closestPosition[0] - mySpheres[sphereIndex].getXPos()) + 2 * shadowRay[1] * (closestPosition[1] - mySpheres[sphereIndex].getYPos()) + 2 * shadowRay[2] * (closestPosition[2] - mySpheres[sphereIndex].getZPos());
					groundUnderSquareRoot[2] = mySpheres[sphereIndex].getXPos() * mySpheres[sphereIndex].getXPos() + mySpheres[sphereIndex].getYPos() * mySpheres[sphereIndex].getYPos() + mySpheres[sphereIndex].getZPos() * mySpheres[sphereIndex].getZPos() + closestPosition[0] * closestPosition[0] + closestPosition[1] * closestPosition[1] + closestPosition[2] * closestPosition[2] + -2 * (mySpheres[sphereIndex].getXPos() * closestPosition[0] + mySpheres[sphereIndex].getYPos() * closestPosition[1] + mySpheres[sphereIndex].getZPos() * closestPosition[2]) - mySpheres[sphereIndex].getRadius() * mySpheres[sphereIndex].getRadius();

					float underSquareRootValueGround = groundUnderSquareRoot[1] * groundUnderSquareRoot[1] - 4 * groundUnderSquareRoot[0] * groundUnderSquareRoot[2];

					//Intercects a sphere
					if (underSquareRootValueGround > 0)
					{
						//Make sure the point is atually infront the point
						float tValueShadow = (groundUnderSquareRoot[1] - sqrt(groundUnderSquareRoot[1] * groundUnderSquareRoot[1] - 4 * groundUnderSquareRoot[0] * groundUnderSquareRoot[2])) / (2 * groundUnderSquareRoot[0]);

						if (tValueShadow < 0)
						{
							//Point is obsured from the light source, so it is in shadow
							inShadow = true;
							//Found that the sphere is already in shadow. No need to check the remaining objects
							break;
						}
					}
				}
				
				//Find the final colour
				float finalColour[3];
				if (inShadow)
				{
					finalColour[0] = ambient * mySpheres[closestSphere].getRed();
					finalColour[1] = ambient * mySpheres[closestSphere].getGreen();
					finalColour[2] = ambient * mySpheres[closestSphere].getBlue();
				}
				else
				{
					finalColour[0] = ambient * mySpheres[closestSphere].getRed() + diffuse * lightIntensity * intensity * mySpheres[closestSphere].getRed() + specularIntensity * mySpheres[closestSphere].getRed();
					finalColour[1] = ambient * mySpheres[closestSphere].getGreen() + diffuse * lightIntensity * intensity * mySpheres[closestSphere].getGreen() + specularIntensity * mySpheres[closestSphere].getGreen();
					finalColour[2] = ambient * mySpheres[closestSphere].getBlue() + diffuse  * lightIntensity * intensity * mySpheres[closestSphere].getBlue() + specularIntensity * mySpheres[closestSphere].getBlue();
				}

				//Draw the pixil
				glPushMatrix();
				glColor3f(finalColour[0], finalColour[1], finalColour[2]);
				glBegin(GL_QUADS);
				glVertex3f(x, y, 0);
				glVertex3f(x, y + 1, 0);
				glVertex3f(x + 1, y + 1, 0);
				glVertex3f(x + 1, y, 0);
				glEnd();
				glPopMatrix();
			}
			else if (-1 * ray[1] < MIN_HEIGHT) //Ray didn't hit any spheres but may have hit ground plane. Check if the ray goes below the plane value and if it does the ground colour pixil
			{
				//Check if is actually being covered so is in shadow 
				bool inShadow = false;

				//Form from the point to the light source
				// = P + tL
				float shadowRay[3];
				//point = e + t(e-s). What t makes point.y = plane value. 
				//t = yvalue -e.y / (e.y - s.y)
				float groundT = (MIN_HEIGHT - cameraValues[1]) / (ray[1]);
				float groundPoint[3];

				groundPoint[0] = cameraValues[0] + groundT * (ray[0]);
				groundPoint[1] = cameraValues[1] + groundT * (ray[1]); //Should be zero
				groundPoint[2] = cameraValues[2] + groundT * (ray[2]);

				shadowRay[0] = lightSource[0] - groundPoint[0];
				shadowRay[1] = lightSource[1] - groundPoint[1];
				shadowRay[2] = lightSource[2] - groundPoint[2];

				float groundUnderSquareRoot[3];

				//Check if intercept a sphere. If so don't draw the pixil. If not, draw the ground pixil. If so, its obsured so its in shadow
				for (int sphereIndex = 0; sphereIndex < NUMBER_OF_SPHERES; sphereIndex++)
				{
					groundUnderSquareRoot[0] = shadowRay[0] * shadowRay[0] + shadowRay[1] * shadowRay[1] + shadowRay[2] * shadowRay[2];
					groundUnderSquareRoot[1] = 2 * shadowRay[0] * (groundPoint[0] - mySpheres[sphereIndex].getXPos()) + 2 * shadowRay[1] * (groundPoint[1] - mySpheres[sphereIndex].getYPos()) + 2 * shadowRay[2] * (groundPoint[2] - mySpheres[sphereIndex].getZPos());
					groundUnderSquareRoot[2] = mySpheres[sphereIndex].getXPos() * mySpheres[sphereIndex].getXPos() + mySpheres[sphereIndex].getYPos() * mySpheres[sphereIndex].getYPos() + mySpheres[sphereIndex].getZPos() * mySpheres[sphereIndex].getZPos() + groundPoint[0] * groundPoint[0] + groundPoint[1] * groundPoint[1] + groundPoint[2] * groundPoint[2] + -2 * (mySpheres[sphereIndex].getXPos() * groundPoint[0] + mySpheres[sphereIndex].getYPos() * groundPoint[1] + mySpheres[sphereIndex].getZPos() * groundPoint[2]) - mySpheres[sphereIndex].getRadius() * mySpheres[sphereIndex].getRadius();

					float underSquareRootValueGround = groundUnderSquareRoot[1] * groundUnderSquareRoot[1] - 4 * groundUnderSquareRoot[0] * groundUnderSquareRoot[2];

					//Intercects a sphere
					if (underSquareRootValueGround > 0)
					{
						inShadow = true;
						break;
					}
				}

				//Draw the pixil
				glPushMatrix();
				
				if (!inShadow)
				{
					glColor3f(groundColour[0], groundColour[1], groundColour[2]);
				}
				else
				{
					glColor3f(groundColour[0] * ambient, groundColour[1] * ambient, groundColour[2] * ambient);
				}
				
				glBegin(GL_QUADS);
				glVertex3f(x, y, 0);
				glVertex3f(x, y + 1, 0);
				glVertex3f(x + 1, y + 1, 0);
				glVertex3f(x + 1, y, 0);
				glEnd();
				glPopMatrix();
			}
		}
	}
}


void display()
{
	//Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//Draw the sphere using ray casting
	drawWithRayCasting();

	glPopMatrix();

	//Swap the buffer
	glutSwapBuffers();
}

//Function to call of the functions needed to be called when idling
void idleFunctions()
{
	//Move the spheres to their new positions
	for (int i = 0; i < 3; i++)
	{
		mySpheres[i].move();
	}

	//Force a redraw to the screen
	glutPostRedisplay();
}

//Set up the inital states of the scene
void init()
{
	//Turn on depth
	glEnable(GL_DEPTH_TEST);

	//Set Black background
	glClearColor(0.0, 0.0, 0.0, 1.0);

	//Set up persepective
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60, 4/3, 1, 80);
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -2, 2);

	//Set Up Camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(realCameraValues[0], realCameraValues[1], realCameraValues[2], realCameraValues[3], realCameraValues[4], realCameraValues[5], realCameraValues[6], realCameraValues[7], realCameraValues[8]);
}

int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(WINDOW_POSITION_X + 100, WINDOW_POSITION_Y + 100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("CS3388 Assignment 4");
	glutDisplayFunc(display);
	glutIdleFunc(idleFunctions);

	init();

	glutMainLoop();
}

