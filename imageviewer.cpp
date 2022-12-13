#include <QtWidgets>
#include <QtPrintSupport/QPrintDialog>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#include<QPushButton>
#endif
#endif

#include "imageviewer.h"
//#include "ImageProcessing.h"

extern QImage everywherepic;
unsigned char* rBandUC, * gBandUC, * bBandUC;
GDALDataset* poDataset;
unsigned char* given;
std::map< std::pair<int, int>, int> vec;

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
    //QFileDialog dialog(this, tr("Open File"));
    //initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
    //while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
    //打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, tr("open a file."), "C:/Users/Lenovo/Pictures/", tr("images(*)"));
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Warning!", "Failed to open the image!");
    }
    loadFile(fileName);
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
    savepictureAct = fileMenu->addAction(tr("&savepicture"), this, &ImageViewer::savepicture);

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

    QMenu* processMenu = menuBar()->addMenu(tr("&Process"));
    MorphDilateAct = processMenu->addAction(tr("&MorphDilate"), this, &ImageViewer::MorphDilate);
    MorphErosionAct = processMenu->addAction(tr("&MorphErosion"), this, &ImageViewer::MorphErosion);
    MorphOpenAct = processMenu->addAction(tr("&MorphOpen"), this, &ImageViewer::MorphOpen);
    MorphCloseAct = processMenu->addAction(tr("&MorphClose"), this, &ImageViewer::MorphClose);
    doHistogramMatchAct = processMenu->addAction(tr("&doHistogramMatch"), this, &ImageViewer::doHistogramMatch);
    FeatureExtractionAct= processMenu->addAction(tr("&FeatureExtraction"), this, &ImageViewer::FeatureExtraction);
    LinearStretchAct = processMenu->addAction(tr("&LinearStretch"), this, &ImageViewer::LinearStretch);
    SiftAct = processMenu->addAction(tr("&SIFT"), this, &ImageViewer::SIFT);
    MoravecAct= processMenu->addAction(tr("&Moravec"), this, &ImageViewer::Moravec);
    HoughAct= processMenu->addAction(tr("&Hough"), this, &ImageViewer::Hough);
    //MorphOpenAct->setEnabled(false);

    QMenu* geometric_correctionMenu = menuBar()->addMenu(tr("&geometric_correction"));
    geometric_correctionAct = geometric_correctionMenu->addAction(tr("&do_geometric_correction"), this, &ImageViewer::geometric_correction);

    
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


bool ImageViewer::loadFile(const QString& fileName) {
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
    //GDALDataset* poDataset;
    QByteArray ba = QTextCodec::codecForName("utf-8")->fromUnicode(fileName);
        //fileName.toLatin1();
    poDataset = (GDALDataset*)GDALOpen(ba.data(), GA_ReadOnly);//fileName.toStdString().c_str()
    if (poDataset == NULL)
    {
        QMessageBox::critical(this, tr("Error!"), tr("Can not open file %1").arg(fileName));
        return false;
    }
    //ShowImgInfor(fileName, poDataset);
    int iWidth = poDataset->GetRasterXSize();
    int iHeight= poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    //GDALRasterBand* poBand = poDataset->GetRasterBand(1);
    //unsigned char* pBuf = new unsigned char[iWidth * iHeight * iBandCount];  //分配存储空间
    //int panBandMap[3] = { 1,2,3 };   //如果想读取为RGB，那么将数据换成1，2，3
    //poDataset->RasterIO(GF_Read, 0, 0, iWidth, iHeight, pBuf, iWidth, iHeight, poBand->GetRasterDataType(), iBandCount, panBandMap, 0,static_cast<GSpacing>(iWidth) * 3, GDALGetDataTypeSize(poBand->GetRasterDataType()) / 8);// static_cast<GSpacing>(iWidth) * 3, GDALGetDataTypeSize(GDT_Byte) / 8

    int bytePerLine = (iWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iHeight * 3];
    float* rBand = new float[iWidth * iHeight];
    float* gBand = new float[iWidth * iHeight];
    float* bBand = new float[iWidth * iHeight];
    //unsigned char* rBandUC, * gBandUC, * bBandUC;
    QList<GDALRasterBand*> bandList;

    if (iBandCount >= 3) {
        //QList<GDALRasterBand*>* imgBand = &bandList;
        bandList.append(poDataset->GetRasterBand(1));
        bandList.append(poDataset->GetRasterBand(2));
        bandList.append(poDataset->GetRasterBand(3));
        bandList[0]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, rBand, iWidth, iHeight, GDT_Float32, 0, 0);
        bandList[1]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, gBand, iWidth, iHeight, GDT_Float32, 0, 0);
        bandList[2]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, bBand, iWidth, iHeight, GDT_Float32, 0, 0);
        rBandUC = ImgSketch(rBand, bandList[0], iWidth * iHeight, bandList[0]->GetNoDataValue());
        gBandUC = ImgSketch(gBand, bandList[1], iWidth * iHeight, bandList[1]->GetNoDataValue());
        bBandUC = ImgSketch(bBand, bandList[2], iWidth * iHeight, bandList[2]->GetNoDataValue());
    }
    else {
        bandList.append(poDataset->GetRasterBand(1));
        bandList[0]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, rBand, iWidth, iHeight, GDT_Float32, 0, 0);
        rBandUC = ImgSketch(rBand, bandList[0], iWidth * iHeight, bandList[0]->GetNoDataValue());
        //LinearStretch(rBandUC);
        //HistogramMatch("C:/Users/Lenovo/Pictures/Saved Pictures/OIP-C.jpeg", rBandUC);
        gBandUC = rBandUC;
        bBandUC = rBandUC;
    }
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
        }
    }
    given = allBandUC;
    QImage Ima(allBandUC, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888
    
    imageLabel->setPixmap(QPixmap::fromImage(Ima));
    //imageLabel->setScaledContents(true);
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();
    fitToWindow();
    ShowImgInfor(fileName, poDataset);
    //delete[]allBandUC;

    return true;
}

//图片信息
void ImageViewer::ShowImgInfor(const QString filename, GDALDataset* podataset)
{
    QString a0 = QString::fromLocal8Bit("您已成功打开图片") + filename;
    QString a1 = podataset->GetDriver()->GetDescription();//图片格式
    QString a2= podataset->GDALDataset::GetProjectionRef();//投影信息
    QString a3 = QString::number(podataset->GDALDataset::GetRasterXSize());//图像宽度
    QString a4 = QString::number(podataset->GDALDataset::GetRasterYSize());//图像高度
    QString a = a0 + '\n' + QString::fromLocal8Bit("图片格式:   ") + a1 + '\n' + QString::fromLocal8Bit("投影信息:   ") + a2 + '\n'
        + QString::fromLocal8Bit("图像宽度:   ") + a3 + '\n' + QString::fromLocal8Bit("图像高度:   ") + a4;
    QMessageBox::information(this, tr("About this Image"),a);
}

