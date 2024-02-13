#include "AudioSteganography.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AudioSteganography w;
    w.show();
    return a.exec();
}
