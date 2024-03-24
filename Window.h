#ifndef PARTICLES_WINDOW_H
#define PARTICLES_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QPainter>
#include "Point.h"
#include <vector>
#include <QKeyEvent>
#include <thread>

using namespace std;

class Window: public QMainWindow {
    Q_OBJECT
public:
    Window(QWidget *parent = 0, const char *name = 0):QMainWindow(parent), center(1100, 800) {
        Point rot(0,0,0.00036);
        const float rk = 0.4;
        for(int i=0; i<N; i++) {

//            if(i<N*0.65)
                MPS.push_back(Point::rnd(0, 650).mult(rk));
//            else if(i<N*0.82)
//                MPS.push_back(Point::rnd(200, 380).mult(rk));
//            else
//                MPS.push_back(Point::rnd(400, 500).mult(rk));

            MPS.back().v = MPS.back().x ;
//                    + MPS.back().x.norm().mult(5);
            MPS.back().v.setMult(rot);
        }

        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(draw()));
        timer->setInterval(40);
        timer->start();
    }
protected:
    void paintEvent(QPaintEvent *e) {
        QPainter* painter=new QPainter(this);

        for(int i = 0; i<30; i++) {
            // speeds
            for(int i=0; i<N; i++)
                for(int j=i+1; j<N; j++) {
                    // phys

                    Point r = MPS[j].x.sub(MPS[i].x);
                    float rr = r.l2();
                    if (rr > rigid_dist2) {
                        Point dir = r.norm();
                        float krr = 0.0025 / (rr);
                        MPS[i].v.setAdd(dir.mult(krr));
                        MPS[j].v.setAdd(dir.mult(-krr));
                        if(rr > rigid_dist2_lim) {
                            auto el = pairs.find(make_pair(i,j));
                            if(el != pairs.end())
                                pairs.erase(el);
                        }
                    } else {
                        pairs[make_pair(i, j)] = true;
                    }
                }


            // pairs vs
            for(auto& p: pairs) {
                int i = p.first.first;
                int j = p.first.second;
                auto tmp = MPS[i].x.sub(MPS[j].x);
                if(tmp.l2()>0.000000001) {
                    tmp.setNorm();
                    Point V1n = MPS[i].v.getComponent(tmp);
                    Point V2n = MPS[j].v.getComponent(tmp);

                    MPS[i].v.setSub(V1n.mult(1));
                    MPS[j].v.setSub(V2n.mult(1));

                    Point Vn = V1n.mix(V2n);
                    MPS[i].v.setAdd(Vn);
                    MPS[j].v.setAdd(Vn);
                }
            }

            // coords
            for(auto& x:MPS) {
                x.x.setAdd(x.v);
            }

//            // pairs coords
            for(auto& p: pairs) {
                int i = p.first.first;
                int j = p.first.second;

                // push away
                auto tmp = MPS[i].x.sub(MPS[j].x);
                if(tmp.l2()<0.000001) continue;

                tmp.setNorm().setMult(0.00028);
                MPS[i].v.setAdd(tmp);
                MPS[j].v.setSub(tmp);

// rigid
//                auto mix = MPS[i].x.mix(MPS[j].x);
//                Point x1 = MPS[i].x.sub(mix);
//                Point x2 = MPS[j].x.sub(mix);
//                if (x1.l2() > 0.00001) {
//                    x1.setNorm().setMult(rigid_dist / 2.);
//                    MPS[i].x = (mix.add(x1));
//
//                    x2.setNorm().setMult(rigid_dist / 2.);
//                    MPS[j].x = (mix.add(x2));
//                }
            }
        }

        QPen pen(Qt::black);
        pen.setWidth(11);
        pen.setCapStyle(Qt::PenCapStyle::RoundCap);
        pen.setColor(QColor(150,0,0, 90));
        painter->setPen(pen);


        for(auto& mp: MPS)
            painter->drawPoint(center+QPoint(mp.x.x*scale, mp.x.y*scale));

        delete painter;
    }
public slots:
    void keyPressEvent(QKeyEvent *event) {
        const int stride = 30;
        switch(event->key())
        {
            case Qt::Key_Right:
                center.setX(center.x() - stride);
                break;
            case Qt::Key_Left:
                center.setX(center.x() + stride);
                break;
            case Qt::Key_Up:
                center.setY(center.y() + stride);
                break;
            case Qt::Key_Down:
                center.setY(center.y() - stride);
                break;
            case Qt::Key_Z:
                scale+=0.04;
                break;
            case Qt::Key_X:
                scale-=0.04;
                break;
        }
    };
    void draw() {
        update();
    }
private:
    vector<MatPoint> MPS;
    map<pair<int, int>, bool> pairs;
    QPoint center;
    const float rigid_dist = 5;
    const float rigid_dist2 = rigid_dist * rigid_dist;
    const float rigid_dist_lim_k = 1.3;
    const float rigid_dist2_lim =  rigid_dist * rigid_dist * rigid_dist_lim_k * rigid_dist_lim_k;
    const int N = 580;
    float scale = 1.4;
};

#endif //PARTICLES_WINDOW_H
