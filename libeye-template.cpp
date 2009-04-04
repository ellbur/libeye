
#include <cmath>

// Implementation of template functions.
//
// Must be included in libeye.hpp

namespace libeye {

// -------------------------------------------------------

template<int N>
point<N>::point() {
	// nada
}

template<int N>
point<N>::point(double x, double y, double z) {
	coords[0] = x;
	coords[1] = y;
	coords[2] = z;
}

template<int N>
point<N>::point(double x, double y) {
	coords[0] = x;
	coords[1] = y;
}

template<int N>
double& point<N>::x() { return coords[0]; }

template<int N>
double& point<N>::y() { return coords[1]; }

template<int N>
double& point<N>::z() { return coords[2]; }

template<int N>
double point<N>::x() const { return coords[0]; }

template<int N>
double point<N>::y() const { return coords[1]; }

template<int N>
double point<N>::z() const { return coords[2]; }

// -------------------------------------------------------

template<int N>
std::ostream& operator<<(std::ostream &s, const point<N> &p)
{
	s << "(";
	
	for (int i=0; i<N; i++) {
		s << p[i];
		
		if (i < N-1) s << ", ";
	}
	
	s << ")";
	
	return s;
}

template<int N>
inline point<N> operator-(const point<N> &p)
{
	point<N> r;
	int i;
	
	for (i=0; i<N; i++) {
		r[i] = -p[i];
	}
	
	return r;
}

template<int N>
inline point<N> operator-(const point<N> &p1, const point<N> &p2)
{
	point<N> r;
	
	for (int i=0; i<N; i++) {
		r[i] = p1[i] - p2[i];
	}
	
	return r;
}

template<int N>
inline point<N> operator+(const point<N> &p1, const point<N> &p2)
{
	point<N> r;
	
	for (int i=0; i<N; i++) {
		r[i] = p1[i] + p2[i];
	}
	
	return r;
}

template<int N>
inline point<N> operator*(const point<N> &p, double k)
{
	point<N> r;
	
	for (int i=0; i<N; i++) {
		r[i] = p[i] * k;
	}
	
	return r;
}

template<int N>
inline point<N> operator*(double k, const point<N> &p)
{
	return p * k;
}

template<int N> double norm(const point<N> &p)
{
	double sum = 0;
	int i;
	
	for (i=0; i<N; i++) {
		sum += p[i] * p[i];
	}
	
	return sqrt(sum);
}

template<int N> double dist(const point<N> &p1, const point<N> &p2)
{
	return norm(p1 - p2);
}

template<int N> double dot(const point<N> &p1, const point<N> &p2)
{
	double sum = 0;
	int i;
	
	for (i=0; i<N; i++) {
		sum += p1[i] * p2[i];
	}
	
	return sum;
}

} /* namespace libeye */
