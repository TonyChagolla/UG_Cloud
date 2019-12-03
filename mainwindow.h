#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <string>
#include <QUrlQuery>
#include <QFile>
#include <QtCore>
#include <QtNetwork>
#include <cstdio>

void test();

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static bool isHttpRedirect(QNetworkReply *reply);
public slots:
     bool saveToDisk(const QString &filename, QIODevice *data);
     void doDownload(const QString file);

private slots:
    void on_btnRequest_clicked();
    void onManagerFinished(QNetworkReply *);
    void onManagerFinishedGet(QNetworkReply *reply);
    void on_btnGet_clicked();
    bool saveLoginCookie(const QNetworkCookie &cookie);
    bool loadLoginCookie();
    void sendFile();



    void on_btnOpenFile_clicked();

    void on_btnDownload_clicked();

    //Prueba
    void downloadFile(QString url);
    void startRequest(QUrl requestedUrl);
    void downloadFinished(QNetworkReply *reply);

    void on_tableView_clicked(const QModelIndex &index);

    void on_tableView_activated(const QModelIndex &index);

    void on_btnDelete_clicked();

    void onManagerFinishedDelete(QNetworkReply *reply);

    void on_actionIniciar_sesi_n_triggered();

    void on_actionCerrar_sesi_n_triggered();

    void onManagerFinishedLogOut(QNetworkReply *reply);

private:
    void disconnectManager();
    void login();
    Ui::MainWindow *ui;
    QUrl urlsearch, urlcookie;
    QNetworkAccessManager *manager;
    //QNetworkCookieJar *logcookies;
    QNetworkRequest request;
    QNetworkCookie *logcookies;
    bool m_loaded;
    QString filePath, fileName, data, fileDownload;
    QString username, password;

    //Prueba
    QString url_download;
    QFile *file;
    QNetworkReply *reply;

};
#endif // MAINWINDOW_H
