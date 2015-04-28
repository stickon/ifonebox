#include "Log4Qt/LogManager"
#include "Log4Qt/propertyconfigurator.h"
#include "Log4Qt/helpers/properties.h"
#include "mymainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include <QFile>
#include "AppSetting.h"
#include "AntiCrack.h"
#include <QtSingleApplication>
using namespace std;
#include "moyea_log.h"

using namespace MoyeaBased;

namespace Internal
{
    enum{
        emAppConfigError = 200,  ///< AppConfig Error
        emGUIDSame,              ///< GUID is same
        emInstallInnoMutex,      ///< Inno pkg is Installing
        emOther                  ///< other error
    };
}

int main(int argc, char *argv[])
{
    int returnValue = -1;
    try
    {
        QtSingleApplication a(argc, argv);
        if (a.isRunning()) {
            a.sendMessage("Wake up!");
            return 0;
        }
#ifdef Q_OS_MAC
        QDir dir(a.applicationDirPath());
        dir.cdUp();
        dir.cd("PlugIns");
        a.setLibraryPaths(QStringList(dir.absolutePath()));
#else //Q_OS_WIN
        QString strPluginPath = a.applicationDirPath() + QString("/plugins");
        a.addLibraryPath(strPluginPath);
#endif
        //定义编码格式
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        //QTextCodec *codec = QTextCodec::codecForName("GB2312");
        //设置和对本地文件系统读写时候的编码格式
        QTextCodec::setCodecForLocale(codec);
        //设置传给tr函数时的默认字符串编码
        QTextCodec::setCodecForTr(codec);
        //用在字符常量或者QByteArray构造QString对象时使用的一种编码方式
        QTextCodec::setCodecForCStrings(codec); 

        //产品UI日志
        Log4Qt::Properties qtlogProper;
        qtlogProper.setProperty("log4j.rootLogger", "DEBUG, A1");
        qtlogProper.setProperty("log4j.appender.A1", "org.apache.log4j.FileAppender");
        qtlogProper.setProperty("log4j.appender.A1.file", CAppSetting::instance().getUILogFilePath());
        qtlogProper.setProperty("log4j.appender.A1.layout", "org.apache.log4j.TTCCLayout");
        qtlogProper.setProperty("log4j.appender.A1.layout.DateFormat", "{yyyy-MM-dd HH:mm:ss}");
        Log4Qt::PropertyConfigurator::configure(qtlogProper);
        Log4Qt::LogManager::setHandleQtMessages(true);
        //底层日志
        QFileInfo logFileInfo(CAppSetting::instance().getLogFilePath());
        if (logFileInfo.size() > 10*1024*1024) {  //10M
            QFile::remove(CAppSetting::instance().getLogFilePath());
        }
#ifdef QT_NO_DEBUG
        LOG_SETTING_RELEASE(CAppSetting::instance().getLogFilePath().toUtf8().data());
#else
        LOG_SETTING_DEBUG(CAppSetting::instance().getLogFilePath().toUtf8().data());
#endif
        LOG_INFO("==================================================================");
        QString skinPath = CAppSetting::instance().getSkinPath();
#ifdef Q_OS_WIN
        QString winMainCss = skinPath + "skin.css";
        QFile file(winMainCss);
#else
        QString macMainCss = skinPath + "skin_mac.css";
        QFile file(macMainCss);
#endif
        if (!file.open(QIODevice::ReadOnly)) {
            Q_ASSERT(false);
        }
        QString styleSheet = file.readAll();
        QString absoluteCssPath = QDir(skinPath).absolutePath();
        styleSheet.replace("%skinpath%",absoluteCssPath,Qt::CaseInsensitive);
        a.setStyleSheet(styleSheet);
        file.close();

#ifdef Q_OS_WIN
        AntiCrack_DecryptEXE();
#endif

        MyMainWindow mainWindow;
        mainWindow.showMainwindow();

        a.setActivationWindow(&mainWindow);
        returnValue = a.exec();
    }
    catch (int e)
    {
        returnValue = e;
    }
    catch (...)
    {
    }
    return returnValue;
}
