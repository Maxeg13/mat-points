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

#include <set>
#include <mutex>
#include <condition_variable>

class Window: public QMainWindow {
    Q_OBJECT
private:
    static constexpr int threads_num = 20;
    static constexpr int N = 2510/threads_num*threads_num;

    vector<MatPoint> MPS;
    set<pair<::uint16_t , ::uint16_t>> pairs[threads_num];

    bool pause = 0;
    QPoint center0;
    QPoint center;
    static constexpr int thread_step = N / threads_num;
    std::thread ts[threads_num];
    int i1[threads_num];
    std::mutex mtx;

    const float rigid_dist = 3.3;
    const float rigid_dist2 = rigid_dist * rigid_dist;
    const float rigid_dist_lim_k = 1.3;
    const float rigid_dist2_lim =  rigid_dist * rigid_dist * rigid_dist_lim_k * rigid_dist_lim_k;
    float scale = 0.7;
    float scale_k = 1.06;
public:
    Window(QWidget *parent = 0, const char *name = 0):QMainWindow(parent),
    center(600, 450)
//    center(1100, 650)
    {
        center0 = QPoint(center);
        Point rot(0,0,0.00000);
        const float rk = 0.58;
        for(int i=0; i<N; i++) {

            if(i<N*0.5) {
                MPS.push_back(Point::rnd(10, 150).mult(rk));
                rot.z = 0.0014;
            }
            else if(i<N*1) {
                MPS.push_back(Point::rnd(150, 250).mult(rk));
                rot.z = 0.0013;
            }
            else {
                MPS.push_back(Point::rnd(450, 650).mult(rk));
                rot.z = 0.00035;
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

                    Point r = MPS[j].x - (MPS[i].x);
                    float rr = r.l2();
                    if (rr > rigid_dist2) {
                        auto el = pairs[idx].find(make_pair(i, j));
                        if (el == pairs[idx].end()) {
                            Point dir = r.norm();
                            float krr = 0.007 / (rr);
                            MPS[i].v.setAdd(dir.mult(krr));
                            MPS[j].v.setAdd(dir.mult(-krr));
                        }
                        if (rr > rigid_dist2_lim) {
//                            std::unique_lock<std::mutex> lock(mtx);
                            if (el != pairs[idx].end())
                                pairs[idx].erase(el);
                        }
                    } else {
//                        std::unique_lock<std::mutex> lock(mtx);
                        pairs[idx].insert(make_pair(i, j));
                    }
                }
            }

        };

        std::thread main_thread([this, lam]{
            while(true)
            {
                if(pause) continue;

                for (int i = 0; i < threads_num; i++) {
                    ts[i] = std::move(std::thread(lam, i));
                }

                for (int i = 0; i < threads_num; i++) {
                    ts[i].join();
                }

                // pairs vs
//                qDebug()<<pairs->size();
                for(int idx = 0; idx < threads_num; idx++) {
                    for (auto &p: pairs[idx]) {
                        int i = p.first;
                        int j = p.second;
                        auto tmp = MPS[i].x - MPS[j].x;
                        if (tmp.l2() > 0.0000001) {
                            tmp.setNorm();
                            Point V1n = MPS[i].v.getComponent(tmp);
                            Point V2n = MPS[j].v.getComponent(tmp);

                            static const float k = 0.034;
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
                        int i = p.first;
                        int j = p.second;

                        // push away
                        auto tmp = MPS[i].x - MPS[j].x;
                        if (tmp.l2() < 0.0000001) continue;

                        tmp.setNorm().setMult(0.0021);
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
    void changeCenter(float k) {
        center+=((center0-center)*(1 - k));
    }

    void paintEvent(QPaintEvent *e) {
        QPainter painter(this);

        QPen pen(Qt::black);
        pen.setWidth(6);
        pen.setCapStyle(Qt::PenCapStyle::RoundCap);
        pen.setColor(QColor(0,0,0));
        painter.setPen(pen);

        for(auto& mp: MPS)
            painter.drawPoint(center+QPoint(mp.x.x*scale, mp.x.y*scale));

        {
            pen.setWidth(3);
            QPoint tmp(30, 30);
            painter.drawLine(tmp, tmp+QPoint(rigid_dist*scale, 0));
        }

        {
            pen.setWidth(3);
            QPoint tmp(30, 36);
            painter.drawLine(tmp, tmp+QPoint(sqrt(rigid_dist2_lim)*scale, 0));
        }
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
                scale*=scale_k;
                changeCenter(scale_k);
                break;
            case Qt::Key_X:
                scale*=(2 - scale_k);
                changeCenter(2 - scale_k);
                break;
            case Qt::Key_P: {
                pause = !pause;
            }
                break;
        }
    };
    void draw() {
        update();
    }
};

#endif //PARTICLES_WINDOW_H
