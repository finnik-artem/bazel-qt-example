#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setMouseTracking(true);

    setGeometry(0, 0, 800, 700);

    mode = new QComboBox(this);
    mode->addItem("Polygons");
    mode->addItem("Light");
    mode->setGeometry(10, 620, 150, 40);

    scene = new CustomScene(this);
    view = new QGraphicsView(scene, this);
    scene->setBackgroundBrush(Qt::gray);
    view->setGeometry(0, 0, 800, 600);
    view->setMouseTracking(true);

    QPoint point1(0, 0);
    QPoint point2(800, 0);
    QPoint point3(800, 600);
    QPoint point4(0, 600);
    controller.AddPolygon();
    controller.AddVertexToLastPolygon(point1);
    controller.AddVertexToLastPolygon(point2);
    controller.AddVertexToLastPolygon(point3);
    controller.AddVertexToLastPolygon(point4);

    connect(scene, &CustomScene::mouseMoved, this, [this](QPointF pos) {
        QPoint normpos(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
        myMove(normpos);
    });
}

CustomScene::CustomScene(QObject *parent) : QGraphicsScene(parent) {
}

void CustomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QPointF scenePos = event->scenePos();
    emit mouseMoved(scenePos); // Отправляем сигнал с координатами
    QGraphicsScene::mouseMoveEvent(event); // Важно!
}

Ray::Ray(const QPoint& begin, const QPointF& vec) : begin_(begin), vec_(vec) {
    if (vec_.x() == 0.0) {
        vec_.setX(0.00001);
    }
    if (vec_.y() == 0.0) {
        vec_.setY(0.00001);
    }
}

Ray Ray::rotate(const double& angle) const {
    QPointF vec(vec_.x(), vec_.y() + angle);
    Ray a(begin_, vec);
    return a;
}

QPointF Ray::get_vec() const {
    return vec_;
}

double Ray::get_end() const {
    return end_;
}

QPoint Ray::get_begin() const {
    return begin_;
}

void Ray::set_end(const double& end) {
    end_ = end;
}

std::vector<QPoint> Polygon::get_vertices() const {
    return vertices_;
}

void Polygon::AddVertex(const QPoint& vertex) {
    vertices_.push_back(vertex);
}

// std::optional<QPoint> Polygon::IntersectRay(const Ray& ray) const {
    
// }

std::vector<Polygon> Controller::get_polygons() const {
    return polygons_;
}

void Controller::AddPolygon() {
    Polygon a;
    polygons_.push_back(a);
}

void Controller::AddVertexToLastPolygon(const QPoint& new_vertex) {
    polygons_[polygons_.size() - 1].AddVertex(new_vertex);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    if (mode->currentIndex() == 1 || pos.x() > 800 || pos.y() > 600) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        if (need_new_polygon) {
            controller.AddPolygon();
            need_new_polygon = false;
        }
        controller.AddVertexToLastPolygon(pos);
        scene->clear();
        for (const auto& i : controller.get_polygons()) {
            const auto& vertices = i.get_vertices();
            for (size_t j = 0; j < vertices.size(); j++) {
                scene->addLine(QLine(vertices[j], vertices[(j + 1) % vertices.size()]), QPen(Qt::darkGreen, 2));
            }
        }
    } else if (event->button() == Qt::RightButton) {
        need_new_polygon = true;
    }
}