//线性拉伸
bool ImageViewer::LinearStretch() {
    unsigned char* line=rBandUC;
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iHeight * 3];
    bool bRet = false;
    double lownumx = QInputDialog::getInt(this, QString::fromLocal8Bit("lownumx"),
        QString::fromLocal8Bit("输入"),40 , 1, 254, 1, &bRet);
    double highnumx = QInputDialog::getInt(this, QString::fromLocal8Bit("highnumx"),
        QString::fromLocal8Bit("输入"), 200, 1, 254, 1, &bRet);
    double lownumy = QInputDialog::getInt(this, QString::fromLocal8Bit("lownumy"),
        QString::fromLocal8Bit("输入"), 20, 1, 254, 1, &bRet);
    double highnumy = QInputDialog::getInt(this, QString::fromLocal8Bit("highnumy"),
        QString::fromLocal8Bit("输入"), 240, 1, 254, 1, &bRet);
    //int lownumx = 20, highnumx = 240;
    //int lownumy = 40, highnumy = 200;
    if (lownumx < 0 || highnumx>255|| lownumy < 0 || highnumy>255) {
        return false;
    }
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++) {
            if ( line[h * iWidth + w] < lownumx) {
                //line[h * iWidth + w] = (highnum - lownum) * (line[h * iWidth + w] / 255) + lownum;
                line[h * iWidth + w] = line[h * iWidth + w] * (lownumy / lownumx);
            }
            else if(lownumx<=line[h * iWidth + w] <= highnumx) {
                //line[h * iWidth + w]= (line[h * iWidth + w] / 255)
                line[h * iWidth + w] = (line[h * iWidth + w] - lownumx) * (highnumy - lownumy) / (highnumx - lownumx) + lownumy;
            }
            else {
                line[h * iWidth + w] = (line[h * iWidth + w] - highnumx) * (255 - highnumy) / (255 - highnumx) + highnumy;
            }
        }
    }
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
        }
    }
    given = allBandUC;
    QImage Ima(allBandUC, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    //QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
    //    "C:/Users/Lenovo/Pictures/Saved Pictures/saved.jpg",
    //    tr("Images (*jpg)"));
    ////QFile file(fileName+"saved.jpg");
    ////file.open(QFile::WriteOnly);
    //Ima.save(fileName);


    imageLabel->setPixmap(QPixmap::fromImage(Ima));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();
    return true;
}

//直方图匹配图版本
void ImageViewer::HistogramMatch(const QString& fileName, unsigned char* line) {
    double nums1[256]={0}, nums2[256]={0};
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int bytePerLine = (iWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iHeight * 3];

    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++) {
            nums1[line[h * iWidth + w]]++;
        }
    }
    for (int i = 0; i < 256; ++i) {
        nums1[i] = 255 * nums1[i] / (iWidth * iHeight);
    }
    for (int i = 1; i < 256;++i) {
        nums1[i] = nums1[i - 1] + nums1[i];
    }
    QImage image = QImage(fileName);//"C:/Users/Lenovo/Pictures/Saved Pictures/OIP-C.jpeg"
    unsigned char* data = image.bits();
    int width = image.width();
    int hight = image.height();
    for (int i = 0; i < hight; i++)
    {
        for (int j = 0; j < width; j++)
        {
            nums2[data[i * hight + j]]++;
        }
    }
    for (int i = 0; i < 256; ++i) {
        nums2[i] = 255 * nums2[i] / (width * hight);
    }
    for (int i = 1; i < 256; ++i) {
        nums2[i] = nums2[i - 1] + nums2[i];
    }

    std::map<int, int> temp;
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            if (int(nums2[j]) == int(nums1[i])) {
                //temp.insert(i, j);
                temp[i] = j;
                j = 256;
            }
        }
    }

    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            line[h * iWidth + w] = temp[line[h * iWidth + w]];
        }
    }

    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
        }
    }
    given = allBandUC;
    QImage Ima(allBandUC, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima));

    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();

}
void ImageViewer::doHistogramMatch() {
    //打开文件选择对话框
     QString fileName = QFileDialog::getOpenFileName(this, tr("open a file."), "C:/Users/Lenovo/Pictures/", tr("images(*)"));
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Warning!", "Failed to open the image!");
    }
    HistogramMatch(fileName, rBandUC);
}


//直方图匹配数组版本
void ImageViewer::HistogramMatch(double nums2[], unsigned char* line, GDALDataset* poDataset) {
    double nums1[256] = { 0 };
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++) {
            nums1[line[h * iWidth + w]]++;
        }
    }
    for (int i = 0; i < 256; ++i) {
        nums1[i] = 255 * nums1[i] / (iWidth * iHeight);
    }
    for (int i = 1; i < 256; ++i) {
        nums1[i] = nums1[i - 1] + nums1[i];
    }

    std::map<int, int> temp;
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            if (int(nums2[j]) == int(nums1[i])) {
                //temp.insert(i, j);
                temp[i] = j;
                j = 256;
            }
        }
    }

    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            line[h * iWidth + w] = temp[line[h * iWidth + w]];
        }
    }

}

//保存图片
void ImageViewer::savepicture() {
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;
    QImage Ima(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
        "C:/Users/Lenovo/Pictures/Saved Pictures/saved.jpg",
        tr("Images (*jpg)"));
    //QFile file(fileName+"saved.jpg");
    //file.open(QFile::WriteOnly);
    Ima.save(fileName);
}


//开运算
void ImageViewer::MorphOpen() {
    MorphErosion();
    MorphDilate();
}

//闭运算
void ImageViewer::MorphClose() {
    MorphDilate();
    MorphErosion();
}

//膨胀
void ImageViewer::MorphDilate() {
    int struct_ele[3][3] = { 1 };
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

    auto f = [bytePerLine](int a, int b)->unsigned char {
        int max = 0;
        for (int i = a - 1; i < a + 2; ++i) {
            for (int j = b - 1; j < b + 2; ++j) {
               //max = given[i * bytePerLine + j * 3 + 0] > max ? given[i * bytePerLine + j * 3 + 0] : max;
                if (given[i * bytePerLine + j * 3 + 0] > max)max = given[i * bytePerLine + j * 3 + 0];
            }
        }
        return max;
    };
    
    unsigned char* temp = new unsigned char[bytePerLine * iHeight * 3];

    for (int h = 1; h < iHeight-1; h++)
    {
        for (int w = 1; w < iWidth-1; w++)
        {
            temp[h * bytePerLine + w * 3 + 0] = f(h,w);
            temp[h * bytePerLine + w * 3 + 1] = temp[h * bytePerLine + w * 3 + 0];
            temp[h * bytePerLine + w * 3 + 2] = temp[h * bytePerLine + w * 3 + 0];
        }
    }
    given = temp;

    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();

    //delete[]given;
}

