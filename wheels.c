#include <stdint.h>
#include <math.h>
#include "graph.h"
#include "wheels.h"

// Вращение точки, для отрисовки колес:
static void point_rot(int *x, int *y, int angle) {
	int xNew, yNew;
	float A = (float)angle / (180 / 3.14);
	xNew = (int)(*x * cos(A) + *y * sin(A));
	yNew = (int)(*y * cos(A) - *x * sin(A));
	*x = xNew;
	*y = yNew;
}

// Отрисовка прямоугольника с поворотом,
// используется для отрисовки колес:
void draw_rect(int x, int y, int angle) {
	int point_0_x = -RECT_X_SIZE;
	int point_0_y = -RECT_Y_SIZE;
	int point_1_x = -RECT_X_SIZE;
	int point_1_y =  RECT_Y_SIZE;
	int point_2_x =  RECT_X_SIZE;
	int point_2_y =  RECT_Y_SIZE;
	int point_3_x =  RECT_X_SIZE;
	int point_3_y = -RECT_Y_SIZE;

	point_rot(&point_0_x, &point_0_y, angle);
	point_rot(&point_1_x, &point_1_y, angle);
	point_rot(&point_2_x, &point_2_y, angle);
	point_rot(&point_3_x, &point_3_y, angle);
	
	point_0_x += x;	point_0_y += y;
	point_1_x += x;	point_1_y += y;
	point_2_x += x;	point_2_y += y;
	point_3_x += x;	point_3_y += y;
	
	graph_line(point_0_x,point_0_y,point_1_x,point_1_y);
	graph_line(point_1_x,point_1_y,point_2_x,point_2_y);
	graph_line(point_2_x,point_2_y,point_3_x,point_3_y);
	graph_line(point_3_x,point_3_y,point_0_x,point_0_y);
}
