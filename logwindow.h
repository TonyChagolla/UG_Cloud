#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QMainWindow>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QObject>
#include <QtCore>

namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    ~LogWindow();
public slots:
    QString getUserLog();
    QString getPassLog();

private slots:
    void on_btnLog_clicked();

    void on_btnSignUp_clicked();

    void on_tbxUser_textChanged(const QString &arg1);

    void on_tbxPass_textChanged(const QString &arg1);

    void on_tbxUserP_textChanged(const QString &arg1);

    void on_tbxPassP_textChanged(const QString &arg1);

    void onManagerFinishedLog(QNetworkReply *reply);
    void onManagerFinishedSign(QNetworkReply *reply);


private:
    Ui::LogWindow *ui;
     QString userLog, passLog, userSign, passSign, data;
};

#endif // LOGWINDOW_H
