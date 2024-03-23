#ifndef PARTICLES_WINDOW_H
#define PARTICLES_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QPainter>
#include "Point.h"
#include <vector>

using namespace std;

class Window: public QMainWindow {
    Q_OBJECT
public:
    Window(QWidget *parent = 0, const char *name = 0):QMainWindow(parent) {
        Point z(0,0,0.0012);
        for(int i=0; i<N; i++) {
            const int s = 250;
            float phi = rand()*0.001;
            float r = rand()%100*0.01;
            MPS.emplace_back(sin(phi)*s*r,cos(phi)*s*r,0);

            MPS.back().v = MPS.back();
            MPS.back().v.setMult(z);
        }

        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(draw()));
        timer->setInterval(40);
        timer->start();
    }
protected:
    void paintEvent(QPaintEvent *e) {
        QPainter* painter=new QPainter(this);

        for(int i = 0; i<40; i++) {
            // speeds
            for(int i=0; i<N; i++)
                for(int j=i+1; j<N; j++) {
                    // phys
                    Point r = MPS[j].sub(MPS[i]);
                    float rr = r.multScal(r);
                    if(rr>0.01) {
                        Point dir = r.norm();
                        float krr = 0.0001/sqrt(rr);
                        MPS[i].v.setAdd(dir.mult(krr));
                        MPS[j].v.setAdd(dir.mult(-krr));
                    } else {
                        auto tmp = MPS[i].v.sub(MPS[j].v);
                        if(tmp.l2()>0.0000001) {
                            tmp.setNorm();
                            Point V1n = MPS[i].v.getComponent(tmp);
                            Point V2n = MPS[j].v.getComponent(tmp);

                            MPS[i].v.setSub(V1n);
                            MPS[j].v.setSub(V2n);

                            Point Vn = V1n.mix(V2n);
                            MPS[i].v.setAdd(Vn);
                            MPS[j].v.setAdd(Vn);
                        }

                        {
                            auto tmp = MPS[i].sub(MPS[j]);
                            tmp.setMult(0.5);
                            MPS[j].setAdd(tmp);
                            MPS[i].setSub(tmp);
                        }
                    }
                }

            // coords
            for(auto& x:MPS) {
                x.setAdd(x.v);
            }
        }

        QPen pen(Qt::black);
        pen.setWidth(4);
        pen.setColor(QColor(150,0,0));
        painter->setPen(pen);
        QPoint p(1100, 800);

        for(auto& v: MPS)
            painter->drawPoint(p+QPoint(v.x, v.y));

        delete painter;
    }
public slots:
    void draw() {
        update();
    }
private:
    vector<MatPoint> MPS;
    map<pair<int, int>, bool> pairs;
    const int N = 550;
};

#endif //PARTICLES_WINDOW_H
