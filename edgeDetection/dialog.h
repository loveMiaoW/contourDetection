#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
using namespace std;
using namespace cv;
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


private:
    Ui::Dialog *ui;
    Mat grayImg;      //原始灰度图
    QString fileName; //图片文件名

    void EdgeTracking(Mat& Edge,vector<Point>& edge_t,vector<vector<Point>>& edges,Mat& trace_edge_color);

private slots:
    void on_openImgBtn_clicked();
    void on_edgeImgBtn_clicked();
    void on_conImgBtn_clicked();
    void on_trackImgBtn_clicked();
};

#endif // DIALOG_H
