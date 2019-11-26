#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H


#include <QtCore>
#include <QtNetwork>
#include <cstdio>
#include <QMainWindow>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

using namespace std;

class downloadManager: public QObject
{

    Q_OBJECT
    QNetworkAccessManager *manager;
    QVector<QNetworkReply *> currentDownloads;

public:
    explicit downloadManager(QNetworkCookie *logCookie);
    ~downloadManager();
    void doDownload(const QString file);
    static QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);
    static bool isHttpRedirect(QNetworkReply *reply);

public slots:
    void execute(QString urlDownload);
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
};

#endif // DOWNLOADMANAGER_H
