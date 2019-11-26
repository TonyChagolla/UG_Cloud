#include "downloadmanager.h"
#include "mainwindow.h"

downloadManager::downloadManager(QNetworkCookie *logCookie)
{
    manager = new QNetworkAccessManager;
    manager->setCookieJar(new QNetworkCookieJar);
    manager->cookieJar()->insertCookie(*logCookie);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));
}
downloadManager::~downloadManager()
{
}

void downloadManager::doDownload(const QString file)
{
    QNetworkRequest request(file);
    //QNetworkReply *reply = manager->get(request);
    qDebug()<<"Request Done";

//#if QT_CONFIG(ssl)
    //connect(manager, SIGNAL(sslErrors(QList<QSslError>)),
     //       SLOT(sslErrors(QList<QSslError>)));
    manager->get(request);
//#endif

    //currentDownloads.append(reply);
}

QString downloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    qDebug()<<path;
    QString basename = QFileInfo(path).fileName();
    qDebug()<<basename;
    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

bool downloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();
    qDebug() << "succes saving file";
    qDebug() << filename;
    return true;
}

bool downloadManager::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void downloadManager::execute(QString urlDownload)
{

    /*
    QStringList args = QCoreApplication::instance()->arguments();
    args.takeFirst();           // skip the first argument, which is the program's name
    if (args.isEmpty()) {
        printf("Qt Download example - downloads all URLs in parallel\n"
               "Usage: download url1 [url2... urlN]\n"
               "\n"
               "Downloads the URLs passed in the command-line to the local directory\n"
               "If the target file already exists, a .0, .1, .2, etc. is appended to\n"
               "differentiate.\n");
        QCoreApplication::instance()->quit();
        return;
    }
    */
    //for (const QString &arg : qAsConst(args)) {
        //QUrl url = QUrl::fromEncoded(arg.toLocal8Bit());
        doDownload(urlDownload);
    //}
}

void downloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void downloadManager::downloadFinished(QNetworkReply *reply)
{
    qDebug()<<"URL";
    QUrl url = reply->url();

    qDebug()<<url;
    if (reply->error()) {
        qDebug()<<"failed";
        fprintf(stderr, "Download of %s failed: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));
    } else {
        if (isHttpRedirect(reply)) {
            qDebug()<<"redirected";
            fputs("Request was redirected.\n", stderr);
        } else {
            QString filename = saveFileName(url);
            if (saveToDisk(filename, reply)) {
                qDebug()<<"DownloadSuccess";
                printf("Download of %s succeeded (saved to %s)\n",
                       url.toEncoded().constData(), qPrintable(filename));
            }
        }
    }

}

