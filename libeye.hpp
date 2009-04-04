
#ifndef _libeye_hpp
#define _libeye_hpp 1

#include <sys/types.h>
#include <string>
#include <ostream>

namespace libeye {

// ------------------------------------------------
// Points

template<int N>
class point {
	public:
	
	// -------------------
	// data
	
	double coords[N];
	
	// -------------------
	// methods
	
	point();
	point(double x, double y, double z);
	point(double x, double y);
	
	double  operator[](size_t i) const { return coords[i]; }
	double& operator[](size_t i)       { return coords[i]; }
	
	double& x();
	double& y();
	double& z();
	
	double x() const;
	double y() const;
	double z() const;
};

typedef point<2> point2;
typedef point<3> point3;

template<int N>
std::ostream& operator<<(std::ostream &s, const point<N> &p);

template<int N>
inline point<N> operator-(const point<N> &p);

template<int N>
inline point<N> operator-(const point<N> &p1, const point<N> &p2);

template<int N>
inline point<N> operator+(const point<N> &p1, const point<N> &p2);

template<int N>
inline point<N> operator*(const point<N> &p, double k);

template<int N>
inline point<N> operator*(double k, const point<N> &p);

template<int N> double norm(const point<N> &p);
template<int N> double dist(const point<N> &p1, const point<N> &p2);

template<int N> double dot(const point<N> &p1, const point<N> &p2);
point3 cross(const point3 &p1, const point3 &p2);

// ------------------------------------------------
// Screens

class Screen {
	public:
	
	point3 origin;
	point3 e1;
	point3 e2;
	
	Screen();
	Screen(const point3 &_origin, const point3 &_e1, const point3 &_e2);
	
	point3 to_real(const point2 &image) const;
	point2 project(const point3 &eye, const point3 &p) const;
	point3 project_back(const Screen &remote,
		const point3 &eye, const point2 &im);
	
	// --------------
	
	static Screen three_points(const point3 &p1,
		const point3 &p2, const point3 &p3);
	
	static Screen line_normal(const point3 &p1,
		const point3 &p2, const point3 &normal);
};

// --------------------------------------------

class View {
	public:
	
	point3 eye;
	Screen screen;
	
	int width;
	int height;
	
	double *buffer;
	
	View(size_t _width, size_t _height,
		const Screen &_screen, const point3 &_eye);
	~View();
	
	double get(int x, int y) const;
	double get(const point2 &p) const;
	
	// Does not overwrite if bigger
	void draw(int x, int y, double depth);
	void draw(const point2 &p, double depth);
	
	// Always overwrites
	void set(int x, int y, double depth);
	
	void flatten(double depth);
	
	void draw_point(const point3 &p);
	void draw_line(const point3 &p1, const point3 &p2);
	
	void draw_triangle(const point3 &p1,
		const point3 &p2, const point3 &p3);
	
	void draw_pgram(const point3 &p,
		const point3 e1, const point3 e2);
	
	point2 stereo_pair(const point3 &eye2, const point2 &p);
	
	private:
	
	int *miny;
	int *maxy;
	
	int minx;
	int maxx;
	
	void start_fill();
	void add_line(const point2 &im1, const point2 &im2);
	void end_fill(const Screen &remote);
};

// --------------------------------------------

} /* namespace libeye */

#include <libeye/libeye-template.cpp>

#endif /* defined _libeye_hpp */
