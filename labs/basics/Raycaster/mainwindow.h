#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPoint>
#include <QMouseEvent>
#include <QBrush>
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPolygonItem>
#include <QPolygonF>

#include <vector>
#include <cmath>
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CustomScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CustomScene(QObject *parent = nullptr);
    
signals:
    void mouseMoved(QPointF scenePos);
    
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

class Ray {
public:
    Ray(const QPoint& begin, const QPointF& vec);
    Ray rotate(const double& angle) const;
    QPointF get_vec() const;
    double get_end() const;
    QPoint get_begin() const;
    void set_end(const double& end);
private:
    QPoint begin_;
    double end_ = 100000.0;
    QPointF vec_;
};

class Polygon {
public:
    std::vector<QPoint> get_vertices() const;
    void AddVertex(const QPoint& vertex);
    std::optional<QPoint> IntersectRay(const Ray& ray) const;
private:
    std::vector<QPoint> vertices_;
};    

class Controller {
public:
    std::vector<Polygon> get_polygons() const;
    void AddPolygon();
    void AddVertexToLastPolygon(const QPoint& new_vertex);
private:
    std::vector<Polygon> polygons_;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    QComboBox* mode;
    QGraphicsView* view;
    CustomScene* scene;

    Controller controller;
    bool need_new_polygon = true;

private slots:
    void myMove(const QPoint& pos1);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
};

bool comp(const Ray& a, const Ray& b);

#endif // MAINWINDOW_H
