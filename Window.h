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

using namespace std;

class Window: public QMainWindow {
    Q_OBJECT
public:
    Window(QWidget *parent = 0, const char *name = 0):QMainWindow(parent), center(1100, 800) {
        Point rot(0,0,0.0011);
        for(int i=0; i<N; i++) {
            const int s = 330;
            float phi = rand()*0.001;
            float r = rand()%1000*0.001;
            MPS.emplace_back(sin(phi)*s*r,cos(phi)*s*r,0);

            MPS.back().v = MPS.back().x;
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

        for(int i = 0; i<40; i++) {
            // speeds
            for(int i=0; i<N; i++)
                for(int j=i+1; j<N; j++) {
                    // phys
                    if(pairs.find(make_pair(i,j)) == pairs.end()) {
                        Point r = MPS[j].x.sub(MPS[i].x);
                        float rr = r.l2();
                        if (rr > rigid_dist2) {
                            Point dir = r.norm();
                            float krr = 0.0001 / sqrt(rr);
                            MPS[i].v.setAdd(dir.mult(krr));
                            MPS[j].v.setAdd(dir.mult(-krr));
                        } else {
                            pairs[make_pair(i, j)] = true;
                        }
                    }
                }


            // pairs vs
            for(auto& p: pairs) {
                int i = p.first.first;
                int j = p.first.second;
                auto tmp = MPS[i].v.sub(MPS[j].v);
                if(tmp.l2()>0.000000001) {
                    tmp.setNorm();
                    Point V1n = MPS[i].v.getComponent(tmp);
                    Point V2n = MPS[j].v.getComponent(tmp);

                    MPS[i].v.setSub(V1n);
                    MPS[j].v.setSub(V2n);

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

                auto tmp = MPS[i].x.sub(MPS[j].x);

                if(tmp.l2()<0.000001) continue;

                tmp.setNorm().setMult(0.0003);
                MPS[i].v.setAdd(tmp);
                MPS[j].v.setSub(tmp);

//                auto mix = MPS[i].x.mix(MPS[j].x);
//                Point x1 = MPS[i].x.sub(mix);
//                Point x2 = MPS[j].x.sub(mix);
//                if (x1.l2() > 0.00001) {
////                    qDebug()<<"h";
//                    x1.setNorm().setMult(rigid_dist / 2.);
//                    MPS[i].x = (mix.add(x1));
//
//                    x2.setNorm().setMult(rigid_dist / 2.);
//                    MPS[j].x = (mix.add(x2));
//                }
            }
        }

        QPen pen(Qt::black);
        pen.setWidth(6);
        pen.setColor(QColor(150,0,0));
        painter->setPen(pen);


        for(auto& mp: MPS)
            painter->drawPoint(center+QPoint(mp.x.x, mp.x.y));

        delete painter;
    }
public slots:
    void keyPressEvent(QKeyEvent *event) {
        switch(event->key())
        {
            case Qt::Key_Right:
                center.setX(center.x() - 3);
                break;
            case Qt::Key_Left:
                center.setX(center.x() + 3);
                break;
            case Qt::Key_Up:
                center.setY(center.y() + 3);
                break;
            case Qt::Key_Down:
                center.setY(center.y() - 3);
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
    const float rigid_dist = 3;
    const float rigid_dist2 = rigid_dist * rigid_dist;
    const int N = 600;
};

#endif //PARTICLES_WINDOW_H