//腐蚀
void ImageViewer::MorphErosion() {
    int struct_ele[3][3] = { 1 };
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

    auto f = [bytePerLine](int a, int b)->unsigned char {
        int max = 255;
        for (int i = a - 1; i < a + 2; ++i) {
            for (int j = b - 1; j < b + 2; ++j) {
                //max = given[i * bytePerLine + j * 3 + 0] > max ? given[i * bytePerLine + j * 3 + 0] : max;
                if (given[i * bytePerLine + j * 3 + 0] < max)max = given[i * bytePerLine + j * 3 + 0];
            }
        }
        return max;
    };

    unsigned char* temp = new unsigned char[bytePerLine * iHeight * 3];

    for (int h = 1; h < iHeight - 1; h++)
    {
        for (int w = 1; w < iWidth - 1; w++)
        {
            temp[h * bytePerLine + w * 3 + 0] = f(h, w);
            temp[h * bytePerLine + w * 3 + 1] = temp[h * bytePerLine + w * 3 + 0];
            temp[h * bytePerLine + w * 3 + 2] = temp[h * bytePerLine + w * 3 + 0];
        }
    }
    given = temp;

    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();
}

//特征点提取
void ImageViewer::FeatureExtraction(){
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

    unsigned char* temp = new unsigned char[bytePerLine * iHeight * 3];


    auto f = [bytePerLine](int a,int b)->unsigned char {
        int gx = 0, gy = 0;
        gx = given[(a + 1) * bytePerLine + (b - 1) * 3 + 0] + 2 * given[(a + 1) * bytePerLine + b * 3 + 0] + given[(a + 1) * bytePerLine + (b + 1) * 3 + 0] -
            given[(a - 1) * bytePerLine + (b - 1) * 3 + 0] - 2 * given[(a - 1) * bytePerLine + b * 3 + 0] - given[(a - 1) * bytePerLine + (b + 1) * 3 + 0];
        gy = given[(a - 1) * bytePerLine + (b - 1) * 3 + 0] + 2 * given[a * bytePerLine + (b - 1) * 3 + 0] + given[(a + 1) * bytePerLine + (b - 1) * 3 + 0] -
            given[(a - 1) * bytePerLine + (b + 1) * 3 + 0] - 2 * given[a * bytePerLine + (b + 1) * 3 + 0] - given[(a + 1) * bytePerLine + (b + 1) * 3 + 0];
        return (abs(gx) + abs(gy)) > 300 ? 255 : 0;
    };

    for (int h = 1; h < iHeight - 1; h++)
    {
        for (int w = 1; w < iWidth - 1; w++)
        {
            temp[h * bytePerLine + w * 3 + 0] = f(h, w);
            temp[h * bytePerLine + w * 3 + 1] = temp[h * bytePerLine + w * 3 + 0];
            temp[h * bytePerLine + w * 3 + 2] = temp[h * bytePerLine + w * 3 + 0];
        }
    }
    given = temp;

    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();
}

