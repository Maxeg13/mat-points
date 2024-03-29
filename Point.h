#ifndef POINT_H
#define POINT_H
#include <math.h>

class Point
{
public:
    Point(float x, float y, float z):x(x), y(y), z(z) {
    };

    Point(float x, float y):x(x), y(y), z(0) {
    };

    static Point rnd(float a, float b) {
        float phi = rand()*0.005;
        float r = rand()%1000*0.001;
        return Point(cos(phi)*(a+r*(b-a)), sin(phi)*(a+r*(b-a)));
    }

    Point() = default;

    Point(const Point&) = default;

    friend Point operator+(const Point& p1, const Point& p2) {
        return {p1.x+p2.x, p1.y+p2.y};
    }

    friend Point operator-(const Point& p1, const Point& p2) {
        return {p1.x - p2.x, p1.y - p2.y};
    }

    float l2() {
        return x*x+y*y;
    }

    Point mix(const Point& p) {
        return ((*this) + p).mult(0.5);
    }

    Point getComponent(const Point& n) {
        return n.mult(multScal(n));
    }

    Point norm() {
        float l = sqrt(l2());
        Point y(*this);
        y.setMult(1/l);
        return y;
    }

    Point& setNorm() {
        float l = sqrt(l2());
        setMult(1/l);
        return *this;
    }

    Point& setRev() {
        x = -x;
        y = -y;
        return *this;
    }

    Point& setAdd(const Point& p) {
        x+=p.x;
        y+=p.y;
        return *this;
    }

    Point& setSub(const Point& p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }

    Point& setMult(float _x) {
        x*=_x;
        y*=_x;
        return *this;
    }

    float multScal(const Point& p) {
        return x*p.x+y*p.y;
    }

    Point mult(const Point& p) {
        return {y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x};
    }

    Point& setMult(const Point& p) {
        Point p1;
        p1.x = y*p.z - z*p.y;
        p1.y = z*p.x - x*p.z;
        p1.z = x*p.y - y*p.x;
        *this = p1;
        return *this;
    }

    Point mult(float _x) const {
        Point _p(*this);
        _p.x*=_x;
        _p.y*=_x;
        return _p;
    }

    float x;
    float y;
    float z;
};


class MatPoint {
public:
    MatPoint(float x, float y, float z):x(x,y,z) {
    }

    MatPoint(Point&& p):x(p) {}

    void setV(Point& _v) {
        v = _v;
    }

    Point x;
    Point v;
};

#endif // POINT_H