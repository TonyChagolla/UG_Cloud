#ifndef EDITPASSWINDOW_H
#define EDITPASSWINDOW_H

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
class EditPassWindow;
}

class EditPassWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditPassWindow(QWidget *parent = nullptr);
    ~EditPassWindow();
public slots:
   QString getNewPass();
   //void setUsername(QString *username);

private slots:
    void on_btnUpdatePass_clicked();

    //void onManagerFinished(QNetworkReply *reply);



private:
    Ui::EditPassWindow *ui;
    QString newPass;
};

#endif // EDITPASSWINDOW_H