//sift
void ImageViewer::SIFT() {
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

    //unsigned char* temp = new unsigned char[bytePerLine * iHeight * 3];

    //高斯滤波,建立高斯金字塔
    unsigned char* firstgroup[4];
    for (int i = 0; i < 4; ++i) {
        firstgroup[i] = new unsigned char[bytePerLine * iHeight * 3];
    }
    float derta = 1.6;
    float k = pow(2, 0.5);
    double gaosi[3][3] = { 0 };
    double weight = 0;
    //for (int i = 0; i < 3; ++i) {
    //    for (int j = 0; j < 3; ++j) {
    //        gaosi[i][j] = (1 / (2 * 3.14 * derta * derta)) * pow(2.71, -(pow(i - 1, 2) + pow(j - 1, 2)) / (2 * derta * derta));
    //        weight += gaosi[i][j];
    //    }
    //}
    //for (int i = 0; i < 3; ++i) {
    //    for (int j = 0; j < 3; ++j) {
    //        gaosi[i][j] /= weight;
    //    }
    //}
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            firstgroup[0][h * bytePerLine + w * 3 + 0] = given[h * bytePerLine + w * 3 + 0];
            firstgroup[0][h * bytePerLine + w * 3 + 1] = given[h * bytePerLine + w * 3 + 1];
            firstgroup[0][h * bytePerLine + w * 3 + 2] = given[h * bytePerLine + w * 3 + 2];
        }
    }
    for (int i = 1; i < 4; ++i) {
        derta = derta * pow(k, i - 1);
        for (int n = 0; n < 3; ++n) {
            for (int j = 0; j < 3; ++j) {
                gaosi[n][j] = (1 / (2 * 3.14 * derta * derta)) * pow(2.71, -(pow(i - 1, 2) + pow(j - 1, 2)) / (2 * derta * derta));
                weight += gaosi[n][j];
            }
        }
        for (int n = 0; n < 3; ++n) {
            for (int j = 0; j < 3; ++j) {
                gaosi[n][j] /= weight;
            }
        }
        auto f = [bytePerLine, gaosi](int a, int b, int c)->unsigned char {
            double result;
            result = given[(a - 1) * bytePerLine + (b - 1) * 3 + c] * gaosi[0][0] + given[(a - 1) * bytePerLine + b * 3 + c] * gaosi[0][1] +
                given[(a - 1) * bytePerLine + (b + 1) * 3 + c] * gaosi[0][2] + given[a * bytePerLine + (b - 1) * 3 + c] * gaosi[1][0] +
                given[a * bytePerLine + b * 3 + c] * gaosi[1][1] + given[a * bytePerLine + (b + 1) * 3 + c] * gaosi[1][2] + given[(a + 1) * bytePerLine + (b - 1) * 3 + c] * gaosi[2][0] +
                given[(a + 1) * bytePerLine + b * 3 + c] * gaosi[2][1] + given[(a + 1) * bytePerLine + (b + 1) * 3 + c] * gaosi[2][2];
            return result;
        };
        for (int h = 1; h < iHeight-1 ; h++)
        {
            for (int w = 1; w < iWidth-1 ; w++)
            {
                firstgroup[i][h * bytePerLine + w * 3 + 0] = f(h, w, 0);
                firstgroup[i][h * bytePerLine + w * 3 + 1] = f(h, w, 1);
                firstgroup[i][h * bytePerLine + w * 3 + 2] = f(h, w, 2);
            }
        }
        given = firstgroup[i];
    }
    //given = firstgroup[1];

    //降采样，构建高斯金字塔
    int iWidth2 = iWidth / 2;
    int iHeight2 = iHeight / 2;
    int bytePerLine2 = (iWidth2 * 24 + 31) / 8;
    unsigned char* secondgroup[4];
    for (int i = 0; i < 4; ++i) {
        secondgroup[i] = new unsigned char[bytePerLine2 * iHeight2 * 3];
    }
    for (int h = 0; h < iHeight2 ; h++)
    {
        for (int w = 0; w < iWidth2 ; w++)
        {
            secondgroup[0][h * bytePerLine2 + w * 3 + 0] = firstgroup[1][2 * h * bytePerLine + 2 * w * 3 + 0];
            secondgroup[0][h * bytePerLine2 + w * 3 + 1] = firstgroup[1][2 * h * bytePerLine + 2 * w * 3 + 1];
            secondgroup[0][h * bytePerLine2 + w * 3 + 2] = firstgroup[1][2 * h * bytePerLine + 2 * w * 3 + 2];
        }
    }//降采样得到第二组第一个
    given = secondgroup[0];
    for (int i = 1; i < 4; ++i) {
        derta = derta * pow(k, i - 1);
        for (int n = 0; n < 3; ++n) {
            for (int j = 0; j < 3; ++j) {
                gaosi[n][j] = (1 / (2 * 3.14 * derta * derta)) * pow(2.71, -(pow(i - 1, 2) + pow(j - 1, 2)) / (2 * derta * derta));
                weight += gaosi[n][j];
            }
        }
        for (int n = 0; n < 3; ++n) {
            for (int j = 0; j < 3; ++j) {
                gaosi[n][j] /= weight;
            }
        }
        auto f = [bytePerLine2, gaosi](int a, int b, int c)->unsigned char {
            double result;
            result = given[(a - 1) * bytePerLine2 + (b - 1) * 3 + c] * gaosi[0][0] + given[(a - 1) * bytePerLine2 + b * 3 + c] * gaosi[0][1] +
                given[(a - 1) * bytePerLine2 + (b + 1) * 3 + c] * gaosi[0][2] + given[a * bytePerLine2 + (b - 1) * 3 + c] * gaosi[1][0] +
                given[a * bytePerLine2 + b * 3 + c] * gaosi[1][1] + given[a * bytePerLine2 + (b + 1) * 3 + c] * gaosi[1][2] + given[(a + 1) * bytePerLine2 + (b - 1) * 3 + c] * gaosi[2][0] +
                given[(a + 1) * bytePerLine2 + b * 3 + c] * gaosi[2][1] + given[(a + 1) * bytePerLine2 + (b + 1) * 3 + c] * gaosi[2][2];
            return result;
        };
        for (int h = 1; h < iHeight2 - 1; h++)
        {
            for (int w = 1; w < iWidth2 - 1; w++)
            {
                secondgroup[i][h * bytePerLine2 + w * 3 + 0] = f(h, w, 0);
                secondgroup[i][h * bytePerLine2 + w * 3 + 1] = f(h, w, 1);
                secondgroup[i][h * bytePerLine2 + w * 3 + 2] = f(h, w, 2);
            }
        }
        given = secondgroup[i];
    }

    //生成dog金字塔
    unsigned char* doggroup[3];
    for (int i = 0; i < 3; ++i) {
        doggroup[i] = new unsigned char[bytePerLine * iHeight * 3];
    }
    for (int i = 1; i < 4; ++i) {
        for (int h = 0; h < iHeight; h++)
        {
            for (int w = 0; w < iWidth; w++)
            {
                doggroup[i - 1][h * bytePerLine + w * 3 + 0] = (firstgroup[i][h * bytePerLine + w * 3 + 0] == firstgroup[i - 1][h * bytePerLine + w * 3 + 0]) ? 255 : 0;
                //doggroup[i - 1][h * bytePerLine + w * 3 + 1] = (firstgroup[i][h * bytePerLine + w * 3 + 1] == firstgroup[i - 1][h * bytePerLine + w * 3 + 1]) ? 255 : 0;
                //doggroup[i - 1][h * bytePerLine + w * 3 + 2] = (firstgroup[i][h * bytePerLine + w * 3 + 2] == firstgroup[i - 1][h * bytePerLine + w * 3 + 2]) ? 255 : 0;
                doggroup[i - 1][h * bytePerLine + w * 3 + 1] = doggroup[i - 1][h * bytePerLine + w * 3 + 0];
                doggroup[i - 1][h * bytePerLine + w * 3 + 2] = doggroup[i - 1][h * bytePerLine + w * 3 + 0];
                //doggroup[i - 1][h * bytePerLine + w * 3 + 1] = firstgroup[i][h * bytePerLine + w * 3 + 1] - firstgroup[i-1][h * bytePerLine + w * 3 + 1];
                //doggroup[i - 1][h * bytePerLine + w * 3 + 2] = firstgroup[i][h * bytePerLine + w * 3 + 2] - firstgroup[i-1][h * bytePerLine + w * 3 + 2];
            }
        }
    }
    given = doggroup[1];

    //空间极值点检测
    std::vector<std::pair<int, int>> toppoints;
    for (int h = 1; h < iHeight-1; h++)
    {
        for (int w = 1; w < iWidth-1; w++)
        {
            int temp[27] = 
            { doggroup[1][h * bytePerLine + w * 3 + 0] ,doggroup[1][(h - 1) * bytePerLine + (w - 1) * 3 + 0] ,doggroup[1][h * bytePerLine + (w - 1) * 3 + 0] ,
                doggroup[1][(h + 1) * bytePerLine + (w - 1) * 3 + 0], doggroup[1][(h - 1) * bytePerLine + w * 3 + 0],doggroup[1][(h + 1) * bytePerLine + w * 3 + 0] ,
            doggroup[1][(h - 1) * bytePerLine + (w + 1) * 3 + 0] ,doggroup[1][h * bytePerLine + (w + 1) * 3 + 0] ,doggroup[1][(h + 1) * bytePerLine + (w + 1) * 3 + 0] ,

             doggroup[0][h * bytePerLine + w * 3 + 0] ,doggroup[0][(h - 1) * bytePerLine + (w - 1) * 3 + 0] ,doggroup[0][h * bytePerLine + (w - 1) * 3 + 0] ,
                doggroup[0][(h + 1) * bytePerLine + (w - 1) * 3 + 0], doggroup[0][(h - 1) * bytePerLine + w * 3 + 0],doggroup[0][(h + 1) * bytePerLine + w * 3 + 0] ,
            doggroup[0][(h - 1) * bytePerLine + (w + 1) * 3 + 0] ,doggroup[0][h * bytePerLine + (w + 1) * 3 + 0] ,doggroup[0][(h + 1) * bytePerLine + (w + 1) * 3 + 0],

            doggroup[2][h * bytePerLine + w * 3 + 0] ,doggroup[2][(h - 1) * bytePerLine + (w - 1) * 3 + 0] ,doggroup[2][h * bytePerLine + (w - 1) * 3 + 0] ,
                doggroup[2][(h + 1) * bytePerLine + (w - 1) * 3 + 0], doggroup[2][(h - 1) * bytePerLine + w * 3 + 0],doggroup[2][(h + 1) * bytePerLine + w * 3 + 0] ,
            doggroup[2][(h - 1) * bytePerLine + (w + 1) * 3 + 0] ,doggroup[2][h * bytePerLine + (w + 1) * 3 + 0] ,doggroup[2][(h + 1) * bytePerLine + (w + 1) * 3 + 0] };
            std::sort(temp, temp + 27);
            if ((temp[0] == doggroup[1][h * bytePerLine + w * 3 + 0]&&temp[0]<temp[1]) || (temp[26] == doggroup[1][h * bytePerLine + w * 3 + 0] && temp[26] > temp[25])) {
                toppoints.push_back(std::make_pair(h, w));
            }
        }
    }
    if (toppoints.empty()) {
        QMessageBox::warning(this, "Warning!","toppoints is empty!");
    }

    int* toppointang = new int[50];
    for (auto&& value : toppoints) {
        
        int a = value.first;
        int b = value.second;
        int angles[18] = { 0 };
        for (int i = a - 1; i < a + 2; ++i) {
            for (int j = b - 1; j < b + 2; ++j) {
                float ang;
                if ((doggroup[1][(i + 1) * bytePerLine + j * 3 + 0] - doggroup[1][(i - 1) * bytePerLine + j * 3 + 0]) != 0) {
                    double temp = (doggroup[1][i * bytePerLine + (j + 1) * 3 + 0] - doggroup[1][i * bytePerLine + (j - 1) * 3 + 0]) /
                        (doggroup[1][(i + 1) * bytePerLine + j * 3 + 0] - doggroup[1][(i - 1) * bytePerLine + j * 3 + 0]);
                    ang = atan(temp) * 180 / 3.14;
                }
                else ang = 90;
                int x = (ang + 90) / 10;
                angles[x] += sqrt(pow(doggroup[1][(i + 1) * bytePerLine + j * 3 + 0] - doggroup[1][(i - 1) * bytePerLine + j * 3 + 0], 2) +
                    pow(doggroup[1][i * bytePerLine + (j + 1) * 3 + 0] - doggroup[1][i * bytePerLine + (j - 1) * 3 + 0], 2));
            }
        }
        std::sort(angles, angles + 18);   
        //std::vector<std::pair<int, int>>
        //std::make_pair(value, angles[17]);
        vec.insert(std::make_pair(value, angles[17]));
    }//获取描述向量

    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888
    //QImage Ima2(secondgroup[0], iWidth2, iHeight2, bytePerLine2, QImage::Format_RGB888);//QImage::Format_RGB888
    //Ima2.save("C:/Users/Lenovo/Pictures/IMG_0.jpg");
    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();
}


