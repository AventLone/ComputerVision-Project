#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "detectimage.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_SelectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,QStringLiteral("文件对话框！"),"D:",QStringLiteral("(*png *jpg);"));
    ui->lineEdit->setText(fileName); // 给lineEdit赋值
}

void MainWindow::on_pushButton_DetectImage_clicked()
{
    string imgPath = ui->lineEdit->text().toStdString(); //从lineEdit中获取字符串

    if(imgPath.empty())
    {
        QMessageBox::critical(this, "Warning!" , "选张图片呀! 傻牛 QAQ");
    }
    else
    {
        // Reading image
        Mat img = imread(imgPath);

        DetectImage(img, imgPath);
    }
}
