#include "frmmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    frmmain w;
    w.move(600, 100);
    w.show();

    return a.exec();
}
