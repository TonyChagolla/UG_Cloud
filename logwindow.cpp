#include "logwindow.h"
#include "ui_logwindow.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);
}

LogWindow::~LogWindow()
{
    delete ui;
}

void LogWindow::on_btnLog_clicked()
{
    userLog = ui->tbxUser->text();
    passLog = ui->tbxPass->text();
    QMessageBox::StandardButton answer;
    if(userLog.count() <=2 || passLog.count()<=7)
    {
        answer = QMessageBox::information(this, "Error", "El usuario debe tener al menos 3 caracteres y la contraseña al menos 8 caracteres");
        return;
    }
    QUrlQuery postData;
    postData.addQueryItem("user", userLog);
    postData.addQueryItem("pass", passLog);

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedLog(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedSign(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedLog(QNetworkReply*)));
    QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/log"));
    //request.setUrl(QUrl("http://tonychagolla.herokuapp.com/api/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void LogWindow::on_btnSignUp_clicked()
{
    userLog = ui->tbxUserP->text();
    passLog = ui->tbxPassP->text();
    QMessageBox::StandardButton answer;
    if(userLog.count() <=2  || passLog.count()<=7)
    {
        answer = QMessageBox::information(this, "Error", "El usuario debe tener al menos 3 caracteres y la contraseña al menos 8 caracteres");
        return;
    }
    QUrlQuery postData;
    postData.addQueryItem("user",  userLog);
    postData.addQueryItem("pass", passLog);

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedLog(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedSign(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedSign(QNetworkReply*)));
    QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/add"));
    //request.setUrl(QUrl("http://tonychagolla.herokuapp.com/api/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void LogWindow::on_tbxUser_textChanged(const QString &arg1)
{
    return;
}

void LogWindow::on_tbxPass_textChanged(const QString &arg1)
{
    return;
}

void LogWindow::on_tbxUserP_textChanged(const QString &arg1)
{
    return;
}

void LogWindow::on_tbxPassP_textChanged(const QString &arg1)
{
   return;
}

void LogWindow::onManagerFinishedSign(QNetworkReply *reply)
{
    data = reply->readAll();
    qDebug().noquote()<<data;

    QStringList fileNames;
    QStringList fileKeys;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["error"].toArray();
    QMessageBox::StandardButton answer;
    if(!jsonObject["error"].toBool())
    {
        answer = QMessageBox::information(this, "Sign Up", "Su usuario ha sido creado: " + userLog);
        this->close();
    }
    else
    {
        answer = QMessageBox::information(this, "Log In", "Error, intente de nuevo");
    }

}

void LogWindow::onManagerFinishedLog(QNetworkReply *reply)
{
    data = reply->readAll();
    qDebug().noquote()<<data;

    QStringList fileNames;
    QStringList fileKeys;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(!jsonObject["error"].toBool())
    {
        answer = QMessageBox::information(this, "Log In", "Has iniciado sesión como: " + userLog);
        this->close();
    }
    else
    {
        answer = QMessageBox::information(this, "Log In", "Error, intente de nuevo");
    }


}

QString LogWindow::getUserLog()
{
    return userLog;
}

QString LogWindow::getPassLog()
{
    return passLog;
}