int ImageViewer::ResampleGDAL(const char* pszSrcFile, const char* pszOutFile, float fResX, float fResY, GDALResampleAlg eResample)
{

    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    GDALDataset* pDSrc = (GDALDataset*)GDALOpen(pszSrcFile, GA_ReadOnly);
    if (pDSrc == NULL)
    {
        return -1;
    }


    GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (pDriver == NULL)
    {
        GDALClose((GDALDatasetH)pDSrc);
        return -2;
    }
    int width = pDSrc->GetRasterXSize();
    int height = pDSrc->GetRasterYSize();
    int nBandCount = pDSrc->GetRasterCount();
    GDALDataType dataType = pDSrc->GetRasterBand(1)->GetRasterDataType();


    char* pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char*>(pDSrc->GetProjectionRef());


    double dGeoTrans[6] = { 0 };
    int nNewWidth = width, nNewHeight = height;
    pDSrc->GetGeoTransform(dGeoTrans);


    bool bNoGeoRef = false;
    double dOldGeoTrans0 = dGeoTrans[0];
    //如果没有投影，人为设置一个  
    if (strlen(pszSrcWKT) <= 0)
    {
        //OGRSpatialReference oSRS;
        //oSRS.SetUTM(50,true);	//北半球  东经120度
        //oSRS.SetWellKnownGeogCS("WGS84");
        //oSRS.exportToWkt(&pszSrcWKT);
        //pDSrc->SetProjection(pszSrcWKT);


        //
        dGeoTrans[0] = 1.0;
        pDSrc->SetGeoTransform(dGeoTrans);
        //


        bNoGeoRef = true;
    }


    //adfGeoTransform[0] /* top left x */
    //adfGeoTransform[1] /* w-e pixel resolution */
    //adfGeoTransform[2] /* rotation, 0 if image is "north up" */
    //adfGeoTransform[3] /* top left y */
    //adfGeoTransform[4] /* rotation, 0 if image is "north up" */
    //adfGeoTransform[5] /* n-s pixel resolution */


    dGeoTrans[1] = dGeoTrans[1] / fResX;
    dGeoTrans[5] = dGeoTrans[5] / fResY;
    nNewWidth = static_cast<int>(nNewWidth * fResX + 0.5);
    nNewHeight = static_cast<int>(nNewHeight * fResY + 0.5);


    //创建结果数据集
    GDALDataset* pDDst = pDriver->Create(pszOutFile, nNewWidth, nNewHeight, nBandCount, dataType, NULL);
    if (pDDst == NULL)
    {
        GDALClose((GDALDatasetH)pDSrc);
        return -2;
    }

    pDDst->SetProjection(pszSrcWKT);
    pDDst->SetGeoTransform(dGeoTrans);


    void* hTransformArg = NULL;
    hTransformArg = GDALCreateGenImgProjTransformer2((GDALDatasetH)pDSrc, (GDALDatasetH)pDDst, NULL); //GDALCreateGenImgProjTransformer((GDALDatasetH) pDSrc,pszSrcWKT,(GDALDatasetH) pDDst,pszSrcWKT,FALSE,0.0,1);


    if (hTransformArg == NULL)
    {
        GDALClose((GDALDatasetH)pDSrc);
        GDALClose((GDALDatasetH)pDDst);
        return -3;
    }

    GDALWarpOptions* psWo = GDALCreateWarpOptions();


    psWo->papszWarpOptions = CSLDuplicate(NULL);
    psWo->eWorkingDataType = dataType;
    psWo->eResampleAlg = eResample;


    psWo->hSrcDS = (GDALDatasetH)pDSrc;
    psWo->hDstDS = (GDALDatasetH)pDDst;


    psWo->pfnTransformer = GDALGenImgProjTransform;
    psWo->pTransformerArg = hTransformArg;


    psWo->nBandCount = nBandCount;
    psWo->panSrcBands = (int*)CPLMalloc(nBandCount * sizeof(int));
    psWo->panDstBands = (int*)CPLMalloc(nBandCount * sizeof(int));
    for (int i = 0; i < nBandCount; i++)
    {
        psWo->panSrcBands[i] = i + 1;
        psWo->panDstBands[i] = i + 1;
    }


    GDALWarpOperation oWo;
    if (oWo.Initialize(psWo) != CE_None)
    {
        GDALClose((GDALDatasetH)pDSrc);
        GDALClose((GDALDatasetH)pDDst);
        return -3;
    }


    oWo.ChunkAndWarpImage(0, 0, nNewWidth, nNewHeight);


    GDALDestroyGenImgProjTransformer(hTransformArg);
    GDALDestroyWarpOptions(psWo);
    if (bNoGeoRef)
    {
        dGeoTrans[0] = dOldGeoTrans0;
        pDDst->SetGeoTransform(dGeoTrans);
        //pDDst->SetProjection("");
    }
    GDALFlushCache(pDDst);


    //ShowImgInfor(fileName, poDataset);
    int iWidth = pDDst->GetRasterXSize();
    int iHeight = pDDst->GetRasterYSize();
    int iBandCount = pDDst->GetRasterCount();
    //GDALRasterBand* poBand = poDataset->GetRasterBand(1);
    //unsigned char* pBuf = new unsigned char[iWidth * iHeight * iBandCount];  //分配存储空间
    //int panBandMap[3] = { 1,2,3 };   //如果想读取为RGB，那么将数据换成1，2，3
    //poDataset->RasterIO(GF_Read, 0, 0, iWidth, iHeight, pBuf, iWidth, iHeight, poBand->GetRasterDataType(), iBandCount, panBandMap, 0,static_cast<GSpacing>(iWidth) * 3, GDALGetDataTypeSize(poBand->GetRasterDataType()) / 8);// static_cast<GSpacing>(iWidth) * 3, GDALGetDataTypeSize(GDT_Byte) / 8

    int bytePerLine = (iWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iHeight * 3];
    float* rBand = new float[iWidth * iHeight];
    float* gBand = new float[iWidth * iHeight];
    float* bBand = new float[iWidth * iHeight];
    //unsigned char* rBandUC, * gBandUC, * bBandUC;
    QList<GDALRasterBand*> bandList;

    if (iBandCount == 3) {
        //QList<GDALRasterBand*>* imgBand = &bandList;
        bandList.append(pDDst->GetRasterBand(1));
        bandList.append(pDDst->GetRasterBand(2));
        bandList.append(pDDst->GetRasterBand(3));
        bandList[0]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, rBand, iWidth, iHeight, GDT_Float32, 0, 0);
        bandList[1]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, gBand, iWidth, iHeight, GDT_Float32, 0, 0);
        bandList[2]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, bBand, iWidth, iHeight, GDT_Float32, 0, 0);
        rBandUC = ImgSketch(rBand, bandList[0], iWidth * iHeight, bandList[0]->GetNoDataValue());
        gBandUC = ImgSketch(gBand, bandList[1], iWidth * iHeight, bandList[1]->GetNoDataValue());
        bBandUC = ImgSketch(bBand, bandList[2], iWidth * iHeight, bandList[2]->GetNoDataValue());
    }
    else {
        bandList.append(pDDst->GetRasterBand(1));
        bandList[0]->RasterIO(GF_Read, 0, 0, iWidth, iHeight, rBand, iWidth, iHeight, GDT_Float32, 0, 0);
        rBandUC = ImgSketch(rBand, bandList[0], iWidth * iHeight, bandList[0]->GetNoDataValue());
        //LinearStretch(rBandUC);
        //HistogramMatch("C:/Users/Lenovo/Pictures/Saved Pictures/OIP-C.jpeg", rBandUC);
        gBandUC = rBandUC;
        bBandUC = rBandUC;
    }
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 0] = rBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 1] = gBandUC[h * iWidth + w];
            //given[h * bytePerLine + w * 3 + 2] = bBandUC[h * iWidth + w];
        }
    }
    given = allBandUC;
    QImage Ima(allBandUC, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima));
    //imageLabel->setScaledContents(true);
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();
    fitToWindow();


    GDALClose((GDALDatasetH)pDSrc);
    GDALClose((GDALDatasetH)pDDst);


    return 0;
}

