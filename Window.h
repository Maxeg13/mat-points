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
#include <mutex>

using namespace std;

#include <mutex>
#include <condition_variable>

class Window: public QMainWindow {
    Q_OBJECT
private:
    static constexpr int threads_num = 24;
    static constexpr int N = 2000/threads_num*threads_num;

    vector<MatPoint> MPS;
    map<pair<int, int>, bool> pairs[threads_num];

    QPoint center;
    static constexpr int thread_step = N / threads_num;
    std::thread ts[threads_num];
    int i1[threads_num];
    std::mutex mtx;

    const float rigid_dist = 3;
    const float rigid_dist2 = rigid_dist * rigid_dist;
    const float rigid_dist_lim_k = 1.3;
    const float rigid_dist2_lim =  rigid_dist * rigid_dist * rigid_dist_lim_k * rigid_dist_lim_k;
    float scale = 0.7;
public:
    Window(QWidget *parent = 0, const char *name = 0):QMainWindow(parent),
//    center(1100, 800)
    center(1100, 450)
    {
        Point rot(0,0,0.00000);
        const float rk = 0.6;
        for(int i=0; i<N; i++) {

            if(i<N*0.69) {
                MPS.push_back(Point::rnd(0, 170).mult(rk));
                rot.z = -0.00009;
            }
            else if(i<N*0.84) {
                MPS.push_back(Point::rnd(250, 440).mult(rk));
                rot.z = 0.0014;
            }
            else {
                MPS.push_back(Point::rnd(450, 650).mult(rk));
                rot.z = 0.00053;
            }

            MPS.back().v = MPS.back().x ;
            MPS.back().v.setMult(rot);
        }

        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(draw()));
        timer->setInterval(40);
        timer->start();

        for(int i = 0; i<threads_num; i++) {
            i1[i] = i * thread_step;
        }

        auto lam = [this](int idx){
            for (int i = i1[idx]; i < i1[idx]+thread_step; i++) {
                for (int j = i + 1; j < N; j++) {
                    // phys

                    Point r = MPS[j].x.sub(MPS[i].x);
                    float rr = r.l2();
                    if (rr > rigid_dist2) {
                        Point dir = r.norm();
                        float krr = 0.007 / (rr);
                        MPS[i].v.setAdd(dir.mult(krr));
                        MPS[j].v.setAdd(dir.mult(-krr));
                        if (rr > rigid_dist2_lim) {
//                            std::unique_lock<std::mutex> lock(mtx);
                            auto el = pairs[idx].find(make_pair(i, j));
                            if (el != pairs[idx].end())
                                pairs[idx].erase(el);
                        }
                    } else {
//                        std::unique_lock<std::mutex> lock(mtx);
                        pairs[idx][make_pair(i, j)] = true;
                    }
                }
            }

        };

        std::thread main_thread([this, lam]{
            while(true)
            {
                for (int i = 0; i < threads_num; i++) {
                    ts[i] = std::move(std::thread(lam, i));
                }

                for (int i = 0; i < threads_num; i++) {
                    ts[i].join();
                }

                // pairs vs
                for(int idx = 0; idx < threads_num; idx++) {
                    for (auto &p: pairs[idx]) {
                        int i = p.first.first;
                        int j = p.first.second;
                        auto tmp = MPS[i].x.sub(MPS[j].x);
                        if (tmp.l2() > 0.000000001) {
                            tmp.setNorm();
                            Point V1n = MPS[i].v.getComponent(tmp);
                            Point V2n = MPS[j].v.getComponent(tmp);

                            static const float k = 0.18;
                            Point V1a = V1n.mult(k);
                            Point V2a = V2n.mult(k);

                            MPS[i].v.setAdd(V2a);
                            MPS[j].v.setAdd(V1a);

                            Point Vn = V1a.mix(V2a);
                            MPS[i].v.setSub(Vn);
                            MPS[j].v.setSub(Vn);
                        }
                    }
                }

                //            // pairs coords
                for(int idx = 0; idx < threads_num; idx++) {
                    for (auto &p: pairs[idx]) {
                        int i = p.first.first;
                        int j = p.first.second;

                        // push away
                        auto tmp = MPS[i].x.sub(MPS[j].x);
                        if (tmp.l2() < 0.000001) continue;

                        tmp.setNorm().setMult(0.00320);
                        MPS[i].v.setAdd(tmp);
                        MPS[j].v.setSub(tmp);
                    }
                }

                // coords
                for (auto &x: MPS) {
                    x.x.setAdd(x.v);
                }
            }
        });
        main_thread.detach();

    }
protected:
    void paintEvent(QPaintEvent *e) {
        QPainter painter(this);

        QPen pen(Qt::black);
        pen.setWidth(6);
        pen.setCapStyle(Qt::PenCapStyle::RoundCap);
        pen.setColor(QColor(0,0,0));
        painter.setPen(pen);

        for(auto& mp: MPS)
            painter.drawPoint(center+QPoint(mp.x.x*scale, mp.x.y*scale));
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
                scale*=1.04;
                break;
            case Qt::Key_X:
                scale*=0.96;
                break;
        }
    };
    void draw() {
        update();
    }
};

#endif //PARTICLES_WINDOW_H
