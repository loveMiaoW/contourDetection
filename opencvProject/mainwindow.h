#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QTimer>
#include <vector>
using namespace cv;
using namespace std;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QImage MatImageToQt(const Mat &src);

private slots:
    void readFarme();

    void writeFarme();

    void on_open_video_clicked();

    void on_close_video_clicked();

    void on_save_video_clicked();

    void on_fun_video_clicked();

private:
    Ui::MainWindow *ui;
    VideoCapture cap;
    Mat src_image;
    Mat out_image;
    Mat frame;
    QTimer *timer;
    QTimer *t;
    QImage image;
    QImage image2;
};
#endif // MAINWINDOW_H
