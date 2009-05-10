
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
	const point3 &eye, const point2 &im) const
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

View::View(size_t _width, size_t _height) {
	this->width  = _width;
	this->height = _height;
	
	buffer = new double[width * height];
	miny   = new int[width];
	maxy   = new int[width];
}

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

point2 View::stereo_pair(const point3 &eye2, const point2 &p) const {
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
	if (xright >= width) xright = width - 1;
	
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

// ------------------------------------------------------
// BiView

BiView::BiView(int _width, int _height, double _eye_back,
		double _eye_sep, double _dpi) :
	left(_width, _height),
	right(_width, _height)
{
	this->width    = _width;
	this->height   = _height;
	this->eye_back = _eye_back;
	this->eye_sep  = _eye_sep;
	this->dpi      = _dpi;
	
	double scale = 1 / dpi;
	
	screen_width  = width  * scale;
	screen_height = height * scale;
	
	double screen_width  = width  * scale;
	double screen_height = height * scale;
	
	point3 eye1(-eye_sep/2, 0, -eye_back);
	point3 eye2(+eye_sep/2, 0, -eye_back);
	
	point3 origin(-screen_width/2, screen_height/2, 0);
	point3 e1(scale, 0, 0);
	point3 e2(0, -scale, 0);
	
	Screen screen(origin, e1, e2);
	
	left.screen  = screen;
	left.eye     = eye1;
	
	right.screen = screen;
	right.eye    = eye2;
}

double BiView::half_width(double depth) {
	return eye_sep/2 +
		(eye_back+depth)/eye_back * (eye_sep/2 + screen_width/2);
}

double BiView::half_height(double depth) {
	return (eye_back + depth) / (eye_back) * screen_height / 2;
}

void BiView::flatten(double depth) {
	left.flatten(depth);
	right.flatten(depth);
}

void BiView::draw_point(const point3 &p) {
	left.draw_point(p);
	right.draw_point(p);
}

void BiView::draw_line(const point3 &p1, const point3 &p2) {
	left.draw_line(p1, p2);
	right.draw_line(p1, p2);
}

void BiView::draw_triangle(const point3 &p1,
	const point3 &p2, const point3 &p3)
{
	left.draw_triangle(p1, p2, p3);
	right.draw_triangle(p1, p2, p3);
}

void BiView::draw_pgram(const point3 &p,
	const point3 e1, const point3 e2)
{
	left.draw_pgram(p, e1, e2);
	right.draw_pgram(p, e1, e2);
}

point2 BiView::left_pair(const point2 &im) {
	return right.stereo_pair(left.eye, im);
}

point2 BiView::right_pair(const point2 &im) {
	return left.stereo_pair(right.eye, im);
}

// --------------------------------------------

StereoBlank::StereoBlank(int _width, int _height) {
	this->width  = _width;
	this->height = _height;
	
	left_pair_buffer  = new int[width * height];
	right_pair_buffer = new int[width * height];
}

StereoBlank::StereoBlank(const BiView &biview) {
	this->width  = biview.right.width;
	this->height = biview.right.height;
	
	left_pair_buffer  = new int[width * height];
	right_pair_buffer = new int[width * height];
	
	set_left(biview.left, biview.right.eye);
	set_right(biview.right, biview.left.eye);
}

StereoBlank::StereoBlank(const View &right, const point3 &eye) {
	this->width  = right.width;
	this->height = right.height;
	
	left_pair_buffer  = new int[width * height];
	right_pair_buffer = new int[width * height];
	
	set_right(right, eye);
}

StereoBlank::~StereoBlank() {
	delete[] left_pair_buffer;
	delete[] right_pair_buffer;
}

void StereoBlank::set_left(const View &left, const point3 &eye) {
	int x, y;
	
	for (x=0; x<width; x++)
	for (y=0; y<height; y++) {
		
		point2 p(x, y);
		p = left.stereo_pair(eye, p);
		
		int pair_x = (int) p.x();
		
		right_pair_buffer[x + y*width] = pair_x;
	}
}

void StereoBlank::set_right(const View &right, const point3 &eye) {
	int x, y;
	
	for (x=0; x<width; x++)
	for (y=0; y<height; y++) {
		
		point2 p(x, y);
		p = right.stereo_pair(eye, p);
		
		int pair_x = (int) p.x();
		
		left_pair_buffer[x + y*width] = pair_x;
	}
}

int StereoBlank::get_left(int x, int y) const {
	if (x < 0 || x >= width)  return -1;
	if (y < 0 || y >= height) return -1;
	
	int pair_x = left_pair_buffer[x + y*width];
	
	if (pair_x < 0 || pair_x >= width)
		return -1;
	
	int check_x = right_pair_buffer[pair_x + y*width];
	
	if (std::abs((double)(check_x - x)) > 2)
		return -1;
	
	return pair_x;
}

int StereoBlank::get_right(int x, int y) const {
	if (x < 0 || x >= width)  return -1;
	if (y < 0 || y >= height) return -1;
	
	int pair_x = right_pair_buffer[x + y*width];
	
	if (pair_x < 0 || pair_x >= width)
		return -1;
	
	int check_x = left_pair_buffer[pair_x + y*width];
	
	if (std::abs((double)(check_x - x)) > 2)
		return -1;
	
	return pair_x;
}

int StereoBlank::force_left(int x, int y) const {
	if (x < 0 || x >= width)  return -1;
	if (y < 0 || y >= height) return -1;
	
	int pair_x = left_pair_buffer[x + y*width];
	
	if (pair_x < 0 || pair_x >= width)
		return -1;
	
	return pair_x;
}

int StereoBlank::force_right(int x, int y) const {
	if (x < 0 || x >= width)  return -1;
	if (y < 0 || y >= height) return -1;
	
	int pair_x = right_pair_buffer[x + y*width];
	
	if (pair_x < 0 || pair_x >= width)
		return -1;
	
	return pair_x;
}

void StereoBlank::isometric_grid(int &rows, int &cols,
	int *&xvec, int &vgap, int rep) const
{
	int row, col;
	int i;
	
	int background_gap = force_right(0, 0);
	int gap = background_gap / rep;
	vgap = (int) (gap * 0.28868);
	
	rows = height / vgap;
	cols = width / gap;
	
	xvec = new int[rows * cols];
	
	int *ring = new int[rep];
	
	for (row=0; row<rows; row++) {
		int offset = (row%2 == 0 ? 0 : gap/2);
		for (i=0; i<rep; i++) {
			ring[i] = offset + gap*i;
		}
		
		int y = row * vgap;
		
		for (col=0; col<cols; col++) {
			int x = ring[col%rep];
			xvec[col + row*cols] = x;
			
			int next = force_right(x, y);
			if (next < 0) next = x + gap;
			
			ring[col%rep] = next;
		}
	}
	
	delete[] ring;
}

// ------------------------------------------------------

} /* namespace libeye */