//几何校正大题
void ImageViewer::geometric_correction(){

    //特征点提取
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("open a file."),
        "C:/Users/Lenovo/Pictures/Saved Pictures/",
        tr("images(*)"));
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Warning!", "Failed to open the video!");
    }
    loadFile(fileName);
    SIFT();
    std::map< std::pair<int, int>, int> vec2 = vec;
    vec.clear();
    
    QString fileName2 = QFileDialog::getOpenFileName(
        this,
        tr("open a file."),
        "C:/Users/Lenovo/Pictures/Saved Pictures/",
        tr("images(*)"));
    if (fileName2.isEmpty()) {
        QMessageBox::warning(this, "Warning!", "Failed to open the video!");
    }
    loadFile(fileName2);
    SIFT();
    std::map< std::pair<int, int>, int> vec3 = vec;



    const char* pszInFile = "C:/Users/Lenovo/Pictures/Saved Pictures/testright.jpg";
    const char* pszOutFile = "C:/Users/Lenovo/Pictures/Saved Pictures/testright (2).jpg";
    //ProcessGCP(pszInFile, pszOutFile, nGCPCount, pGCPs, pszWkt1, 0, 0, 0, GRA_NearestNeighbour, "GTiff");
    ResampleGDAL(pszInFile, pszOutFile, 1, 1, GRA_NearestNeighbour);

}

