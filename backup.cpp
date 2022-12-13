#include <QtWidgets>
#include <QtPrintSupport/QPrintDialog>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#endif

#include "imageviewer.h"

//! [0]
ImageViewer::ImageViewer()
    : imageLabel(new QLabel)
    , scrollArea(new QScrollArea)
    , scaleFactor(1)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);//
}

//! [0]
//! [2]

//bool ImageViewer::loadFile(const QString& fileName)
//{
//    QImageReader reader(fileName);
//    reader.setAutoTransform(true);
//    const QImage newImage = reader.read();
//    if (newImage.isNull()) {
//        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
//            tr("Cannot load %1: %2")
//            .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
//        return false;
//    }
//    //! [2]
//
//    setImage(newImage);
//
//    setWindowFilePath(fileName);
//
//    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
//        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
//    statusBar()->showMessage(message);
//    return true;
//}

void ImageViewer::setImage(const QImage& newImage)
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    //! [4]
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}

//! [4]

bool ImageViewer::saveFile(const QString& fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot write %1: %2")
            .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

//! [1]

static void initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach(const QByteArray & mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}
//! [1]

void ImageViewer::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

//! [5]
void ImageViewer::print()
//! [5] //! [6]
{
    Q_ASSERT(imageLabel->pixmap());
#if QT_CONFIG(printdialog)
    //! [6] //! [7]
    QPrintDialog dialog(&printer, this);
    //! [7] //! [8]
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}
//! [8]

void ImageViewer::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData* mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

//void ImageViewer::paste()
//{
//#ifndef QT_NO_CLIPBOARD
//    const QImage newImage = clipboardImage();
//    if (newImage.isNull()) {
//        statusBar()->showMessage(tr("No image in clipboard"));
//    }
//    else {
//        setImage(newImage);
//        setWindowFilePath(QString());
//        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
//            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
//        statusBar()->showMessage(message);
//    }
//#endif // !QT_NO_CLIPBOARD
//}

//! [9]
void ImageViewer::zoomIn()
//! [9] //! [10]
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

//! [10] //! [11]
void ImageViewer::normalSize()
//! [11] //! [12]
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}
//! [12]

//! [13]
void ImageViewer::fitToWindow()
//! [13] //! [14]
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}
//! [14]


//! [15]
void ImageViewer::about()
//! [15] //! [16]
{
    QMessageBox::about(this, tr("About Image Viewer"),
        tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
            "and QScrollArea to display an image. QLabel is typically used "
            "for displaying a text, but it can also display an image. "
            "QScrollArea provides a scrolling view around another widget. "
            "If the child widget exceeds the size of the frame, QScrollArea "
            "automatically provides scroll bars. </p><p>The example "
            "demonstrates how QLabel's ability to scale its contents "
            "(QLabel::scaledContents), and QScrollArea's ability to "
            "automatically resize its contents "
            "(QScrollArea::widgetResizable), can be used to implement "
            "zooming and scaling features. </p><p>In addition the example "
            "shows how to use QPainter to print an image.</p>"));
}
//! [16]

//! [17]
void ImageViewer::createActions()
//! [17] //! [18]
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction* exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    //QAction* pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    //pasteAct->setShortcut(QKeySequence::Paste);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}
//! [18]

//! [21]
void ImageViewer::updateActions()
//! [21] //! [22]
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}
//! [22]

//! [23]
void ImageViewer::scaleImage(double factor)
//! [23] //! [24]
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}
//! [24]

//! [25]
void ImageViewer::adjustScrollBar(QScrollBar* scrollBar, double factor)
//! [25] //! [26]
{
    scrollBar->setValue(int(factor * scrollBar->value()
        + ((factor - 1) * scrollBar->pageStep() / 2)));
}




void ImageViewer::ShowBand(GDALRasterBand* band) {
    if (band == NULL)
    {
        return;
    }

    QList<GDALRasterBand*> myBand;
    myBand.append(band);
    myBand.append(band);
    myBand.append(band);

    ShowImg(&myBand);

}


