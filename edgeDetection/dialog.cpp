#include "dialog.h"
#include "ui_dialog.h"
#include<QFileDialog>
#include<QMessageBox>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

/*----------------------------
* 功能 : 响应用户点击打开图片按钮事件
*----------------------------
* 函数 : on_openImgBtn_clicked
* 访问 : private
* 返回 : void
*
*/
void Dialog::on_openImgBtn_clicked()
{
    //打开原始灰度图
    fileName = QFileDialog::getOpenFileName(this,"Open Image",".","Image File(*.png *.jpg *.jpeg *.bmp)");
    grayImg = imread(fileName.toLatin1().data(),IMREAD_GRAYSCALE);
    if(grayImg.empty())
        return;

    //在QLabel中显示图像
    QPixmap *pixmap = new QPixmap(fileName);
    pixmap->scaled(ui->oriImgLabel->size(), Qt::KeepAspectRatio);
    ui->oriImgLabel->setScaledContents(true);
    ui->oriImgLabel->setPixmap(*pixmap);
}

/*----------------------------
* 功能 : 响应用户点击边缘检测按钮事件
*----------------------------
* 函数 : on_edgeImgBtn_clicked
* 访问 : private
* 返回 : void
*
*/
void Dialog::on_edgeImgBtn_clicked()
{
    uchar index = ui->edgeComBox->currentIndex();//获取选择的算子的标号
    Mat edgeImg;

    switch (index) {
        case 0:
        {
            //Canny边缘检测
            Canny(grayImg, edgeImg, 30, 80);
            break;
        }
        case 1:
        {
            //Sobel边缘检测
            Mat x_edgeImg, y_edgeImg;
            Mat abs_x_edgeImg, abs_y_edgeImg;

            /*****先对x方向进行边缘检测********/
            //因为Sobel求出来的结果有正负，8位无符号表示不全，故用16位有符号表示
            Sobel(grayImg,x_edgeImg, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
            convertScaleAbs(x_edgeImg, abs_x_edgeImg);//将16位有符号转化为8位无符号

            /*****再对y方向进行边缘检测********/
            Sobel(grayImg, y_edgeImg, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
            convertScaleAbs(y_edgeImg, abs_y_edgeImg);

            addWeighted(abs_x_edgeImg, 0.5, abs_y_edgeImg, 0.5, 0, edgeImg);
            break;
        }
        case 2:
        {
            //Laplacian边缘检测
            Mat lapImg;

            Laplacian(grayImg, lapImg, CV_16S, 5, 1, 0, BORDER_DEFAULT);
            convertScaleAbs(lapImg, edgeImg);
            break;
        }
        default:
        {
            break;
        }
    }


    imwrite("edge.bmp",edgeImg);

    //在QLabel中显示图像
    QPixmap *pixmap = new QPixmap("edge.bmp");
    pixmap->scaled(ui->edgeImgLabel->size(), Qt::KeepAspectRatio);
    ui->edgeImgLabel->setScaledContents(true);
    ui->edgeImgLabel->setPixmap(*pixmap);
}
/*----------------------------
* 功能 : 响应用户点击轮廓提取按钮事件
*----------------------------
* 函数 : on_conImgBtn_clicked
* 访问 : private
* 返回 : void
*
*/
void Dialog::on_conImgBtn_clicked()
{
    Mat contImg = Mat ::zeros(grayImg.size(),CV_8UC3);//定义三通道轮廓提取图像

    Mat binImg;
    threshold(grayImg, binImg, 127, 255, THRESH_OTSU);//大津法进行图像二值化

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    //查找轮廓
    findContours(binImg, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);
    //绘制查找到的轮廓
    drawContours(contImg, contours, -1, Scalar(0,255,0));

    imwrite("cont.bmp",contImg);

    //在QLabel中显示图像
    QPixmap *pixmap = new QPixmap("cont.bmp");
    pixmap->scaled(ui->conImgLabel->size(), Qt::KeepAspectRatio);
    ui->conImgLabel->setScaledContents(true);
    ui->conImgLabel->setPixmap(*pixmap);
}

/*----------------------------
* 功能 : 响应用户点击轮廓跟踪按钮事件
*----------------------------
* 函数 : on_trackImgBtn_clicked
* 访问 : private
* 返回 : void
*
*/
void Dialog::on_trackImgBtn_clicked()
{
    Mat edgeImg,trackImg;

    // Canny边缘检测
    Canny(grayImg, edgeImg, 50, 100);
    vector<Point> edge_t;
    vector<vector<Point>> edges;
    //边缘跟踪
    EdgeTracking(edgeImg,edge_t,edges,trackImg);

    imwrite("track.bmp",trackImg);

    //在QLabel中显示图像
    QPixmap *pixmap = new QPixmap("track.bmp");
    pixmap->scaled(ui->trackImgLabel->size(), Qt::KeepAspectRatio);
    ui->trackImgLabel->setScaledContents(true);
    ui->trackImgLabel->setPixmap(*pixmap);
}

/*----------------------------
* 功能 : 边缘跟踪函数
*----------------------------
* 函数 : EdgeTracking
* 访问 : private
* 返回 : void
*
* 参数 : Mat& Edge
* 参数 : vector<Point>& edge_t
* 参数 : vector<vector<Point>>& edges
* 参数 : Mat& trace_edge_color
*/
void Dialog::EdgeTracking(Mat& Edge,vector<Point>& edge_t,vector<vector<Point>>& edges,Mat& trace_edge_color)
{
    // 8 neighbors
    const Point directions[8] = { { 0, 1 }, {1,1}, { 1, 0 }, { 1, -1 }, { 0, -1 },  { -1, -1 }, { -1, 0 },{ -1, 1 } };
    int i, j, counts = 0, curr_d = 0;
    for (i = 1; i < Edge.rows - 1; i++)
        for (j = 1; j < Edge.cols - 1; j++)
        {
            // 起始点及当前点
            //Point s_pt = Point(i, j);
            Point b_pt = Point(i, j);
            Point c_pt = Point(i, j);

            // 如果当前点为前景点
            if (255 == Edge.at<uchar>(c_pt.x, c_pt.y))
            {
                edge_t.clear();
                bool tra_flag = false;
                // 存入
                edge_t.push_back(c_pt);
                Edge.at<uchar>(c_pt.x, c_pt.y) = 0;    // 用过的点直接给设置为0

                // 进行跟踪
                while (!tra_flag)
                {
                    // 循环八次
                    for (counts = 0; counts < 8; counts++)
                    {
                        // 防止索引出界
                        if (curr_d >= 8)
                        {
                            curr_d -= 8;
                        }
                        if (curr_d < 0)
                        {
                            curr_d += 8;
                        }

                        // 当前点坐标
                        // 跟踪的过程，应该是个连续的过程，需要不停的更新搜索的root点
                        c_pt = Point(b_pt.x + directions[curr_d].x, b_pt.y + directions[curr_d].y);

                        // 边界判断
                        if ((c_pt.x > 0) && (c_pt.x < Edge.cols - 1) &&
                            (c_pt.y > 0) && (c_pt.y < Edge.rows - 1))
                        {
                            // 如果存在边缘
                            if (255 == Edge.at<uchar>(c_pt.x, c_pt.y))
                            {
                                curr_d -= 2;   // 更新当前方向
                                edge_t.push_back(c_pt);
                                Edge.at<uchar>(c_pt.x, c_pt.y) = 0;

                                // 更新b_pt:跟踪的root点
                                b_pt.x = c_pt.x;
                                b_pt.y = c_pt.y;

                                //cout << c_pt.x << " " << c_pt.y << endl;

                                break;   // 跳出for循环
                            }
                        }
                        curr_d++;
                    }   // end for
                    // 跟踪的终止条件：如果8邻域都不存在边缘
                    if (8 == counts )
                    {
                        // 清零
                        curr_d = 0;
                        tra_flag = true;
                        edges.push_back(edge_t);

                        break;
                    }

                }  // end if
            }  // end while

        }

    // 显示一下
    Mat trace_edge = Mat::zeros(Edge.rows, Edge.cols, CV_8UC1);
    //Mat trace_edge_color;
    cvtColor(trace_edge, trace_edge_color, COLOR_GRAY2BGR);
    for (i = 0; i < edges.size(); i++)
    {
        Scalar color = Scalar(rand()%255, rand()%255, rand()%255);

        // 过滤掉较小的边缘
        if (edges[i].size() > 5)
        {
            for (j = 0; j < edges[i].size(); j++)
            {
                trace_edge_color.at<Vec3b>(edges[i][j].x, edges[i][j].y)[0] = color[0];
                trace_edge_color.at<Vec3b>(edges[i][j].x, edges[i][j].y)[1] = color[1];
                trace_edge_color.at<Vec3b>(edges[i][j].x, edges[i][j].y)[2] = color[2];
            }
        }

    }
}