//int ProcessGCP(const char* pszSrcFile,const char* pszDstFile,int nGCPCount,const GDAL_GCP* pasGCPList,const char* pszDstWKT,int iOrder,double dResX,double dResY,GDALResampleAlg eResampleMethod,const char* pszFormat)
//{
//
//    GDALAllRegister();
//    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
//
//    // 打开原始图像
//    GDALDatasetH hSrcDS = GDALOpen(pszSrcFile, GA_ReadOnly);
//    if (NULL == hSrcDS)
//    {
//        return 0;
//    }
//
//    GDALDataType eDataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS, 1));
//    int nBandCount = GDALGetRasterCount(hSrcDS);
//
//    // 创建几何多项式坐标转换关系
//    void* hTransform = GDALCreateGCPTransformer(nGCPCount, pasGCPList, iOrder, FALSE);
//    if (NULL == hTransform)
//    {
//        GDALClose(hSrcDS);
//        return 0;
//    }
//
//    // 计算输出图像四至范围、大小、仿射变换六参数等信息
//    double adfGeoTransform[6] = { 0 };
//    double adfExtent[4] = { 0 };
//    int    nPixels = 0, nLines = 0;
//
//    if (GDALSuggestedWarpOutput2(hSrcDS, GDALGCPTransform, hTransform,
//        adfGeoTransform, &nPixels, &nLines, adfExtent, 0) != CE_None)
//    {
//        GDALClose(hSrcDS);
//        return 0;
//    }
//
//    // 下面开始根据用户指定的分辨率来反算输出图像的大小和六参数等信息
//    double dResXSize = dResX;
//    double dResYSize = dResY;
//
//    //如果为0，则默认为原始影像的分辨率
//    if (dResXSize == 0.0 && dResYSize == 0.0)
//    {
//        double dbGeoTran[6] = { 0 };
//        GDALGCPsToGeoTransform(nGCPCount, pasGCPList, dbGeoTran, 0);
//        dResXSize = fabs(dbGeoTran[1]);
//        dResYSize = fabs(dbGeoTran[5]);
//    }
//
//    // 如果用户指定了输出图像的分辨率
//    else if (dResXSize != 0.0 || dResYSize != 0.0)
//    {
//        if (dResXSize == 0.0) dResXSize = adfGeoTransform[1];
//        if (dResYSize == 0.0) dResYSize = adfGeoTransform[5];
//    }
//
//    if (dResXSize < 0.0) dResXSize = -dResXSize;
//    if (dResYSize > 0.0) dResYSize = -dResYSize;
//
//    // 计算输出图像的范围
//    double minX = adfGeoTransform[0];
//    double maxX = adfGeoTransform[0] + adfGeoTransform[1] * nPixels;
//    double maxY = adfGeoTransform[3];
//    double minY = adfGeoTransform[3] + adfGeoTransform[5] * nLines;
//
//    nPixels = ceil((maxX - minX) / dResXSize);
//    nLines = ceil((minY - maxY) / dResYSize);
//    adfGeoTransform[0] = minX;
//    adfGeoTransform[3] = maxY;
//    adfGeoTransform[1] = dResXSize;
//    adfGeoTransform[5] = dResYSize;
//
//    // 创建输出图像
//    GDALDriverH hDriver = GDALGetDriverByName(pszFormat);
//    if (NULL == hDriver)
//    {
//        return 0;
//    }
//    GDALDatasetH hDstDS = GDALCreate(hDriver, pszDstFile, nPixels, nLines, nBandCount, eDataType, NULL);
//    if (NULL == hDstDS)
//    {
//        return 0;
//    }
//    GDALSetProjection(hDstDS, pszDstWKT);
//    GDALSetGeoTransform(hDstDS, adfGeoTransform);
//
//    //获得原始图像的行数和列数
//    int nXsize = GDALGetRasterXSize(hSrcDS);
//    int nYsize = GDALGetRasterYSize(hSrcDS);
//
//    //然后是图像重采样
//    int nFlag = 0;
//    float dfValue = 0;
//    CPLErr err = CE_Failure;
//    //最邻近采样
//    for (int nBandIndex = 0; nBandIndex < nBandCount; nBandIndex++)
//    {
//        GDALRasterBandH hSrcBand = GDALGetRasterBand(hSrcDS, nBandIndex + 1);
//        GDALRasterBandH hDstBand = GDALGetRasterBand(hDstDS, nBandIndex + 1);
//        for (int nRow = 0; nRow < nLines; nRow++)
//        {
//            for (int nCol = 0; nCol < nPixels; nCol++)
//            {
//                double dbX = adfGeoTransform[0] + nCol * adfGeoTransform[1]
//                    + nRow * adfGeoTransform[2];
//                double dbY = adfGeoTransform[3] + nCol * adfGeoTransform[4]
//                    + nRow * adfGeoTransform[5];
//
//                //由输出的图像地理坐标系变换到原始的像素坐标系
//                GDALGCPTransform(hTransform, TRUE, 1, &dbX, &dbY, NULL, &nFlag);
//                int nXCol = (int)(dbX + 0.5);
//                int nYRow = (int)(dbY + 0.5);
//
//                //超出范围的用0填充
//                if (nXCol < 0 || nXCol >= nXsize || nYRow < 0 || nYRow >= nYsize)
//                {
//                    dfValue = 0;
//                }
//
//                else
//                {
//                    err = GDALRasterIO(hSrcBand, GF_Read, nXCol, nYRow, 1, 1, &dfValue, 1, 1, eDataType, 0, 0);
//
//                }
//                err = GDALRasterIO(hDstBand, GF_Write, nCol, nRow, 1, 1, &dfValue, 1, 1, eDataType, 0, 0);
//
//            }
//        }
//
//    }
//
//
//
//    if (hTransform != NULL)
//    {
//        GDALDestroyGCPTransformer(hTransform);
//        hTransform = NULL;
//    }
//
//    GDALClose(hSrcDS);
//    GDALClose(hDstDS);
//
//
//    return 1;
//}