void ImageViewer::ShowImg(QList<GDALRasterBand*>* imgBand)
{
    if (imgBand->size() != 3)
    {
        return;
    }

    int imgWidth = imgBand->at(0)->GetXSize();
    int imgHeight = imgBand->at(0)->GetYSize();

    m_scaleFactor = this->height() * 2.0 / imgHeight;//this->height() * 1.0 / imgHeight

    int iScaleWidth = (int)(imgWidth * m_scaleFactor - 1);
    int iScaleHeight = (int)(imgHeight * m_scaleFactor - 1);

    GDALDataType dataType = imgBand->at(0)->GetRasterDataType();

    // 首先分别读取RGB三个波段
    float* rBand = new float[iScaleWidth * iScaleHeight];
    float* gBand = new float[iScaleWidth * iScaleHeight];
    float* bBand = new float[iScaleWidth * iScaleHeight];

    unsigned char* rBandUC, * gBandUC, * bBandUC;

    // 根据是否显示彩色图像，判断RGB三个波段的组成方式，并分别读取
    if (m_showColor == true)
    {
        imgBand->at(0)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        imgBand->at(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        imgBand->at(2)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);

        // 分别拉伸每个波段并将Float转换为unsigned char
        rBandUC = ImgSketch(rBand, imgBand->at(0), iScaleWidth * iScaleHeight, imgBand->at(0)->GetNoDataValue());
        gBandUC = ImgSketch(gBand, imgBand->at(1), iScaleWidth * iScaleHeight, imgBand->at(1)->GetNoDataValue());
        bBandUC = ImgSketch(bBand, imgBand->at(2), iScaleWidth * iScaleHeight, imgBand->at(2)->GetNoDataValue());
    }
    else
    {
        imgBand->at(0)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);

        rBandUC = ImgSketch(rBand, imgBand->at(0), iScaleWidth * iScaleHeight, imgBand->at(0)->GetNoDataValue());
        gBandUC = rBandUC;
        bBandUC = rBandUC;
    }

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight * 3];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBandUC[h * iScaleWidth + w];
        }
    }

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    // 构造图像并显示
    QGraphicsPixmapItem* imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888)));
    QGraphicsScene* myScene = new QGraphicsScene();
    myScene->addItem(imgItem);
    QGraphicsView* that = new QGraphicsView(myScene, this);

    updateActions();
    //ui.image_show->setScene(myScene);
}



unsigned char* ImageViewer::ImgSketch(float* buffer, GDALRasterBand* currentBand, int bandSize, double noValue)
{
    unsigned char* resBuffer = new unsigned char[bandSize];
    double max, min;
    double minmax[2];

    currentBand->ComputeRasterMinMax(1, minmax);
    min = minmax[0];
    max = minmax[1];
    if (min <= noValue && noValue <= max)
    {
        min = 0;
    }
    for (int i = 0; i < bandSize; i++)
    {
        if (buffer[i] > max)
        {
            resBuffer[i] = 255;
        }
        else if (buffer[i] <= max && buffer[i] >= min)
        {
            resBuffer[i] = static_cast<uchar>(255 - 255 * (max - buffer[i]) / (max - min));
        }
        else
        {
            resBuffer[i] = 0;
        }
    }

    return resBuffer;
}


//bool ImageViewer::loadFile(const QString& fileName)
//{
//    GDALAllRegister();
//    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
//    GDALDataset* poDataset;
//    QByteArray ba = fileName.toLatin1();
//    poDataset = (GDALDataset*)GDALOpen(ba.data(), GA_ReadOnly);//fileName.toStdString().c_str()
//    if (poDataset == NULL)
//    {
//        QMessageBox::critical(this, tr("Error!"), tr("Can not open file %1").arg(fileName));
//        return true;
//    }
//    //ShowFileList(fileName);
//    //ShowImgInfor(fileName);
//    // 如果图像文件并非三个波段，则默认只显示第一波段灰度图像
//    if (poDataset->GetRasterCount() != 3)
//    {
//        m_showColor = false;
//        ShowBand(poDataset->GetRasterBand(1));
//    }
//    // 如果图像正好三个波段，则默认以RGB的顺序显示彩色图
//    else
//    {
//        m_showColor = true;
//        QList<GDALRasterBand*> bandList;
//        bandList.append(poDataset->GetRasterBand(1));
//        bandList.append(poDataset->GetRasterBand(2));
//        bandList.append(poDataset->GetRasterBand(3));
//        ShowImg(&bandList);
//        //QMessageBox::critical(this, tr("Error!"), tr("open file %1").arg(fileName));
//
//    }
//    //DALClose(poDataset);
//
//    return true;
//}


bool ImageViewer::loadFile(const QString& fileName) {
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    GDALDataset* poDataset;
    QByteArray ba = fileName.toLatin1();
    poDataset = (GDALDataset*)GDALOpen(ba.data(), GA_ReadOnly);//fileName.toStdString().c_str()

    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    unsigned char* pBuf = new unsigned char[iWidth * iHeight * iBandCount];  //分配存储空间
    int panBandMap[3] = { 3,2,1 };   //如果想读取为RGB，那么将数据换成1，2，3
    poDataset->RasterIO(GF_Read, 0, 0, iWidth, iHeight, pBuf, iWidth, iHeight,
        GDT_Byte, 3, panBandMap, 3, iWidth * 3, 1);


    delete[]pBuf;
    pBuf = NULL;

    return true;
}