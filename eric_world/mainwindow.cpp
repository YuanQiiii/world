#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QDebug>
#include <omp.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);

    std::vector<Point> points = globalcreator();

    Transformation transformation;
    transformation.translate(myposition);
    transformation.zoom(1, 1, 1);

    transformation.rotate(lr * M_PI / 180.0, Point(1, 0, 0, 0)); // Horizontal rotation
    transformation.rotate(ud * M_PI / 180.0, Point(0, 0 , 1, 0)); // Vertical rotation
    transformation.rotate(ud * M_PI / 180.0, Point(0, 1 , 0, 0)); // Vertical rotation
    transformation.project(100 , 1, 0.01, 1000, 30.0); // 添加缩放因子

    Matrix transformation_matrix = transformation.combine();
    std::vector<Point> todraw(points.size());

    // 设置随机数生成器
    srand(static_cast<unsigned int>(time(nullptr)));

    #pragma omp parallel for
    for (size_t i = 0; i < points.size(); ++i) {
        todraw[i] = transformation_matrix.transformation_right(points[i]);
    }

    pcount = int(todraw.size());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置背景颜色，以确保与点的颜色形成对比
    painter.fillRect(this->rect(), Qt::black);

    // 绘制每个点
    for (const Point &p : todraw)
    {
        // 随机生成颜色
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        QPen pen(QColor(r, g, b));
        pen.setWidth(1);
        painter.setPen(pen);

        // 将点的坐标平移到窗口中心
        painter.drawPoint(QPointF(p.position[0] + width() / 2, p.position[1] + height() / 2));
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const double moveSpeed = 0.5;

    switch (event->key())
    {
    case Qt::Key_W:
        myposition.position[0] += moveSpeed;
        break;
    case Qt::Key_S:
        myposition.position[0] -=  moveSpeed;
        break;
    case Qt::Key_A:

        myposition.position[1] -=  moveSpeed;
        break;
    case Qt::Key_D:

        myposition.position[1] +=  moveSpeed;
        break;
    case Qt::Key_Q:
        myposition.position[2] += moveSpeed;
        break;
    case Qt::Key_E:
        myposition.position[2] -= moveSpeed;
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
    update();
     ui->datalabel->setText(showdata());
    qDebug() << "keyboard";
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentPos = event->pos();
    if (firstMouseMove) {
        lastMousePos = currentPos;
        firstMouseMove = false;
    }

    int dx = currentPos.x() - lastMousePos.x();
    int dy = currentPos.y() - lastMousePos.y();

    lr -= dx * 0.1; // 水平旋转 (绕 Y 轴)
    ud += dy * 0.1; // 垂直旋转 (绕 X 轴)

    // 限制俯仰角度以避免翻转
    if (ud > 89.0)
        ud = 89.0;
    if (ud < -89.0)
        ud = -89.0;

    lastMousePos = currentPos;

    update();
    qDebug() << "mouse";
    ui->datalabel->setText(showdata());
}


