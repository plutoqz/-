#pragma once
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include<map>
#include<gdal_priv.h>
#include <QMainWindow>
#include <QImage>

#ifndef QT_NO_PRINTER
#include <QtPrintSupport/QPrinter>
#endif

#include<vector>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "gdalwarper.h"
#include<algorithm>
#include <stack>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE



//! [0]
class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString&);//
    //QImage QImg;
private slots:
    void open();
    void saveAs();
    void print();
    void copy();
    //void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

    bool LinearStretch();//线性拉伸
    void HistogramMatch(const QString& fileName, unsigned char* line);//直方图匹配picture
    void doHistogramMatch();//执行直方图匹配picture
    void HistogramMatch(double nums2[], unsigned char* line, GDALDataset* poDataset);//直方图匹配nums[]
    //void doHistogramMatch2();//执行直方图匹配nums[]
    void MorphOpen();//开运算
    void MorphClose();//闭运算
    void MorphDilate();//膨胀
    void MorphErosion();//腐蚀
    void FeatureExtraction();//特征点提取
    void SIFT();//sift特征点提取
    void geometric_correction();//几何校正大题
    void savepicture();//
    void Moravec();//
    void Hough();//

    //void calculate(STACK bands, std::stack<char>& symbols);

    int ResampleGDAL(const char* pszSrcFile, const char* pszOutFile, float fResX, float fResY, GDALResampleAlg eResample);

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString& fileName);
    void setImage(const QImage& newImage);//
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);
/*    void ShowBand(GDALRasterBand* band);
    void ShowImg(QList<GDALRasterBand*>* imgBand)*/;

    unsigned char* ImgSketch(float* buffer, GDALRasterBand* currentBand, int size, double noValue);
    void ShowImgInfor(const QString filename, GDALDataset* podataset);

    float m_scaleFactor;
    bool m_showColor;

    //Ui::open_imageClass ui;
    GDALDataset* poDataset;
    QImage image;
    QLabel* imageLabel;
    QScrollArea* scrollArea;
    double scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction* saveAsAct;
    QAction* printAct;
    QAction* copyAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* normalSizeAct;
    QAction* fitToWindowAct;

    QAction* MorphDilateAct;
    QAction* MorphErosionAct;
    QAction* MorphOpenAct;
    QAction* MorphCloseAct;
    QAction* doHistogramMatchAct;
    QAction* FeatureExtractionAct;
    QAction* SiftAct;
    QAction* geometric_correctionAct;
    QAction* LinearStretchAct;
    QAction* savepictureAct;
    QAction* MoravecAct;
    QAction* HoughAct;
};
//! [0]





#endif