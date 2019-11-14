#include "imgviewer.h"
#include <QDebug>
#include <QMessageBox>
#include <QPrintDialog>
#include <QFileDialog>
#include <QWheelEvent>
#include <QtMath>
#include <QMatrix>

ImgViewer::ImgViewer(QWidget *parent) :
    QGraphicsView(parent), m_rotateAngle(0), m_IsFitWindow(false), m_IsViewInitialized(false)
{
    m_scene = new QGraphicsScene(this);
    this->setScene(m_scene);
    this->setBackgroundBrush(QBrush(QColor(38,38,38,255),Qt::SolidPattern));
    this->setDragMode(NoDrag);
}

bool ImgViewer::loadFile(const QString &strFilePath, QString &strError)
{
    // удаление предыдущего и обновление
    resetView();

    m_image.load(strFilePath);
    if(m_image.isNull()){
        strError = QObject::tr("Не удалось загрузить %1.").arg(strFilePath);
        return false;
    }

    m_fileName = strFilePath;
    m_pixmap = QPixmap::fromImage(m_image);
    m_pixmapItem = m_scene->addPixmap(m_pixmap); // добавление pixmap на сцену и возвращение указателя на pixmapItem
    m_scene->setSceneRect(m_pixmap.rect());      // указание сцены для изображения
    this->centerOn(m_pixmapItem);                // обеспечение элементу центрального положение при просмотре

    // сохранить fitWindow, если активирован
    if(m_IsFitWindow)
        fitWindow();
    else
        this->setDragMode(ScrollHandDrag);

    m_IsViewInitialized = true;
    return true;
}

void ImgViewer::resetView()
{
    if(m_image.isNull()){
        return;
    }

    m_scene->clear();
    m_image = QImage();
    m_fileName.clear();
    m_rotateAngle = 0;
    this->setDragMode(NoDrag);
    this->resetTransform();
}

void ImgViewer::reactToFitWindowToggle(bool checked)
{
    m_IsFitWindow = checked;
    if(checked)
    {
        fitWindow();
    }
    else
    {
        originalSize();
    }
}

void ImgViewer::fitWindow()
{
    if(m_image.isNull())
        return;

    this->setDragMode(NoDrag);
    //this->resetTransform();
    QPixmap px = m_pixmap; // масштабировать копию до размера просмотра (масштабирование исходных результатов в размытом изображении)
    if(this->m_rotateAngle == 0 || this->m_rotateAngle == 180 || this->m_rotateAngle == -180)
    {
        px = px.scaled(QSize(this->width(), this->height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        px = px.scaled(QSize(this->height(),this->width()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    m_pixmapItem->setPixmap(px);
    m_scene->setSceneRect(px.rect());
}

void ImgViewer::originalSize()
{
    if(m_image.isNull())
        return;

    this->setDragMode(ScrollHandDrag);
    //m_pixmap = m_pixmap.scaled(QSize(m_image.width(),m_image.height()),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    m_pixmap = m_pixmap.scaled(QSize(m_image.width(),m_image.height()));

    m_pixmapItem->setPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
    this->centerOn(m_pixmapItem);
}

void ImgViewer::rotateView(const int nVal)
{
    if(m_image.isNull()){
        return;
    }

    // поворот изображения
    this->rotate(nVal);

    // обновление угла
    m_rotateAngle += nVal;  // a=a+(-90)== -90

    // сброс угла
    if(m_rotateAngle >= 360 || m_rotateAngle <= -360)
    {
        m_rotateAngle = 0;
    }
}

void ImgViewer::printView()
{
    if(m_image.isNull()){
        return;
    }
#ifndef QT_NO_PRINTER
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        m_scene->render(&painter);
    }
#endif
}

bool ImgViewer::saveViewToDisk(QString &strError)
{
    if(m_image.isNull()){
        strError = QObject::tr("Сохранение не удалось.");
        return false;
    }

    // сохранения копии
    QImage imageCopy = m_image;

    // Output file dialog
    QString fileFormat = getImageFormat(m_fileName);
    QString strFilePath = QFileDialog::getSaveFileName(
               this,
               tr("Сохранить файл"),
               QDir::homePath(),
               fileFormat);


    // Если нажать кнопку «Отмена», getSaveFileName () возвращает пустую строку
    if(strFilePath==""){
        strError = QObject::tr("");
        return false;
    }

    // убеждаемся, что выходной путь имеет правильное расширение
    if(!strFilePath.endsWith(fileFormat))
         strFilePath += "."+fileFormat;

    // сохраняем изображение в измененном состоянии
    if(isModified()) {
        QTransform t;
        t.rotate(m_rotateAngle);
        imageCopy = imageCopy.transformed(t, Qt::SmoothTransformation);
    }

    // фактор разрешения (-1 обычный, 100 максимальный)
    // описание: -1 примерно в 4 раза меньше оригинала, 100 больше оригинала
    if(!imageCopy.save(strFilePath,fileFormat.toLocal8Bit().constData(),100)){
        strError = QObject::tr("Сохранение не удалось.");
        return false;
    }
    return true;
}

QString ImgViewer::getImageFormat(QString strFileName)
{
    QString str = strFileName.mid(strFileName.lastIndexOf(".")+1,-1);
    if(str.toLower() == "tif")
        str = "tiff";
    return str;
}

// сохранить состояние fitWindow при изменении размера окна
void ImgViewer::resizeEvent(QResizeEvent *event)
{
    if(!m_IsViewInitialized)
        return;

    if(m_IsFitWindow)
        fitWindow();
    else {
        QGraphicsView::resizeEvent(event);
        this->centerOn(m_pixmapItem);
    }
}

// масштабируем изображение на wheelEvent
void ImgViewer::wheelEvent(QWheelEvent *event)
{
    // предотвращение масштабирования, когда fitWindow активно
    if(m_IsFitWindow)
        return;

    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // zoomIn фактор
    qreal factor = 1.15;

    // zoomOut фактор
    if(event->delta() < 0)
        factor = 1.0 / factor;

    // масштабировать вид
    this->scale(factor,factor);
    event->accept();
}