//摄影测量学Moravec
void ImageViewer::Moravec() {
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

   int* temp = new int[bytePerLine * iHeight * 3];
    unsigned char* temp2 = new unsigned char[bytePerLine * iHeight * 3];
    //temp2 = { 0 };
    //temp = { 0 };

    //QDialog dialog(this);
    //QFormLayout form(&dialog);
    //form.addRow(new QLabel("User input:"));
    //// Value1
    //QString value1 = QString("Value1: ");
    //QSpinBox* spinbox1 = new QSpinBox(&dialog);
    //form.addRow(value1, spinbox1);
    //// Value2
    //QString value2 = QString("Value2: ");
    //QSpinBox* spinbox2 = new QSpinBox(&dialog);
    //form.addRow(value2, spinbox2);
    //// Add Cancel and OK button
    //QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
    //    Qt::Horizontal, &dialog);
    //form.addRow(&buttonBox);
    //QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    //QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    ////
    //// Process when OK button is clicked
    //if (dialog.exec() == QDialog::Accepted) {
    //    // Do something here
    //}

    bool bRet = false;
    double windowsize = QInputDialog::getInt(this, QString::fromLocal8Bit("窗口大小"),
        QString::fromLocal8Bit("输入"), 3, 3, 9, 2, &bRet);//默认5，最小3，最大7
    int threshold = 5000;
    int ws = windowsize / 2;
    auto f = [bytePerLine,ws, threshold](int a, int b)->unsigned char {
        int num[4] = { 0,0,0,0 };
        //|
        for (int i = -ws; i < ws+1; ++i) {
            num[0] += pow(given[(a + i) * bytePerLine + b * 3 + 0] - given[(a + i + 1) * bytePerLine + b * 3 + 0], 2);
        }
        //一
        for (int i = -ws; i < ws+1; ++i) {
            num[1] += pow(given[a * bytePerLine + (b + i) * 3 + 0] - given[a * bytePerLine + (b + i + 1) * 3 + 0], 2);
        }
        ///
        for (int i = -ws; i < ws+1; ++i) {
            for (int j = ws; j > -ws-1; --j) {
                num[2] += pow(given[(a + i) * bytePerLine + (b + j) * 3 + 0] - given[(a + i + 1) * bytePerLine + (b + j + 1) * 3 + 0], 2);
            }
        }
        //\.
        for (int i = -ws; i < ws+1; ++i) {
            for (int j = ws; j > -ws-1; --j) {
                num[3] += pow(given[(a + j) * bytePerLine + (b + i) * 3 + 0] - given[(a + j + 1) * bytePerLine + (b + i + 1) * 3 + 0], 2);
            }
        }
        std::sort(num,num+4);
        return (num[0]> threshold)?num[0]:0;
    };
    for (int h = ws; h < iHeight - ws; h++)
    {
        for (int w = ws; w < iWidth - ws; w++)
        {
            temp[h * bytePerLine + w * 3 + 0] = f(h, w);
            temp[h * bytePerLine + w * 3 + 1] = temp[h * bytePerLine + w * 3 + 0];
            temp[h * bytePerLine + w * 3 + 2] = temp[h * bytePerLine + w * 3 + 0];
        }
    }
    //for (int h = 2; h < iHeight - 2; h++)
    //{
    //    for (int w = 2; w < iWidth - 2; w++)
    //    {
    //        int max = 0;
    //        int tnum[25];
    //        for (int i = -2; i < 3; ++i) {
    //            for (int j = -2; j < 3; ++j) {
    //                //max = (temp[(h+i) * bytePerLine + (w+j) * 3 + 0] > max) ? temp[(h + i) * bytePerLine + (w + j) * 3 + 0] : max;
    //                tnum[max] = temp[(h + i) * bytePerLine + (w + j) * 3 + 0];
    //                ++max;
    //            }
    //        }//(temp[h * bytePerLine + w * 3 + 0] > tnum[23])?255:0
    //        std::sort(tnum, tnum + 25);
    //        temp2[h * bytePerLine + w * 3 + 0] = (temp[h * bytePerLine + w * 3 + 0] < tnum[24]) ? 0 : ((temp[h * bytePerLine + w * 3 + 0] > tnum[23]) ? 255 : 0);
    //        temp2[h * bytePerLine + w * 3 + 1] = temp2[h * bytePerLine + w * 3 + 0];
    //        temp2[h * bytePerLine + w * 3 + 2] = temp2[h * bytePerLine + w * 3 + 0];
    //        if (temp2[h * bytePerLine + w * 3 + 0] == 255) {
    //            given[h * bytePerLine + w * 3 + 0] = 255;
    //            given[h * bytePerLine + w * 3 + 1] = 0;
    //            given[h * bytePerLine + w * 3 + 2] = 0;
    //        }
    //    }
    //}
    
    //for (int h = 3; h < iHeight - 3; h++)
    //{
    //    for (int w = 3; w < iWidth - 3; w++)
    //    {
    //        int max = 0;
    //        int tnum[49];
    //        for (int i = -3; i < 4; ++i) {
    //            for (int j = -3; j < 4; ++j) {
    //                //max = (temp[(h+i) * bytePerLine + (w+j) * 3 + 0] > max) ? temp[(h + i) * bytePerLine + (w + j) * 3 + 0] : max;
    //                tnum[max] = temp[(h + i) * bytePerLine + (w + j) * 3 + 0];
    //                ++max;
    //            }
    //        }//(temp[h * bytePerLine + w * 3 + 0] > tnum[23])?255:0
    //        std::sort(tnum, tnum + 49);
    //        temp2[h * bytePerLine + w * 3 + 0] = (temp[h * bytePerLine + w * 3 + 0] < tnum[48]) ? 0 : ((temp[h * bytePerLine + w * 3 + 0] > tnum[47]) ? 255 : 0);
    //        temp2[h * bytePerLine + w * 3 + 1] = temp2[h * bytePerLine + w * 3 + 0];
    //        temp2[h * bytePerLine + w * 3 + 2] = temp2[h * bytePerLine + w * 3 + 0];
    //        if (temp2[h * bytePerLine + w * 3 + 0] == 255) {
    //            given[h * bytePerLine + w * 3 + 0] = 255;
    //            given[h * bytePerLine + w * 3 + 1] = 0;
    //            given[h * bytePerLine + w * 3 + 2] = 0;
    //        }
    //    }
    //}

    for (int h = 1; h < iHeight - 1; h++)
    {
        for (int w = 1; w < iWidth - 1; w++)
        {
            int max = 0;
            int tnum[9];
            for (int i = -1; i < 2; ++i) {
                for (int j = -1; j < 2; ++j) {
                    //max = (temp[(h+i) * bytePerLine + (w+j) * 3 + 0] > max) ? temp[(h + i) * bytePerLine + (w + j) * 3 + 0] : max;
                    tnum[max] = temp[(h + i) * bytePerLine + (w + j) * 3 + 0];
                    ++max;
                }
            }//(temp[h * bytePerLine + w * 3 + 0] > tnum[23])?255:0
            std::sort(tnum, tnum + 9);
            temp2[h * bytePerLine + w * 3 + 0] = (temp[h * bytePerLine + w * 3 + 0] < tnum[8]) ? 0 : ((temp[h * bytePerLine + w * 3 + 0] > tnum[7]) ? 255 : 0);
            temp2[h * bytePerLine + w * 3 + 1] = temp2[h * bytePerLine + w * 3 + 0];
            temp2[h * bytePerLine + w * 3 + 2] = temp2[h * bytePerLine + w * 3 + 0];
            if (temp2[h * bytePerLine + w * 3 + 0] == 255) {
                given[h * bytePerLine + w * 3 + 0] = 255;
                given[h * bytePerLine + w * 3 + 1] = 0;
                given[h * bytePerLine + w * 3 + 2] = 0;
            }
        }
    }

    //given = temp2;

    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888

    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();
}


void ImageViewer::Hough() {
    int iWidth = poDataset->GetRasterXSize();
    int iHeight = poDataset->GetRasterYSize();
    int iBandCount = poDataset->GetRasterCount();
    int bytePerLine = (iWidth * 24 + 31) / 8;

    unsigned char* temp = new unsigned char[bytePerLine * iHeight * 3];
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            temp[h * bytePerLine + w * 3 + 0] = given[h * bytePerLine + w * 3 + 0];
            temp[h * bytePerLine + w * 3 + 1] = given[h * bytePerLine + w * 3 + 1];
            temp[h * bytePerLine + w * 3 + 2] = given[h * bytePerLine + w * 3 + 2];
        }
    }

    std::map<double, int> count;
    for (int h = 0; h < iHeight; h++)
    {
        for (int w = 0; w < iWidth; w++)
        {
            if (temp[h * bytePerLine + w * 3 + 0] == 255) {
                for (int i = -180; i < 180; ++i) {
                    double tempnum = h * cos(i * 3.14 / 180) + w * sin(i * 3.14 / 180);
                    if (tempnum > 0)tempnum = 1 * 100000000 + (i + 180) * 100000 + tempnum;
                    else tempnum = 2 * 100000000 + (i + 180) * 100000 - tempnum;
                    //std::vector<int, int> tempvec;
                    //tempvec.push_back(i); tempvec.push_back(tempnum);
                    if (count.end() != count.find(tempnum)) count[tempnum]++;
                    else count.insert(std::pair<double, int>(tempnum, 1));
                }
            }
        }
    }

    for (auto it : count) {
        if (it.second > 10) {
            double p; int angle;
            angle = it.first;

            if (it.first > 200000000) {
                p = -((angle % 100000000) % 100000);
            }
            else p = (angle % 100000000) % 100000;
            angle = (angle % 100000000) / 100000 - 180;
            //painter.drawLine(0, p / sin(angle * 3.14 / 180), iHeight, p / sin(angle * 3.14 / 180) - iHeight / tan(angle * 3.14 / 180));
            for (int h = 0; h < iHeight; h++) {
                int w = p / sin(angle * 3.14 / 180) - h / tan(angle * 3.14 / 180);
                temp[h * bytePerLine + w * 3 + 0] = 255;
                temp[h * bytePerLine + w * 3 + 1] = 0;
                temp[h * bytePerLine + w * 3 + 2] = 0;
            }
        }
    }


    given = temp;
    QImage Ima2(given, iWidth, iHeight, bytePerLine, QImage::Format_RGB888);//QImage::Format_RGB888
    imageLabel->setPixmap(QPixmap::fromImage(Ima2));
    QPainter painter(this);


    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    fitToWindow();
    updateActions();

}

