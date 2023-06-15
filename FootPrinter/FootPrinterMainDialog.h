#ifndef FOOTPRINTERMAINDIALOG_H
#define FOOTPRINTERMAINDIALOG_H

#include <QDialog>

#include "NAU7802.h"
#include "GPSAccess.h"
#include "MotorControl.h"

#include <limits>

class QTimer;

namespace Ui {
class FootPrinterMainDialog;
}

class FootPrinterMainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FootPrinterMainDialog(QWidget *parent = nullptr);
    ~FootPrinterMainDialog();

    static std::map<std::string, std::string> GetLocalIDAddress();

private:
    Ui::FootPrinterMainDialog *ui;

    QTimer *m_timer = nullptr;

    NAU7802 m_loadCell;

    GPSAccess m_gps;

    MotorControl m_motorControl;

    int32_t m_stepsOffsetLast = std::numeric_limits<int32_t>::max();

    void readPreferences();
    void writePreferences();

private slots:
    void toolButtonGoClicked();
    void toolButtonStopClicked();
    void toolButtonShutdownClicked();
    void pushButtonExitClicked();
    void pushButtonRebootClicked();
    void pushButtonDataFolderClicked();
    void pushButtonImportClicked();
    void pushButtonExportClicked();
    void pushButtonDefaultsClicked();
    void checkBoxEnableAdvancedOptionsClicked();
    void tabWidgetCurrentChanged(int index);
    void timerTimeout();
};

#endif // FOOTPRINTERMAINDIALOG_H
