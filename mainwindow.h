#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDir>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QStringList strlDir;
    QDir absPath;
    int iter;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *m_infoLabel;
    void enableControls(bool bEnable);
    QString formatByteSize(int nBytes);
    void updateStatusBarInfo(QString strFile);

private slots:
    void openImage();
    void closeImage();
    void printImage();
    void saveImage();
    void rotateImage();
    void on_actionAbout_triggered();
    void openImageNext();
    void openImagePrev();
    void imageZoomIn();
    void imageZoomOut();
    void CEImage();
};

#endif // MAINWINDOW_H
