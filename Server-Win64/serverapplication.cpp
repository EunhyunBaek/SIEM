#include "serverapplication.h"
#include "server.h"
#include "settingswidget.h"

#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QDebug>

ServerApplication::ServerApplication(int argc, char *argv[])
    : QApplication(argc, argv)
{
    QCoreApplication::setApplicationName("Server-Win64");
    QCoreApplication::setOrganizationName("LETI");
    QCoreApplication::setOrganizationDomain("eltech.ru");
    QCoreApplication::setApplicationVersion("0.1");

    setQuitOnLastWindowClosed(false);

    // Инициализация и запуск сервера в отдельном потоке
    m_server = new Server;

    QThread *serverThread = new QThread;
    m_server->moveToThread(serverThread);

    connect(this,         SIGNAL(quitApp()),  serverThread, SLOT(quit()));
    connect(serverThread, SIGNAL(started()),  m_server,     SLOT(start()));
    connect(serverThread, SIGNAL(finished()), m_server,     SLOT(deleteLater()));
    connect(serverThread, SIGNAL(finished()), serverThread, SLOT(deleteLater()));

    serverThread->start();

    // Создание виджета настройки приложения
    m_settingsWidget = new SettingsWidget;

    // Создание контекстного меню IconTray
    m_trayIcon = new QSystemTrayIcon(QIcon(":serverIcon.png"), this);

    QMenu *menu = new QMenu;
    menu->addAction(tr("Settings"), this, SLOT(openSettings()));
    menu->addSeparator();
    menu->addAction(tr("Exit"),     this, SLOT(onQuitAction()));

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->show();

    // Сигнал выхода из приложения. Кидается по нажатию на Exit контекстного меню
    connect(this, SIGNAL(quitApp()), qApp, SLOT(quit()));

    // Коннекты на апдейт настроек
    connect(m_settingsWidget, SIGNAL(settingsChanged()), this,     SLOT(updateSettings()));
    connect(m_settingsWidget, SIGNAL(settingsChanged()), m_server, SLOT(updateSettings()));
}

ServerApplication::~ServerApplication()
{
    delete m_trayIcon;
    delete m_settingsWidget;
    qDebug() << "~ServerApplication";
}

void ServerApplication::openSettings()
{
    m_settingsWidget->show();
}

void ServerApplication::onQuitAction()
{
    emit quitApp();
}

void ServerApplication::updateSettings()
{
    QSettings settings;
    bool isStartup = settings.value("General/Startup", QVariant(false)).toBool();
    QSettings setStartup("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    QString value = ServerApplication::applicationFilePath();
    value.replace(QRegExp("/"), "\\");

    if ( isStartup )
        setStartup.setValue("Siem-Server", value);
     else
        setStartup.remove("Siem-Server");
}

