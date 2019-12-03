#include "editpasswindow.h"
#include "ui_editpasswindow.h"

EditPassWindow::EditPassWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditPassWindow)
{
    ui->setupUi(this);
}

EditPassWindow::~EditPassWindow()
{
    delete ui;
}

void EditPassWindow::on_btnUpdatePass_clicked()
{
    newPass = ui->tbxPassP->text();
    QMessageBox::StandardButton answer;
    if(newPass.count() <=7)
    {
        answer = QMessageBox::information(this, "Error", "La contraseña debe tener al menos 8 caracteres");
        return;
    }
    if(newPass != ui->tbxPassP_2->text())
    {
        answer = QMessageBox::information(this, "Error", "Las contraseñas no coinciden.");
        return;
    }
    this->close();
}

/*void EditPassWindow::onManagerFinishedActualizar(QNetworkReply *reply)
{
    QString data = reply->readAll();
    qDebug().noquote()<<data;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(!jsonObject["error"].toBool() & jsonObject["eror"].toString()=="Actualizado")
    {
        answer = QMessageBox::information(this, "Éxito", "Tu contraseña ha sido actualizada");
        this->close();
    }
    else
    {
        answer = QMessageBox::information(this, "Error", "Ocurrio un error al actualizar tu contraseña");
    }
}*/

QString EditPassWindow::getNewPass()
{
    return newPass;
}
/*
void EditPassWindow::setUsername(QString *username)
{
    userName = username;
}
*/
