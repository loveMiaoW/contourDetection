#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("提取视频轮廓");
    this->setFixedSize(this->geometry().size());
    timer = new QTimer(this);
    t = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(readFarme()));
    connect(t,SIGNAL(timeout()),this,SLOT(writeFarme()));
    connect(ui->open_video,SIGNAL(clicked()),this,SLOT(on_open_video_clicked()));
    connect(ui->close_video,SIGNAL(clicked()),this,SLOT(on_close_video_clicked()));

    connect(ui->fun_video,SIGNAL(clicked()),this,SLOT(on_fun_video_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
Mat result;
int tmp;
void MainWindow::readFarme()
{
    cap.read(src_image);
    image = MatImageToQt(src_image);
    ui->src_video->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::writeFarme()
{

    cv::Mat frame, gray, blurred, edges;
    std::vector<std::vector<cv::Point>> contours;

    // 读取一帧图像
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "Frame is empty" << std::endl;
        return;
    }

    // 将图像转换为灰度图
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 应用高斯模糊
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // 动态调整Canny边缘检测的阈值
    double sigma = 0.33;
    double v = cv::mean(blurred)[0];
    double lower = std::max(0.0, (1.0 - sigma) * v);
    double upper = std::min(255.0, (1.0 + sigma) * v);
    cv::Canny(blurred, edges, lower, upper);

    // 查找轮廓
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 绘制轮廓并显示结果
    cv::Mat drawing = cv::Mat::zeros(gray.size(), CV_8UC1);
    cv::drawContours(drawing, contours, -1, cv::Scalar(255), 2);

    image2 = MatImageToQt(drawing);
    ui->out_video->setPixmap(QPixmap::fromImage(image2));
}



QImage MainWindow::MatImageToQt(const cv::Mat &mat)
{
//    //CV_8UC1 8位无符号的单通道---灰度图片
//    if(src.type() == CV_8UC1)
//    {
//        //使用给定的大小和格式构造图像
//        //QImage(int width, int height, Format format)
//        QImage qImage(src.cols,src.rows,QImage::Format_Indexed8);
//        //扩展颜色表的颜色数目
//        qImage.setColorCount(256);

//        //在给定的索引设置颜色
//        for(int i = 0; i < 256; i ++)
//        {
//            //得到一个黑白图
//            qImage.setColor(i,qRgb(i,i,i));
//        }
//        //复制输入图像,data数据段的首地址
//        uchar *pSrc = src.data;
//        //
//        for(int row = 0; row < src.rows; row ++)
//        {
//            //遍历像素指针
//            uchar *pDest = qImage.scanLine(row);
//            //从源src所指的内存地址的起始位置开始拷贝n个
//            //字节到目标dest所指的内存地址的起始位置中
//            memcmp(pDest,pSrc,src.cols);
//            //图像层像素地址
//            pSrc += src.step;
//        }
//        return qImage;
//    }
//    //为3通道的彩色图片
//    if(src.type() == CV_8UC3)
//    {
//        //得到图像的的首地址
//        const uchar *pSrc = (const uchar*)src.data;
//        //以src构造图片
//        QImage qImage(pSrc,src.cols,src.rows,src.step,QImage::Format_RGB888);
//        //在不改变实际图像数据的条件下，交换红蓝通道
//        return qImage.rgbSwapped();
//    }
//    //四通道图片，带Alpha通道的RGB彩色图像
//    if(src.type() == CV_8UC4)
//    {
//        const uchar *pSrc = (const uchar*)src.data;
//        QImage qImage(pSrc, src.cols, src.rows, src.step, QImage::Format_ARGB32);
//        //返回图像的子区域作为一个新图像
//        return qImage.copy();
//    }
//    else
//    {
//        return QImage();
//    }
    switch (mat.type())
    {
        // 8位无符号的单通道---灰度图片
        case CV_8UC1:
        {
            QImage image(mat.cols, mat.rows, QImage::Format_Grayscale8);
            // 复制Mat数据到QImage
            uchar *pSrc = mat.data;
            for (int row = 0; row < mat.rows; row++)
            {
                uchar *pDest = image.scanLine(row);
                memcpy(pDest, pSrc, mat.cols);
                pSrc += mat.step;
            }
            return image;
        }
        // 3通道的彩色图片
        case CV_8UC3:
        {
            const uchar *pSrc = (const uchar*)mat.data;
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }
        // 四通道图片，带Alpha通道的RGB彩色图像
        case CV_8UC4:
        {
            const uchar *pSrc = (const uchar*)mat.data;
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image;
        }
        default:
            return QImage();
    }
}


void MainWindow::on_open_video_clicked()
{
    cap.open(0);
    timer->start(33);
}

void MainWindow::on_close_video_clicked()
{
    timer->stop();
    cap.release();
    ui->src_video->clear();
}

void MainWindow::on_save_video_clicked()
{
    image.save("out.png");
}

void MainWindow::on_fun_video_clicked()
{
    t->start(33);
}
