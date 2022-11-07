# QT 封装边缘检测程序

## 边缘检测

### 边缘检测的步骤

- **滤波**：边缘检测的算法主要是基于图像强度的一阶和二阶导数，但导数通常对噪声很敏感，因此必须采用滤波器来改善与噪声有关的边缘检测器的性能。
- **增强**：增强边缘的基础是确定图像各点邻域的变化值。增强算法可以将图像灰度点邻域强度值有显著变化的点凸显出来。
- **检测**：经过增强的图像，往往邻域中有很多点的梯度值比较大，而在特定的应用中，这些点并不是要找的边缘点，所以应该采用某种方法来对这些点进行取舍。

### Canny 算子

canny边缘检测算子是一个多级边缘检测算法。
最优边缘检测的评价标准：低错误率、高定位性、最小响应。为了满足这些要求，Canny使用了变分法，这是一种寻找满足特定功能的函数的方法。最优检测用4个指数函数项的和表示，但是它非常近似于高斯函数的一阶导数。

Canny函数利用Canny算子来进行图像的边缘检测操作。
```c++
void Canny(
InputArray image, 
OutputArray edges, 
double threshold1, 
double threshold2, 
int apertureSize = 3, 
bool L2gradient = false) ;
```

代码中：

- **InputArray image**：为输入的图像，即源图像，填 Mat 类的对象即可，且需为单通道 8 位图像；
- **OutputArray edges**：输出的边缘图，需要和源图片有一样的尺寸和类型；
- **double threshold1**：第一个滞后性阈值；
- **double threshold2**：第二个滞后性阈值；
- **int apertureSize**：表示应用 Sobel 算子的孔径大小，其有默认值 3；
- **bool L2gradient**：计算图像梯度幅值的标识，默认值 false。

 ```c++
 #include<opencv2/opencv.hpp>
 #include<opencv2/imgproc/imgproc.hpp>
 using namespace cv;
 
 int main()
 {
 	Mat srcImage = imread("fg.jpg");
 	imshow("[原图]Canny边缘检测", srcImage);
 
 	Mat dstImage, edge, grayImage;
 	dstImage.create(srcImage.size(), srcImage.type());
 	cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
 	blur(grayImage, edge, Size(3, 3));
 	Canny(edge, edge, 3, 9, 3);
 	imshow("[效果图]Canny边缘检测", edge);
 	waitKey(0);
 
 	return 0;
 }
 ```

## Qt 程序

### 文件交互窗口

```c++
#include <QFileDialog>
QString fileName = QFileDialog::getOpenFileName(this,QStringLiteral("文件对话框！"),"D:",QStringLiteral("(*png *jpg);"));
```



### 文本框赋值与取值

```c++
ui->lineEdit->setText(fileName); // 给lineEdit赋值
string imgPath = ui->lineEdit->text().toStdString(); //从lineEdit中获取字符串
```

`QString`与`string`之间的转换：

```c++
QString A;
string B;
B = A.toStdString();  //QString to string
A = QString::fromStdString(B)  //string to QString
```



### 弹出警告窗口

```c++
#include <QMessageBox>
QMessageBox::critical(this, "Warning!" , "选张图片呀! 傻牛 QAQ");
```

