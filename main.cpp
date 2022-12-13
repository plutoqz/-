#include"ImageProcessing.h"
#include "imageviewer.h"
#include <QApplication>
#include <QCommandLineParser>




int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    //QWidget w;
    //QPushButton b,c;
    ///*显示窗口*/
    //w.show();
    ///*把按钮b的父对象设定为窗口w*/
    //b.setParent(&w);
    //c.setParent(&w);
    ///*显示空间*/
    //b.show();
    ///*在按钮控件上显示PushButton*/
    //b.setText(QString::fromLocal8Bit("运行")); 
    //c.setText(QString::fromLocal8Bit("打开图片"));
    //c.move(50, 0);
    //c.show();

    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Image Viewer"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(ImageViewer::tr("[file]"), ImageViewer::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    ImageViewer imageViewer;
    if (!commandLineParser.positionalArguments().isEmpty()
        && !imageViewer.loadFile(commandLineParser.positionalArguments().front())) {
        return -1;
    }
    imageViewer.show();

    return app.exec();
}

