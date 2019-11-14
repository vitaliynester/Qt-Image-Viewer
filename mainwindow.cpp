#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdlg.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("QLabel {padding-left:3px;}");
    this->setStyleSheet("QStatusBar::item {border:none;}");


    // статус бар слева снизу (основная инфа, а тут её настройка)
    QPalette palette;
    palette.setColor( QPalette::WindowText,"#cccccc");
    ui->statusBar->setPalette(palette);


    ui->statusBar->addWidget(m_infoLabel);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openImage()));   
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(closeImage()));
    connect(ui->actionFitWindow, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(reactToFitWindowToggle(bool)));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(printImage()));   
    connect(ui->actionSave,SIGNAL(triggered()), this, SLOT(saveImage()));    
    connect(ui->actionRotate_Left, SIGNAL(triggered()), this, SLOT(rotateImage())); 
    connect(ui->actionRotate_right, SIGNAL(triggered()), this, SLOT(rotateImage()));
    connect(ui->actionPrevImage, SIGNAL(triggered()), this, SLOT(openImagePrev()));
    connect(ui->actionNextPage, SIGNAL(triggered()), this, SLOT(openImageNext()));
    connect(ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(imageZoomIn()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(imageZoomOut()));
    connect(ui->actionCE, SIGNAL(triggered()), this, SLOT(CEImage()));

    enableControls(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CEImage()
{
    ui->graphicsView->scale(0.1, 0.1);
    ui->graphicsView->resetMatrix();
    ui->graphicsView->scale(1,1);
}

void MainWindow::openImagePrev()
{
    QString strError;
    iter = iter - 1;
    if(iter>=0)
    {
        QString newFileNmae = absPath.absolutePath() + '/' + strlDir.at(iter);
        if(!ui->graphicsView->loadFile(newFileNmae,strError) ){
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this,tr("Error"),tr(newFileNmae.toLocal8Bit().constData())); //"ошибка при попытке изображения."
            return;
        }
        QApplication::restoreOverrideCursor();
        ui->actionFitWindow->setChecked(false);
        updateStatusBarInfo(newFileNmae);
        enableControls(true);
    }
}

void MainWindow::imageZoomIn()
{
    if(!ui->actionFitWindow->isChecked())
    {
        qreal factor = 1.15;
        ui->graphicsView->scale(factor, factor);
    }
}

void MainWindow::imageZoomOut()
{
    if(!ui->actionFitWindow->isChecked())
    {
        qreal factor = 1.15;
        factor = 1 / factor;
        ui->graphicsView->scale(factor, factor);
    }
}

void MainWindow::openImageNext()
{
    QString strError;
    iter = iter + 1;
    if(iter != strlDir.size())
    {
        QString newFileNmae = absPath.absolutePath() + '/' + strlDir.at(iter);
        if(!ui->graphicsView->loadFile(newFileNmae,strError) ){
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this,tr("Error"),tr(strError.toLocal8Bit().constData())); //"ошибка при попытке изображения."
            return;
        }
        QApplication::restoreOverrideCursor();
        ui->actionFitWindow->setChecked(false);
        updateStatusBarInfo(newFileNmae);
        enableControls(true);
    }
}

void MainWindow::enableControls(bool bEnable)
{ 
    ui->actionClear->setEnabled(bEnable);
    ui->actionPrint->setEnabled(bEnable);
    ui->actionRotate_Left->setEnabled(bEnable);
    ui->actionRotate_right->setEnabled(bEnable);
    ui->actionSave->setEnabled(bEnable);
    ui->actionFitWindow->setEnabled(bEnable);
    ui->actionNextPage->setEnabled(bEnable);
    ui->actionPrevImage->setEnabled(bEnable);
    ui->actionZoomIn->setEnabled(bEnable);
    ui->actionZoomOut->setEnabled(bEnable);
    ui->actionCE->setEnabled(bEnable);
}

void MainWindow::openImage()
{
    QString strFilePath = QFileDialog::getOpenFileName(
                this,
                tr("Открыть файл"),
                QDir::homePath(),
                tr("Изображения (*.png *.jpg *.bmp *.tiff *.tif *.gif)"));

    if (!strFilePath.isEmpty()){

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString strError;

        QDir fileDir = QFileInfo(strFilePath).absoluteDir();
        absPath = QFileInfo(strFilePath).absolutePath();
        QStringList filters;
        filters<<"*.png"<< "*.jpg"<< "*.bmp" << "*.tiff" << "*.tif" << "*.gif";
        fileDir.setNameFilters(filters);
        strlDir = fileDir.entryList();
        iter = strlDir.indexOf(strFilePath);

        if(!ui->graphicsView->loadFile(strFilePath,strError) ){
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this,tr("Error"),tr(strError.toLocal8Bit().constData()));
            return;
        }
        QApplication::restoreOverrideCursor();

        updateStatusBarInfo(strFilePath);
        enableControls(true);
    }
}

void MainWindow::printImage()
{
    ui->graphicsView->printView();
}

void MainWindow::closeImage()
{
    ui->graphicsView->resetView();
    m_infoLabel->setText("");
    enableControls(false);
}

void MainWindow::saveImage()
{
    QString strError;
    if(!ui->graphicsView->saveViewToDisk(strError)){
        QApplication::restoreOverrideCursor();
        if(!strError.isEmpty())
            QMessageBox::information(this,tr("Error"),strError);
        return;
    }
}

void MainWindow::rotateImage()
{
    QObject* obj = sender();
    ui->actionFitWindow->setChecked(false);
    if(obj == ui->actionRotate_Left)
        ui->graphicsView->rotateView(-90);
    else
        ui->graphicsView->rotateView(90);
}

QString MainWindow::formatByteSize(int nBytes)
{
    int B = 1;
    int KB = 1024 * B;
    int MB = 1024 * KB;
    int GB = 1024 * MB;
    QString str;

    if(nBytes > GB)         str = QString::number(nBytes/GB, 'f', 2) + " GB";
    else if(nBytes > MB)    str = QString::number(nBytes/MB, 'f', 2) + " MB";
    else if(nBytes > KB)    str = QString::number(nBytes/KB, 'f', 2) + " KB";
    else                    str = QString::number(nBytes, 'f', 2) + " B";

    return str;
}

void MainWindow::updateStatusBarInfo(QString strFile)
{
    QFileInfo imgInfo(strFile);
    QDateTime dtLastModified = imgInfo.lastModified();
    QString strStatusInfo = QString(strFile+"   "+"~%1"+"   "+"%2").arg(formatByteSize(imgInfo.size())).arg(dtLastModified.toString("d/M/yyyy hh:mm:ss"));
    m_infoLabel->setText(strStatusInfo);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDlg dlg(this);
    dlg.exec();
}
