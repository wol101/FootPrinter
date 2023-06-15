#include "FootPrinterMainDialog.h"
#include "Preferences.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    Preferences::Read();

    QApplication a(argc, argv);

    FootPrinterMainDialog w;
    w.show();

    int status = a.exec();
    return status;
}
