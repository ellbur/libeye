
#include "libeye.hpp"

#include "matrix.h"
#include <cmath>
#include <iostream>

using namespace std;

namespace libeye {

// ------------------------------------------------------
// Points

point3 cross(const point3 &p1, const point3 &p2) {
	return point3(
		p1.y() * p2.z() - p1.z() * p2.y(),
		p1.z() * p2.x() - p1.x() * p2.z(),
		p1.x() * p2.y() - p1.y() * p2.x() );
}

// ------------------------------------------------------
// Screen

Screen::Screen() {
	origin = point3(0, 0, 0);
	e1     = point3(1, 0, 0);
	e2     = point3(0, 1, 0);
}

Screen::Screen(const point3 &_origin, const point3 &_e1, const point3 &_e2) {
	this->origin = _origin;
	this->e1     = _e1;
	this->e2     = _e2;
}

point3 Screen::to_real(const point2 &image) const {
	return origin + image.x() * e1 + image.y() * e2;
}

point2 Screen::project(const point3 &eye, const point3 &p) const {
	double mat[] = {
		e1.x(), e2.x(), p.x() - eye.x(), p.x() - origin.x(),
		e1.y(), e2.y(), p.y() - eye.y(), p.y() - origin.y(),
		e1.z(), e2.z(), p.z() - eye.z(), p.z() - origin.z(),
	};
	double sol[3];
	
	gr_solve(sol, mat, 3);
	
	return point2(sol[0], sol[1]);
}

point3 Screen::project_back(const Screen &remote,
	const point3 &eye, const point2 &im)
{
	point3 real   = to_real(im);
	point2 far_im = remote.project(eye, real);
	
	return remote.to_real(far_im);
}

Screen Screen::three_points(const point3 &p1,
	const point3 &p2, const point3 &p3)
{
	return Screen(
		p1,
		p2 - p1,
		p3 - p1 );
}

Screen Screen::line_normal(const point3 &p1,
	const point3 &p2, const point3 &normal)
{
	return Screen(
		p1,
		p2 - p1,
		cross((p2 - p1), normal) );
}

// -----------------------------------------------------------

View::View(size_t _width, size_t _height,
	const Screen &_screen, const point3 &_eye)
{
	this->width  = _width;
	this->height = _height;
	this->eye    = _eye;
	this->screen = _screen;
	
	buffer = new double[width * height];
	miny   = new int[width];
	maxy   = new int[width];
}

View::~View() {
	delete[] buffer;
	delete[] miny;
	delete[] maxy;
}

double View::get(int x, int y) const {
	if (x < 0 || width  <= x) return 0.0;
	if (y < 0 || height <= y) return 0.0;
	
	return buffer[x + y*width];
}

double View::get(const point2 &p) const {
	return get( (int) p.x(), (int) p.y() );
}

void View::draw(int x, int y, double depth) {
	if (x < 0 || width  <= x) return;
	if (y < 0 || height <= y) return;
	
	if (buffer[x + y*width] < depth) return;
	
	buffer[x + y*width] = depth;
}

void View::draw(const point2 &p, double depth) {
	draw((int) p.x(), (int) p.y(), depth);
}

void View::set(int x, int y, double depth) {
	if (x < 0 || width  <= x) return;
	if (y < 0 || height <= y) return;
	
	buffer[x + y*width] = depth;
}

void View::flatten(double offset) {
	int x, y;
	
	point3 normal = cross(screen.e1, screen.e2);
	normal = normal * (1 / norm(normal));
	
	point2 im;
	point3 real;
	
	double closest = dot(normal, screen.origin - eye);
	closest = fabs(closest);
	
	for (x=0; x<width; x++)
	for (y=0; y<height; y++) {
		im.x() = x;
		im.y() = y;
		
		real = screen.to_real(im);
		
		double hypot = dist(real, eye);
		double depth = hypot / closest * (closest + offset);
		
		buffer[x + y*width] = depth;
	}
}

void View::draw_point(const point3 &p) {
	point2 image = screen.project(eye, p);
	
	int x = (int) image.x();
	int y = (int) image.y();
	
	draw(x, y, dist(p, eye));
}

void View::draw_line(const point3 &p1, const point3 &p2) {
	int i;
	point2 scan;
	
	Screen remote = Screen::line_normal(p1, p2, p1 - eye);
	
	point2 im1 = screen.project(eye, p1);
	point2 im2 = screen.project(eye, p2);
	
	int len = (int) dist(im1, im2);
	
	for (i=0; i<=len; i++) {
		double t = (double) i / len;
		
		scan = t*im1 + (1-t)*im2;
		
		double depth = dist(eye,
			screen.project_back(remote, eye, scan));
		
		draw((int) scan.x(), (int) scan.y(), depth);
	}
}

void View::draw_triangle(const point3 &p1,
	const point3 &p2, const point3 &p3)
{
	Screen remote = Screen::three_points(p1, p2, p3);
	
	point2 im1 = screen.project(eye, p1);
	point2 im2 = screen.project(eye, p2);
	point2 im3 = screen.project(eye, p3);
	
	minx = width;
	maxx = 0;
	
	if ((int) im1.x() < minx) minx = (int) im1.x();
	if ((int) im2.x() < minx) minx = (int) im2.x();
	if ((int) im3.x() < minx) minx = (int) im3.x();
	
	if ((int) im1.x() > maxx) maxx = (int) im1.x();
	if ((int) im2.x() > maxx) maxx = (int) im2.x();
	if ((int) im3.x() > maxx) maxx = (int) im3.x();
	
	if (minx < 0)      minx = 0;
	if (maxx >= width) maxx = width - 1;
	
	start_fill();
	add_line(im1, im2);
	add_line(im2, im3);
	add_line(im3, im1);
	end_fill(remote);
}

void View::draw_pgram(const point3 &p,
	const point3 e1, const point3 e2)
{
	draw_triangle(p, p+e1, p+e2);
	draw_triangle(p+e1+e2, p+e1, p+e2);
}

point2 View::stereo_pair(const point3 &eye2, const point2 &p) {
	point3 real  = screen.to_real(p);
	double depth = get(p);
	
	point3 leg = real - eye;
	point3 far = eye + leg * (depth / norm(leg));
	
	return screen.project(eye2, far);
}

void View::start_fill() {
	int x;
	
	for (x=minx; x<=maxx; x++) {
		miny[x] = width;
		maxy[x] = 0;
	}
}

void View::add_line(const point2 &im1, const point2 &im2) {
	point2 pleft, pright;
	int xleft, xright;
	int x, y;
	double slope;
	
	// it won't contribute to the fill so stop
	if ((int) (im1.x() - im2.x()) == 0) return;
	
	if (im1.x() < im2.x()) {
		pleft  = im1;
		pright = im2;
	}
	else {
		pleft  = im2;
		pright = im1;
	}
	
	xleft  = (int) pleft.x();
	xright = (int) pright.x();
	
	if (xleft  < 0)      xleft  = 0;
	if (xright >= width) xright = width;
	
	slope = (im2.y() - im1.y()) / (im2.x() - im1.x());
	
	for (x=xleft; x<=xright; x++) {
		y = (int) (pleft.y() + (x - pleft.x()) * slope);
		
		if (y < 0)       y = 0;
		if (y >= height) y = height;
		
		if (y < miny[x]) miny[x] = y;
		if (y > maxy[x]) maxy[x] = y;
	}
}

void View::end_fill(const Screen &remote) {
	int x, y;
	
	for (x=minx; x<=maxx; x++)
	for (y=miny[x]; y<=maxy[x]; y++) {
		
		point3 back = screen.project_back(
			remote, eye, point2(x,y));
		
		double depth = dist(eye, back);
		
		draw(x, y, depth);
	}
}

// -------------------------------------------------

} /* namespace libeye */
