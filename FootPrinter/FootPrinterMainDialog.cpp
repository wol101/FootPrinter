#include "FootPrinterMainDialog.h"
#include "ui_FootPrinterMainDialog.h"

#include "Preferences.h"

#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QSizePolicy>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QScreen>

#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>


FootPrinterMainDialog::FootPrinterMainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FootPrinterMainDialog)
{
    this->setWindowFlags(this->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint); //Makes the frame invisible
    this->setWindowState(Qt::WindowFullScreen); //Maximizes the window
//    this->setWindowState(Qt::WindowMaximized); //Maximizes the window
//    this->setGeometry(0, 0, 320, 240);
//    this->setFixedSize(200, 200);

    // adjust some sizes
//    QRect geometry = screen()->geometry();
//    std::cerr << geometry.x() << " " << geometry.y() << " "<< geometry.width() << " " << geometry.height() << "\n";
//    int iconSide = std::min(int(geometry.height() * 0.5), int(geometry.width() * 0.5));
//    ui->toolButtonGo->setFixedSize(QSize(iconSide, iconSide));
//    ui->toolButtonGo->setIconSize(QSize(iconSide, iconSide));
//    ui->toolButtonStop->setFixedSize(QSize(iconSide, iconSide));
//    ui->toolButtonStop->setIconSize(QSize(iconSide, iconSide));
//    ui->toolButtonShutdown->setFixedSize(QSize(iconSide, iconSide));
//    ui->toolButtonShutdown->setIconSize(QSize(iconSide, iconSide));

    readPreferences();

    // this command just disables most of the options
    checkBoxEnableAdvancedOptionsClicked();

    // always start with tab 0 (in case I have saved the form with the wrong tab showing)
    ui->tabWidget->setCurrentIndex(0);

    connect(ui->toolButtonGo, &QToolButton::clicked, this, &FootPrinterMainDialog::toolButtonGoClicked);
    connect(ui->toolButtonStop, &QToolButton::clicked, this, &FootPrinterMainDialog::toolButtonStopClicked);
    connect(ui->toolButtonShutdown, &QToolButton::clicked, this, &FootPrinterMainDialog::toolButtonShutdownClicked);
    connect(ui->pushButtonExit, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonExitClicked);
    connect(ui->pushButtonReboot, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonRebootClicked);
    connect(ui->pushButtonDataFolder, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonDataFolderClicked);
    connect(ui->pushButtonExport, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonExportClicked);
    connect(ui->pushButtonImport, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonImportClicked);
    connect(ui->pushButtonDefaults, &QPushButton::clicked, this, &FootPrinterMainDialog::pushButtonDefaultsClicked);
    connect(ui->checkBoxEnableAdvancedOptions, &QCheckBox::clicked, this, &FootPrinterMainDialog::checkBoxEnableAdvancedOptionsClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &FootPrinterMainDialog::tabWidgetCurrentChanged);

    // I2C address from "sudo i2cdetect -y 1"
    // /dev/i2c-1, 0x2a
    QString i2cDev = ui->lineEditLoadCellDevice->text();
    unsigned int i2cAdd = static_cast<unsigned int>(ui->spinBoxLoadCellAddress->value());
    if (!m_loadCell.begin(i2cDev.toStdString(), i2cAdd, true))
    {
        QString errorMessage = QString("Unable to initialise load cell\nRecommend abort.");
        auto ret = QMessageBox::critical(nullptr, "", errorMessage,
                                         QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
        if (ret != QMessageBox::Ignore) exit(EXIT_FAILURE);
    }
    m_loadCell.startReadThread();
    m_motorControl.SetNau7802(&m_loadCell);
    m_motorControl.setStepsLimit(ui->spinBoxStepsLimit->value());
    m_motorControl.setStepOnDuration(ui->spinBoxOnDuration->value());
    m_motorControl.setStepOffDuration(ui->spinBoxOffDuration->value());
    m_motorControl.setStartingOffset(ui->spinBoxStartingOffset->value());
    m_motorControl.setReadingLimit(ui->spinBoxReadingLimit->value());
    m_motorControl.setOverloadCountLimit(ui->spinBoxLimitCount->value());
    m_motorControl.SetPositionToZero();

    // USB GPS device
    // /dev/ttyACM0
    QString GPSDev = ui->lineEditGPSDevice->text();
    int status = m_gps.OpenDevice(GPSDev.toStdString());
    if (status)
    {
        QString errorMessage = QString("Unable to access gps device\nRecommend abort.");
        auto ret = QMessageBox::critical(nullptr, "", errorMessage,
                                         QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
        if (ret != QMessageBox::Ignore) exit(EXIT_FAILURE);
    }
    status = m_gps.SetMessages();
    if (status)
    {
        QString errorMessage = QString("Unable to set gps messages\nLimited functionality.");
        auto ret = QMessageBox::warning(nullptr, "", errorMessage,
                                        QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Ignore);
        if (ret != QMessageBox::Ignore) exit(EXIT_FAILURE);
    }

    QTimer *m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FootPrinterMainDialog::timerTimeout);
    m_timer->start(100);

}

FootPrinterMainDialog::~FootPrinterMainDialog()
{
    writePreferences();
    delete ui;
}

void FootPrinterMainDialog::toolButtonGoClicked()
{
    ui->tabShutdown->setEnabled(false);
    ui->tabOptions->setEnabled(false);
    ui->toolButtonGo->setEnabled(false);
    ui->toolButtonStop->setEnabled(true);
    m_motorControl.PlungeStroke();
}

void FootPrinterMainDialog::toolButtonStopClicked()
{
    m_motorControl.StopMotor();
    ui->tabShutdown->setEnabled(true);
    ui->tabOptions->setEnabled(true);
    ui->toolButtonGo->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
}

void FootPrinterMainDialog::toolButtonShutdownClicked()
{
    // for shutdown to work we need to enable shutdown for the user
    // which means the sudoers file needs to be edited
    // pi raspberrypi =NOPASSWD: /usr/sbin/reboot /usr/sbin/shutdown
    QString errorMessage = QString("Are you sure you want to power off the machine?");
    auto ret = QMessageBox::warning(nullptr, "", errorMessage,
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::No)
    {
        writePreferences();
        system("sudo shutdown -h 0");
    }
}

void FootPrinterMainDialog::pushButtonRebootClicked()
{
    // for reboot to work we need to enable reboot for the user
    // which means the sudoers file needs to be edited
    // pi raspberrypi =NOPASSWD: /usr/sbin/reboot /usr/sbin/shutdown
    QString errorMessage = QString("Are you sure you want to reboot the machine?");
    auto ret = QMessageBox::warning(nullptr, "", errorMessage,
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::No)
    {
        writePreferences();
        system("sudo shutdown -r 0");
    }
}

void FootPrinterMainDialog::pushButtonExitClicked()
{
    QString errorMessage = QString("Are you sure you want to exit the program?");
    auto ret = QMessageBox::warning(nullptr, "", errorMessage,
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::No) close();
}

void FootPrinterMainDialog::pushButtonDataFolderClicked()
{
    QFileDialog *fd = new QFileDialog;
    fd->setFileMode(QFileDialog::Directory);
    fd->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    fd->setViewMode(QFileDialog::List);
    fd->setDirectory(ui->lineEditDataFolder->text());
    // fd->setGeometry(20, 20, 320 - 40, 240 - 40);
    fd->setFixedSize(320, 240);
    // I could probably go through and hide various unnecessary elements here if I really wanted to make the best use of the space
    int result = fd->exec();
    // Shows the dialog as a modal dialog, blocking until the user closes it. The function returns a DialogCode result.
    // If the dialog is application modal, users cannot interact with any other window in the same application until they close the dialog.
    // If the dialog is window modal, only interaction with the parent window is blocked while the dialog is open. By default, the dialog is application modal.
    // Note: Avoid using this function; instead, use open(). Unlike exec(), open() is asynchronous, and does not spin an additional event loop.
    // This prevents a series of dangerous bugs from happening (e.g. deleting the dialog's parent while the dialog is open via exec()).
    // When using open() you can connect to the finished() signal of QDialog to be notified when the dialog is closed.
    QString directory;
    if (result)
    {
        QString directory = fd->selectedFiles()[0];
        ui->lineEditDataFolder->setText(directory);
    }
}

void FootPrinterMainDialog::checkBoxEnableAdvancedOptionsClicked()
{
    bool enable = ui->checkBoxEnableAdvancedOptions->isChecked();
    ui->spinBoxStepsLimit->setEnabled(enable);
    ui->spinBoxOnDuration->setEnabled(enable);
    ui->spinBoxOffDuration->setEnabled(enable);
    ui->spinBoxStartingOffset->setEnabled(enable);
    ui->spinBoxReadingLimit->setEnabled(enable);
    ui->spinBoxLimitCount->setEnabled(enable);
    ui->spinBoxLoadCellAddress->setEnabled(enable);
    ui->lineEditLoadCellDevice->setEnabled(enable);
    ui->lineEditGPSDevice->setEnabled(enable);
    ui->pushButtonExport->setEnabled((enable));
    ui->pushButtonImport->setEnabled((enable));
}

void FootPrinterMainDialog::tabWidgetCurrentChanged(int index)
{
    if (index == 3) // this means we have switched to the status tab
    {
        std::map<std::string, std::string> ipAddressList = GetLocalIDAddress();
        QString statusText = "IP Addresses:\n";
        for (auto &&ipAddress : ipAddressList)
        {
            statusText.append(QString::fromStdString(ipAddress.first + " " + ipAddress.second + "\n"));
        }
        ui->plainTextEditStatus->setPlainText(statusText);
    }
}

void FootPrinterMainDialog::readPreferences()
{
    ui->lineEditDataFolder->setText(Preferences::valueQString("DataFolder", QDir::homePath()));
    ui->spinBoxStepsLimit->setValue(Preferences::valueInt("StepsLimit", 250 * 200));
    ui->spinBoxOnDuration->setValue(Preferences::valueInt("OnDuration", 500));
    ui->spinBoxOffDuration->setValue(Preferences::valueInt("OffDuration", 500));
    ui->spinBoxStartingOffset->setValue(Preferences::valueInt("StartingOffset", 200));
    ui->spinBoxReadingLimit->setValue(Preferences::valueInt("ReadingLimit", 1000000));
    ui->spinBoxLimitCount->setValue(Preferences::valueInt("LimitCount", 10));
    ui->spinBoxLoadCellAddress->setValue(Preferences::valueInt("LoadCellAddress", 0x2A));
    ui->lineEditLoadCellDevice->setText(Preferences::valueQString("LoadCellDevice", "/dev/i2c-1"));
    ui->lineEditGPSDevice->setText(Preferences::valueQString("GPSDevice", "/dev/ttyACM0"));
}

void FootPrinterMainDialog::writePreferences()
{
    Preferences::insert("DataFolder", ui->lineEditDataFolder->text());
    Preferences::insert("StepsLimit", ui->spinBoxStepsLimit->value());
    Preferences::insert("OnDuration", ui->spinBoxOnDuration->value());
    Preferences::insert("OffDuration", ui->spinBoxOffDuration->value());
    Preferences::insert("StartingOffset", ui->spinBoxStartingOffset->value());
    Preferences::insert("ReadingLimit", ui->spinBoxReadingLimit->value());
    Preferences::insert("LimitCount", ui->spinBoxLimitCount->value());
    Preferences::insert("LoadCellAddress", ui->spinBoxLoadCellAddress->value());
    Preferences::insert("LoadCellDevice", ui->lineEditLoadCellDevice->text());
    Preferences::insert("GPSDevice", ui->lineEditGPSDevice->text());
    Preferences::Write();
}


void FootPrinterMainDialog::pushButtonExportClicked()
{
    QString lastExportedFile = Preferences::valueQString("LastExportedFile", "FootPrinterSettings.xml");
    QString fileName = QFileDialog::getSaveFileName(this, "Export Settings File", lastExportedFile, "Exported Settings Files (*.xml)", nullptr);
    if (fileName.isNull() == false)
    {
        writePreferences();
        Preferences::insert("LastExportedFile", fileName);
        Preferences::Export(fileName);
    }
}

void FootPrinterMainDialog::pushButtonImportClicked()
{
    QString lastImportedFile = Preferences::valueQString("LastImportedFile", "FootPrinterSettings.xml");
    QString fileName = QFileDialog::getOpenFileName(this, "Import Settings File", lastImportedFile, "Exported Settings Files (*.xml);;Any File (*.* *)", nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::Import(fileName);
        Preferences::insert("LastImportedFile", fileName);
        readPreferences();
    }
}

void FootPrinterMainDialog::pushButtonDefaultsClicked()
{
    QMessageBox msgBox;
    msgBox.setText("Preferences will be set to their defaults");
    // msgBox.setInformativeText("Click OK to proceed, and Cancel to abort this action.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
    {
        Preferences::LoadDefaults();
        readPreferences();
    }
}

void FootPrinterMainDialog::timerTimeout()
{
    bool motorStopped = m_motorControl.MotorStopped();
    ui->toolButtonGo->setEnabled(motorStopped);
    ui->toolButtonStop->setEnabled(!motorStopped);   
    ui->tabShutdown->setEnabled(motorStopped);
    ui->tabOptions->setEnabled(motorStopped);
    int32_t zeroOffset = m_motorControl.zeroOffset();

    int32_t stepsOffset = m_motorControl.position();
    if (m_stepsOffsetLast != stepsOffset)
    {
        m_stepsOffsetLast = stepsOffset;
        ui->labelHeight->setText(QString("%1").arg(m_stepsOffsetLast));
    }
    ui->labelLoad->setText(QString("%1").arg(m_loadCell.threadedValue() - zeroOffset));
    if (m_gps.DataReady())
    {
        m_gps.ReadData();
        m_gps.ParseNMEA();
        m_gps.ParseTimeString();
        m_gps.ParsePositionString();
        std::string timeStr = m_gps.GetLastValidTime();
        std::string positionStr = m_gps.GetLastValidPosition();
        ui->labelTime->setText(QString::fromStdString(timeStr));
        ui->labelLocation->setText(QString::fromStdString(positionStr));
    }

    while (m_motorControl.GetLoadData() && m_motorControl.GetLoadData()->size())
    {
        QDir dir(ui->lineEditDataFolder->text());
        if (!dir.exists())
        {
            if (QDir(QDir::root()).mkpath(dir.absolutePath()) == false)
            {
                QString errorMessage = QString("Unable to access the data folder:\n%1").arg(dir.absolutePath());
                QMessageBox::warning(nullptr, "", errorMessage);
                break;
            }
        }
        QString extension(".tab");
        QString filename = ui->labelTime->text() + ui->labelLocation->text();
        if (filename.size() == 0) filename = "footprint_data";
        if (dir.exists(filename + extension))
        {
            int uniqueCount = 0;
            while (true)
            {
                QString uniqueName = QString("%1%2").arg(filename).arg(uniqueCount);
                if (!dir.exists(uniqueName + extension))
                {
                    filename = uniqueName;
                    break;
                }
                uniqueCount++;
            }
        }
        QString filePath = dir.absoluteFilePath(filename + extension);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly) == false)
        {
            QString errorMessage = QString("Unable to open the file:\n%1").arg(filePath);
            QMessageBox::warning(nullptr, "", errorMessage);
            break;
        }
        for (auto &&data : *m_motorControl.GetLoadData())
        {
            QString line = QString("%1\t%2\t%3\t%4\n").arg(data.time).arg(data.position).arg(data.reading).arg(data.reading - zeroOffset);
            if (file.write(line.toUtf8()) != line.size())
            {
                QString errorMessage = QString("Unable to write to the file:\n%1").arg(filePath);
                QMessageBox::warning(nullptr, "", errorMessage);
                break;
            }
        }
        file.close();
        m_motorControl.GetLoadData()->clear();
        break;
    }
}

std::map<std::string, std::string> FootPrinterMainDialog::GetLocalIDAddress()
{
    std::map<std::string, std::string> results;
    //which family do we require , AF_INET or AF_INET6
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
    int family , s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return results;
    }

    // Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr) { continue; }
        family = ifa->ifa_addr->sa_family;
        if (family == fm)
        {
            s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                break;
            }
            printf("address: %s %s\n", ifa->ifa_name, host);
            results[ifa->ifa_name] = host;
        }
    }

    freeifaddrs(ifaddr);
    return results;
}