void MainWindow::myMove(const QPoint& pos1) {
    if (mode->currentIndex() == 0 || pos1.x() > 800 || pos1.y() > 600) {
        return;
    }

    scene->clear();

    std::vector<QPoint> sources(7);

    QPoint point1(pos1.x() + 5, pos1.y() + 10);
    QPoint point2(pos1.x() - 5, pos1.y() + 10);
    QPoint point3(pos1.x() - 11, pos1.y());
    QPoint point4(pos1.x() - 5, pos1.y() - 10);
    QPoint point5(pos1.x() + 5, pos1.y() - 10);
    QPoint point6(pos1.x() + 11, pos1.y());

    sources.push_back(pos1);
    sources.push_back(point1);
    sources.push_back(point2);
    sources.push_back(point3);
    sources.push_back(point4);
    sources.push_back(point5);
    sources.push_back(point6);

    for (size_t w = 0; w < sources.size(); w++) {
        QPoint pos = sources[w];
        if (pos.x() < 0 || pos.y() < 0 || pos.x() > 800 || pos.y() > 600) {
            continue;
        }
        std::vector <Ray> rays;
        for (const auto& i : controller.get_polygons()) {
            const auto& vertices = i.get_vertices();
            for (size_t j = 0; j < vertices.size(); j++) {
                QPointF vec(vertices[j].x() - pos.x(), vertices[j].y() - pos.y());
                Ray a(pos, vec);
                Ray a1 = a.rotate(7), a2 = a.rotate(-7);
                rays.push_back(a);
                rays.push_back(a1);
                rays.push_back(a2);
            }
        }

        for (size_t u = 0; u < rays.size(); u++) {
            for (const auto& i : controller.get_polygons()) {
                const auto& vertices = i.get_vertices();
                for (size_t j = 0; j < vertices.size(); j++) {
                    QPointF vec = rays[u].get_vec();
                    int dx = vertices[(j + 1) % vertices.size()].x() - vertices[j].x();
                    int dy = vertices[(j + 1) % vertices.size()].y() - vertices[j].y();
                    if (vec.x() / dx == vec.y() / dy) {
                        continue;
                    }
                    double t2 = (vec.x() * (vertices[j].y() - rays[u].get_begin().y()) + vec.y() * (rays[u].get_begin().x() - vertices[j].x())) / (dx * vec.y() - dy * vec.x());
                    double t1 = (vertices[j].x() + dx * t2 - rays[u].get_begin().x()) / vec.x();
                    if (t2 >= 0 && t2 <= 1 && t1 >= 0 && t1 < rays[u].get_end()) {
                        rays[u].set_end(t1);
                    }
                }
            }
        }

        sort(rays.begin(), rays.end(), comp);

        for (size_t i = 0; i < rays.size(); i++) {
            QPolygonF triangle;
            triangle << rays[i].get_begin() << rays[i].get_begin() + rays[i].get_vec() * rays[i].get_end() << rays[i].get_begin() + rays[(i + 1) % rays.size()].get_vec() * rays[(i + 1) % rays.size()].get_end();
            QGraphicsPolygonItem *triangleItem = new QGraphicsPolygonItem(triangle);

            QColor fillColor(Qt::darkGreen);
            fillColor.setAlpha(36);
            triangleItem->setBrush(QBrush(fillColor));
            triangleItem->setPen(Qt::NoPen);
            scene->addItem(triangleItem);
        }
    }

    for (const auto& i : controller.get_polygons()) {
        const auto& vertices = i.get_vertices();
        for (size_t j = 0; j < vertices.size(); j++) {
            scene->addLine(QLine(vertices[j], vertices[(j + 1) % vertices.size()]), QPen(Qt::darkGreen, 2));
        }
    }
}

bool comp(const Ray& a, const Ray& b) {
    double dx1 = a.get_vec().x();
    double dy1 = a.get_vec().y();
    double dx2 = b.get_vec().x();
    double dy2 = b.get_vec().y();
    if (dx1 <= 0 && dx2 <= 0) {
        if (dy1 / dx1 >= dy2 / dx2) {
            return true;
        } else {
            return false;
        }
    } else if (dx1 >= 0 && dx2 >= 0) {
        if ((dy1 >= 0 && dy2 >= 0) || (dy1 <= 0 && dy2 <= 0)) {
            if (dy1 / dx1 >= dy2 / dx2) {
                return true;
            } else {
                return false;
            }
        } else if (dy1 <= 0) {
            return true;
        } else {
            return false;
        }
    } else if (dx1 >= 0) {
        if (dy1 >= 0) {
            return false;
        } else {
            return true;
        }
    } else {
        if (dy2 >= 0) {
            return true;
        } else {
            return false;
        }
    }
}