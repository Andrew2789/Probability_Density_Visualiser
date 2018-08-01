#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <GL/freeglut.h>

using vec4d = std::vector<std::vector<std::vector<std::vector<float>>>>;
using uint = unsigned int;

std::vector<vec4d> contour_sets;

//float lgt_pos[] = {-1.0f, 20.0f, 2.0f, 1.0f};
float deg_to_rad = 3.14159265358979/180;
float cam_pos_angle = 0.0f;
float cam_pos_radius = 3.5f;
float cam_pos[] = {cam_pos_radius, 0.0f, 0.0f};

signed char rotate_dir = 0, vert_dir = 0, horiz_dir = 0, speed = 1, x_flip = 0, ortho = 0;
int draw_index = -1, line_width = 2;

float aspect_ratio = 1.0f;

//Load the contour data from a binary contours file, and add each contour set to the contour_sets vector
char load_contours(const char* file_name) {
	std::ifstream file(file_name, std::ios::binary);

	if (file.is_open()) {
		uint num_contour_sets, num_levels, num_contours, num_points;

		file.read((char*)&num_contour_sets, sizeof(uint)); //read number of levels from bin
		file.read((char*)&num_levels, sizeof(uint)); //read number of levels from bin
		for (uint contour_set_index = 0; contour_set_index < num_contour_sets; contour_set_index++) {
			vec4d contour_set;
			contour_set.resize(num_levels); //initialize pointers for each level
			std::cout << "Reading in " << num_levels << " level(s) for contour set " << contour_set_index << "..." << std::endl;

			for (uint level = 0; level < num_levels; level++) {
				file.read((char*)&num_contours, sizeof(uint)); //read the number of contours in the current level
				contour_set[level].resize(num_contours); //initialize pointers for each contour
				std::cout << "Reading " << num_contours << " contours for level " << level << "..." << std::endl;

				for (uint contour = 0; contour < num_contours; contour++) {
					file.read((char*)&num_points, sizeof(uint)); //read the number of points in the current contour
					contour_set[level][contour].resize(num_points); //initialize pointers for each contour

					for (uint point = 0; point < num_points; point++) { //read the contour points in
						contour_set[level][contour][point].resize(2);
						file.read((char*)&contour_set[level][contour][point][0], sizeof(float)*2);
					}
				}
			}
			contour_sets.push_back(contour_set);
		}
		file.close();
		std::cout << "Done!" << std::endl;
		return 1;
	} else {
		std::cout << "Unable to open contours.bin" << std::endl;
	}
	return 0;
}

//Generate a colour based on a value from 0 - 1
void set_colour(float value, float* colours) {
	float normalized_value;
	if (value < 0.2) {
		normalized_value = value*5;
		colours[0] = 1.0f;
		colours[1] = normalized_value;
		colours[2] = 0;
	} else if (value < 0.4) {
		normalized_value = (value - 0.2)*5;
		colours[0] = 1.0f - normalized_value;
		colours[1] = 1.0f;
		colours[2] = 0;
	} else if (value < 0.6) {
		normalized_value = (value - 0.4)*5;
		colours[0] = 0;
		colours[1] = 1.0f;
		colours[2] = normalized_value;
	} else if (value < 0.8) {
		normalized_value = (value - 0.6)*5;
		colours[0] = 0;
		colours[1] = 1.0f - normalized_value;
		colours[2] = 1.0f;
	} else {
		normalized_value = (value - 0.8)*5;
		colours[0] = normalized_value;
		colours[1] = 0;
		colours[2] = 1.0f;
	}
}

void draw_axis() {
	float minor_tick = 0.02, major_tick = 0.05;
	glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(1.2, 0, 0);

		for (int i = 1; i < 13; i++) { //draw ticks
			glVertex3f(0.1*i, 0, 0);
			if (i == 10) {
				glVertex3f(0.1*i, 0, major_tick);
			} else {
				glVertex3f(0.1*i, 0, minor_tick);
			}
			glVertex3f(0.1*i, 0, 0);
			if (i == 10) {
				glVertex3f(0.1*i, major_tick, 0);
			} else {
				glVertex3f(0.1*i, minor_tick, 0);
			}
		}
	glEnd();
}

void draw_axes() {
	glColor3f(1, 0, 0); //Draw red x axis
	glPushMatrix();
		glTranslatef(-1, -1, -1);
		glScalef(2, 2, 2);
		draw_axis();
	glPopMatrix();

	glColor3f(0, 1, 0); //Draw green y axis
	glPushMatrix();
		glTranslatef(-1, -1, -1);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(2, -2, 2);
		draw_axis();
	glPopMatrix();

	glColor3f(0, 0, 1); //Draw blue z axis
	glPushMatrix();
		glTranslatef(-1, -1, -1);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(2, 2, -2);
		draw_axis();
	glPopMatrix();
}

void draw_contours(vec4d contour_set) {
	//float colours[3] = {0};
	float height;
	for (uint level = 0; level < contour_set.size(); level++) {
		height = ((float)level)/contour_set.size();
		//set_colour(height, colours);
		//glColor3f(colours[0], colours[1], colours[2]);
		for (uint contour = 0; contour < contour_set[level].size(); contour++) {
			glBegin(GL_LINE_STRIP);
				for (uint point = 0; point < contour_set[level][contour].size(); point++) {
					glVertex3f(contour_set[level][contour][point][0], -1.0f + height*2, contour_set[level][contour][point][1]);
				}
			glEnd();
		}
	}
}

