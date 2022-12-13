#include"ImageProcessing.h"
#include "imageviewer.h"
#include <QApplication>
#include <QCommandLineParser>




int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    //QWidget w;
    //QPushButton b,c;
    ///*��ʾ����*/
    //w.show();
    ///*�Ѱ�ťb�ĸ������趨Ϊ����w*/
    //b.setParent(&w);
    //c.setParent(&w);
    ///*��ʾ�ռ�*/
    //b.show();
    ///*�ڰ�ť�ؼ�����ʾPushButton*/
    //b.setText(QString::fromLocal8Bit("����")); 
    //c.setText(QString::fromLocal8Bit("��ͼƬ"));
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