//Render a frame
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();
	if (ortho) {
		glOrtho(-1.1, 1.4, -1.1, 1.4, 1, 100);
	} else {
		gluPerspective(45, aspect_ratio, 1, 100);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//glLightfv(GL_LIGHT0, GL_POSITION, lgt_pos);

	float colours[3] = {0};
	float colour_val = 0;
	glPushMatrix();
		if (x_flip) {
			glScalef(-1.0f, 1.0f, 1.0f);
		}
		draw_axes();
		if (draw_index == -1) {
			for (uint i = 0; i < contour_sets.size(); i++) {
				colour_val = ((float)i)/(contour_sets.size() - 1);
				set_colour(colour_val, colours);
				glColor3f(colours[0], colours[1], colours[2]);
				draw_contours(contour_sets[i]);
			}
		} else {
			uint i = (uint)draw_index;
			colour_val = ((float)i)/(contour_sets.size() - 1);
			set_colour(colour_val, colours);
			glColor3f(colours[0], colours[1], colours[2]);
			draw_contours(contour_sets[i]);
		}
	glPopMatrix();

	glutSwapBuffers();
}

void tick(int val) {
	cam_pos_radius -= horiz_dir*0.05f*speed;
	if (cam_pos_radius < 0.001) {
		cam_pos_radius = 0.001;
	}
	cam_pos[1] += vert_dir*0.05f*speed;
	cam_pos_angle += rotate_dir*0.8f*speed;
	//if (rotate_dir != 0 || horiz_dir != 0) {
		cam_pos[0] = cam_pos_radius*cos(cam_pos_angle*deg_to_rad);
		cam_pos[2] = cam_pos_radius*sin(cam_pos_angle*deg_to_rad);
	//}
	
	glutTimerFunc(30, tick, 0);
	glutPostRedisplay();
}

//OpenGL initialization
void initialize() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/*float black[4] = {0, 0, 0, 1};
	float darkgrey[4] = {0.1, 0.1, 0.1, 1.0};
	float grey[4] = {0.3, 0.3, 0.3, 0.3};
	float lightgrey[4] = {0.6, 0.6, 0.6, 0.6};
	float white[4] = {1.0, 1.0, 1.0, 1.0};

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, darkgrey);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	glEnable(GL_NORMALIZE);*/
	glEnable(GL_DEPTH_TEST);

	glLineWidth(line_width);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, aspect_ratio, 1, 1000);
	glutTimerFunc(25, tick, 0);
}

//Handle movement inputs
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 'w':
			horiz_dir = 1;
			x_flip = 0;
			ortho = 0;
			break;
		case 's':
			horiz_dir = -1;
			x_flip = 0;
			ortho = 0;
			break;
		case 'a':
			rotate_dir = 1;
			x_flip = 0;
			ortho = 0;
			break;
		case 'd':
			rotate_dir = -1;
			x_flip = 0;
			ortho = 0;
			break;
		case 'q':
			vert_dir = 1;
			x_flip = 0;
			ortho = 0;
			break;
		case 'e':
			vert_dir = -1;
			x_flip = 0;
			ortho = 0;
			break;
		case 'x':
			draw_index++;
			if ((uint)draw_index >= contour_sets.size()) {
				draw_index = -1;
			}
		case 'z':
			speed = 5;
			break;
		case 'o':
			ortho = 1;
			break;
		case 'p':
			ortho = 0;
			break;

		case '1':
			cam_pos_radius = 3.5f;
			cam_pos_angle = -90.0f;
			cam_pos[1] = 0.0f;
			if (horiz_dir == 0 && rotate_dir == 0 && vert_dir == 0) {
				x_flip = 1;
				ortho = 1;
			}
			break;
		case '2':
			cam_pos_radius = 3.5f;
			cam_pos_angle = 180.0f;
			cam_pos[1] = 0.0f;
			x_flip = 0;
			if (horiz_dir == 0 && rotate_dir == 0 && vert_dir == 0) {
				ortho = 1;
			}
			ortho = 1;
			break;
		case '3':
			cam_pos_radius = 0.0f;
			cam_pos_angle = 90.0f;
			cam_pos[1] = -3.5f;
			x_flip = 0;
			if (horiz_dir == 0 && rotate_dir == 0 && vert_dir == 0) {
				ortho = 1;
			}
			break;
		case '4':
			line_width--;
			if (line_width == 0) {
				line_width = 1;
			}
			glLineWidth(line_width);
			break;
		case '5':
			line_width++;
			glLineWidth(line_width);
			break;
	};
}

//Handle movement inputs
void keyboard_up(unsigned char key, int x, int y) {
	switch(key) {
		case 'w':
			if (horiz_dir == 1) {
				horiz_dir = 0;
			}
			break;
		case 's':
			if (horiz_dir == -1) {
				horiz_dir = 0;
			};
			break;
		case 'a':
			if (rotate_dir == 1) {
				rotate_dir = 0;
			}
			break;
		case 'd':
			if (rotate_dir == -1) {
				rotate_dir = 0;
			}
			break;
		case 'q':
			if (vert_dir == 1) {
				vert_dir = 0;
			}
			break;
		case 'e':
			if (vert_dir == -1) {
				vert_dir = 0;
			}
			break;
		case 'z':
			speed = 1;
			break;
	};
}

//Keep aspect ratio consistent with window size
void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	aspect_ratio = ((float)width)/height;
}

int main(int argc, char** argv) {
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	if (!load_contours("contours.bin")) {
		return 1; //exit if load fails
	}

	//OpenGL initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB/* | GLUT_DEPTH*/);
	glutInitWindowSize(900, 900);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Distribution Visualiser");
	initialize();

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutMainLoop();
	return 0;
}