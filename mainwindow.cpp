#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logwindow.h"
#include "editpasswindow.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QObject>
#include <QUrlQuery>
#include <QNetworkCookie>
#include <QtCore>
#include <QHttpPart>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loaded(false)
{
    ui->setupUi(this);
    filePath="";
    fileName="";

    logcookies = new QNetworkCookie();
    manager = new QNetworkAccessManager(this); //Creando un Network Acces Manager para manejar los metodos
    manager->setCookieJar(new QNetworkCookieJar); //Asignando un Cookie Jar para guardar las cookies
    QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/add")); //Creando en request al link para iniciar sesión

    //Llamando la funcion para cargar la cookie al manager en caso de existir
    //La funcion loadLoginCookie() regresa un bool, si es falso quiere decir que no existe la cookie, inicia sesión y guarda la cookie
    //FALTA SOLUCIONAR EL INICIO DE SESION CUANDO LA COOKIE EXPIRA
    //SI LA COOKIE EXPIRA, BUSCAR EN DOCUMENTOS EL ARCHIVO cookie.dat Y BORRARLO, REINICIAR LA APP PARA OBTENER UNA NUEVA COOKIE
    if(!loadLoginCookie())
    {
        login();
    }
    on_btnGet_clicked();


}

MainWindow::~MainWindow()
{
    delete ui;
}

//Al presionar el boton btnRequest se hace un get a la ruta /datos
void MainWindow::on_btnRequest_clicked()
{
    disconnectManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedSend(QNetworkReply*)));
    QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/datos")); //Creando el request y asignando la url
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");//Asignando las cabeceras con el tipo de datos que esperamos recibir
    qDebug()<< "Send File boton";//Debuging para saber que boton presionamos, se muestra en consola
    sendFile();
}

//Esta funcion eta conectada a manager y se encarga de obtener la respuesta del servidor
void MainWindow::onManagerFinished(QNetworkReply *reply)
{
    data = reply->readAll();
    qDebug().noquote()<<data;

    QStringList fileNames;
    QStringList fileKeys;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonArray jsonArray = jsonResponse.array();
    QJsonObject jsonObject = jsonResponse.object();
    if(jsonResponse["eror"].toString()=="No inicio session")
    {
        QMessageBox::StandardButton answer;
        answer = QMessageBox::information(this, "Inicia sesión", "La sesión ha caducado o no has iniciado sesión.");
    }
    QAbstractItemModel *m_listModel = new QStandardItemModel(this);
    m_listModel->insertColumns(0,4);
    m_listModel->insertRows(0,jsonArray.count());
    m_listModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    m_listModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Nombre"));
    m_listModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Alt Nombre"));
    m_listModel->setHeaderData(3, Qt::Horizontal, QObject::tr("size KB"));
    int i = 0;
    foreach(const QJsonValue & value, jsonArray){
        QJsonObject obj = value.toObject();
        m_listModel->setData(m_listModel->index(i,0), obj["id_archivos"].toInt());
        m_listModel->setData(m_listModel->index(i,1), obj["nom_or"].toString());
        m_listModel->setData(m_listModel->index(i,2), obj["titulo"].toString());
        m_listModel->setData(m_listModel->index(i,3), obj["size"].toInt()/1024);
        i+=1;
    }
    ui->tableView->reset();
    ui->tableView->setModel(m_listModel);
    ui->tableView->setColumnWidth(0,this->width()/10);
    ui->tableView->setColumnWidth(1,2*this->width()/5);
    ui->tableView->setColumnWidth(2,2*this->width()/5);
    ui->tableView->setColumnWidth(3,this->width()/10);
    qDebug() <<"Finished";
    reply->deleteLater();
}

//Se manager se conesta con esta funcion solo cuando ya existe una cookie
void MainWindow::onManagerFinishedGet(QNetworkReply *reply)
{
    //Si m_loaded es falso significa que no existe la cookie y se procede a crear
    if(!m_loaded){
        QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);//La cookie se encuentra en la cabecera, almacenamos esto en cookieVar
        if(cookieVar.isValid())//Si la cabecera es valida continuamos
        {
            QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie> >();//Creamos una lista de cookies, algunos sitios tienen mas de una cookie, en este caso no pero se debe hacer así
            foreach (QNetworkCookie cookie, cookies)//Iteramos en la lista de cookies, se almacena temporalmente la cookie en el objeto cookie
                {
                    if(cookie.name() == "connect.sid") //En caso de encontrar la cookie con el nombre connect.sid quiere decir que es la cookie de la sesión y la guardamos, el resto se descarta
                    {
                       if(! saveLoginCookie(cookie))//Llamamos la funcion saveLoginCookie(cookie), el parametro es una la cookie actual
                        {
                            qDebug() << "Error al guardar la cookie";
                            return;
                        }
                       on_btnGet_clicked();

                    }
                }
         }
        else
        {
            qDebug()<<"Cookie from file";

        }
        m_loaded = true;//cambiamos m_loaded = true para saber que ya se creo la cookie y no sobreescibirla en la misma sesión
    }
    qDebug().noquote()<< reply->readAll();
    reply->deleteLater();
}

//Hacemos un get a /datos
void MainWindow::on_btnGet_clicked()
{
    disconnectManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinished(QNetworkReply*)));
    QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/datos"));
    //request.setUrl(QUrl("http://tonychagolla.herokuapp.com/api/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
     qDebug()<< "Get Button Request";
    manager->get(request);
}



// Funcion para guardar la cookie de la sesión
bool MainWindow::saveLoginCookie(const QNetworkCookie &cookie)
{
    if (m_loaded) //Si ya se cargó la cookie entonces regresa true
    {
        qDebug() <<"Cookie existente";
        return true;
    }else{
        qDebug() <<"Escribiendo Cookie";
    }

    QDir cookiesDir("./");
    if(! cookiesDir.exists())//Si la dirección existe continuamos
    {
        if(! cookiesDir.mkpath("."))//Si el directorio no existe salimos
        {
            qDebug() << "Error creating the cookies directory";
            return false;
        }
    }

    QFile cookieFile(cookiesDir.path() + "/cookie.dat"); //Establecemos el nombre del archivo junto con su directorio
    if(! cookieFile.open(QIODevice::WriteOnly))//Si no se puede abrir entonces retornamos false
    {
        qDebug() << "Error saving the cookie file:" << cookieFile.fileName();
        return false;
    }
    //Debigin para saber los datos de la cookie
    qDebug() << cookie.name();
    qDebug() << cookie.value();
    qDebug() << cookie.path();
    qDebug() << cookie.domain();
    *logcookies = cookie;
    cookieFile.write(cookie.toRawForm()); //Escribimos la cookie
    cookieFile.close();//Cerramos el archivo

    qDebug() << "Login cookie created successfully.";
    return true; //Retornamos true indicanto que se creo correctamente
}

//Está funcion carga la cookie al manager
bool MainWindow::loadLoginCookie()
{
    //Establecemos la ruta y determinamos si existe la cookie, de lo contrarió retornamos
    QDir cookiesDir("./");
    if(! cookiesDir.exists())
    {
        return false;
    }

    QFile cookieFile(cookiesDir.path() + "/cookie.dat");
    if(! cookieFile.exists())
    {
        qDebug()<<"Connected to onManagerFinished";
        disconnectManager();
        connect(manager, SIGNAL(finished(QNetworkReply*)),SLOT(onManagerFinishedGet(QNetworkReply*)));
        return false;
    }

    if(! cookieFile.open(QIODevice::ReadOnly))//abrimos el archivo y verificamos que se pueda leer
    {
        qDebug() << "Error reading the cookie file:" << cookieFile.fileName();
    }

    QByteArray cookieRaw = cookieFile.readAll();//Creamos un array de las cookies crudas que obtenemos de cookieFile
    cookieFile.close();//Cerramos el archivo
    QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(cookieRaw); //Creamos una nueva lista de cookies, hacemos un parce a las cookies crudas (ahora estan horneadas) :D

    if (newCookies.count() == 0 && cookieRaw.length() != 0)//Si no hay cookies dentro de cookies.dat entonces retornamos false
    {
        qDebug() << "CookieJar: Unable to parse saved cookie:" << cookieRaw;
        return false;
    }


    foreach (QNetworkCookie cookie, newCookies) //Iteramos en las cookies
        {
            if(cookie.name() == "connect.sid") //Buscamos la cookie "connect.sid" y si existe la asignamos al manager
            {
                manager->setCookieJar(new QNetworkCookieJar);//Asignamos un nuevo CookieJar
                qDebug() << cookie.name();//Debug para verificar el nombre de la cookie
                qDebug() << cookie.value();//Debug para verificar el valor de la cookie
                disconnectManager();
                connect(manager, SIGNAL(finished(QNetworkReply*)),SLOT(onManagerFinishedGet(QNetworkReply*)));//Conectamos la función de onManagerFinishedGet con manager
                cookie.setDomain("backcloud2019.herokuapp.com"); //***MUY IMPORTANTE*** debemos asignarle el dominio a la cookie, sin esto el manager no sabra a que dominio pertenece la cookie y no servirá de nada tenerla
                manager->cookieJar()->insertCookie(cookie);//Ponemos la cookie en el jarron. :D
                *logcookies = cookie;
                qDebug()<< cookie.domain(); //Verificamos que el dominio sea correcto
                qDebug()<<"Connected to onManagerFinished Get";//Solo para debig
            }
            else
            {
                qDebug() << "Error loading the login cookie.";//Si no se encontro la cookie entonces lo notificamos
            }
        }
    return true;//Retornamos true si se cargó la cookie
}


//Enviar Archivo
//POR EL MOMENTO SOLO SIRVE PARA ENVIAR UN ARCHIVO LLAMADO test.txt QUE DEBE ESTAR EN EL DIRECTORIO DEL DE PROYECTO
//FALTA IMPLEMENTAR LA SELECCIÓN DE ARCHIVOS
void MainWindow::sendFile(){
    if(filePath=="")
    {
        QMessageBox::information(this, "Advertencia", "Debe selecconar un archivo para subir");
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);//Los archivos se mandan mediante MultiPart
        QHttpPart imagePart; //Esto va a contener la info del archivo a enviar mediante la cabecera
        //Asignamos la cabecera form-data es el tipo de dato, name debe ser = file y filename es el nombre original del archivo.
        //ui->tbxFilePath->setText(sendFilePath);
        //return;
        imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename="+fileName+"\""));
        QHttpPart textPart; //Creamos un QHttpPart llamado textPart, este va a almacenar el archivo a enviar
        //Asignamos las cabeceras
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
        textPart.setBody("Generico");//Asignamos el body, puede ser cualquier cosa

        //QString fileLocation =  ; //Asignamos la ruta del archivo
        QFile *file = new QFile(filePath); //Creamos un objeto con el archivo
        file->open(QIODevice::ReadOnly); //Abrimos el archivo
        imagePart.setBodyDevice(file);
        file->setParent(multiPart); //

        multiPart->append(textPart);//Agregamos al multipart el archivo
        multiPart->append(imagePart);//y las cabeceras

        QNetworkRequest request(QUrl("https://backcloud2019.herokuapp.com/subir"));//Establecemos la ruta
        //qDebug()<<"Posting";
        manager->post(request, multiPart);//Hacemos un post ala ruta contenida en request y enviamos el multipar con nuestro archivo a subir
        //qDebug()<<"Success";
        multiPart->setParent(manager); // delete the multiPart with the reply
        filePath = ""; fileName="";
        ui->tbxFilePath->setText("");

        //FUNCIONES  AUN POR CREAR PARA VISUALIZAR EL PROGRESO DE LA CARGA DEL ARCHIVO
        /*
         connect(manager, SIGNAL(finished()),
                  this, SLOT  (uploadDone()));

         connect(manager, SIGNAL(uploadProgress(qint64, qint64)),
                  this, SLOT  (uploadProgress(qint64, qint64)));*/
}

void MainWindow::on_btnOpenFile_clicked()
{
    filePath = QFileDialog::getOpenFileName(this, "Cargar un archivo",QDir::homePath(), "All files (*.*)");
    QFileInfo fi(filePath);
    fileName = fi.fileName();
    ui->tbxFilePath->setText(filePath);
}

//---------------------------DOWNLOAD---------------------------
void MainWindow::on_btnDownload_clicked(){
    qDebug()<< "Download File boton";//Debuging para saber que boton presionamos, se muestra en consola
    QString file_name =fileDownload;
    qDebug()<< "Nombre de archivo a descargar: " + file_name;
    downloadFile(file_name);
}

void MainWindow::downloadFile(QString url)
{
    const QString urlSpec = "https://backcloud2019.herokuapp.com/download/"+ url;

    QUrl newUrl = urlSpec;
    QString fileName = url;
    QString downloadDirectory = QDir::homePath();
    url_download = QDir::homePath() + "/Downloads/" + url;
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
        fileName.prepend(downloadDirectory + '/');
    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("Overwrite Existing File"),
                                  tr("There already exists a file called %1%2."
                                     " Overwrite?")
                                     .arg(fileName,
                                          useDirectory
                                           ? QString()
                                           : QStringLiteral(" in the current directory")),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No)
            == QMessageBox::No) {
            return;
        }
        QFile::remove(fileName);
    }

    ui->btnDownload->setEnabled(false);
    QNetworkRequest request(urlSpec);
    disconnectManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(downloadFinished(QNetworkReply*)));
    manager->get(request);
    ui->btnDownload->setEnabled(true);
}

void MainWindow::startRequest(QUrl requestedUrl)
{
    request.setUrl(requestedUrl);
   reply = manager->get(request);

   file = new QFile(url_download);
   file->open(QIODevice::WriteOnly);
   connect(manager, SIGNAL(finished(QNetworkReply*)),
           this, SLOT(replyFinished(QNetworkReply*)));
   connect(reply,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
   disconnect(manager, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(replyFinished(QNetworkReply*)));
}

void MainWindow::downloadFinished(QNetworkReply *reply)
{
    QString urlSpec = "https://backcloud2019.herokuapp.com/download/"+fileDownload;
    QUrl url = reply->url();

    qDebug()<<url_download;
    if (reply->error()) {
        QMessageBox::information(this, "Error de descarga","Ocurrio un error al descargar el archivo.");
        qDebug()<<"failed";
        fprintf(stderr, "Download of %s failed: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));
    } else {
        if (isHttpRedirect(reply)) {
            qDebug()<<"redirected";
            fputs("Request was redirected.\n", stderr);
        } else {
            qDebug()<<"About to save";
            if (saveToDisk(url_download, reply)) {
                QMessageBox::information(this, "Descarga exitosa","El archivo se ha descargado correctaente");
                qDebug()<<"Download Success";
                printf("Download of %s succeeded (saved to %s)\n",
                       url.toEncoded().constData(), qPrintable(urlSpec));
            }
        }
    }

}

bool MainWindow::saveToDisk(const QString &filename, QIODevice *data)
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

void MainWindow::doDownload(const QString file)
{
    QNetworkRequest request(file);
    qDebug()<<"Request Done";
    manager->get(request);
}

bool MainWindow::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void MainWindow::disconnectManager()
{
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedSend(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedGet(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinished(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedDelete(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(downloadFinished(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedLogOut(QNetworkReply*)));
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedActualizar(QNetworkReply*)));
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    ui->btnDownload->setEnabled(true);
    ui->btnDelete->setEnabled(true);
    int indexRow = ui->tableView->selectionModel()->currentIndex().row();
    QString selectedItem = ui->tableView->selectionModel()->currentIndex().sibling(indexRow, 2).data().toString();
    file_id = ui->tableView->selectionModel()->currentIndex().sibling(indexRow, 0).data().toString();
    qDebug()<<"file_id: "+file_id;
    fileDownload = selectedItem;
    qDebug()<< fileDownload;
}

void MainWindow::login()
{
    request.setUrl(QUrl("https://backcloud2019.herokuapp.com/log")); //Asignamos la URL a donde haremos la request
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded"); //Establecemos la cabecera con el tipo de datos que enviamos en la peticion

    //Creamos postData, un tipo de datos que asemeja a un formulario, este se manda durante el post y contiene los datos de usuario y contraseña
    //Esto es provicional, posteriormente se iniciará sesión de forma distinta.
    QUrlQuery postData;
    qDebug()<<username;
    qDebug()<<password;
    postData.addQueryItem("user", username);//Se agrega el user y pass
    postData.addQueryItem("pass", password);
    qDebug()<<postData.queryItemValue("pass");
    qDebug()<<postData.queryItemValue("pass");
    qDebug()<<postData.toString(QUrl::FullyEncoded).toUtf8();

    manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());//Se hace un post mediante el manager a la ruta /log para iniciar sesión
}

//-----------------------------DELETE----------------------------
void MainWindow::on_btnDelete_clicked()
{
    QMessageBox::StandardButton answer;
    answer = QMessageBox::question(this, "Advertencia", "¿Deseas eliminar el archivo de forma permanente?", QMessageBox::Yes|QMessageBox::No);
      if (answer == QMessageBox::Yes) {
          disconnectManager();
          connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedDelete(QNetworkReply*)));
          QString url_delete = "http://backcloud2019.herokuapp.com/del/"+file_id;
          QNetworkRequest request(url_delete);
          QUrlQuery deleteData;
          manager->deleteResource(request);

      } else {}
}

void MainWindow::onManagerFinishedDelete(QNetworkReply *reply)
{
    QString respuesta = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(respuesta.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(jsonObject["error"].toBool())
    {
        answer = QMessageBox::information(this, "Advertencia", "Algo salio mal al borrar el archivo.");
    }
    else
    {
        answer = QMessageBox::information(this, "Éxito", "El archivo ha sido eliminado");
        file_id = "";
        fileDownload = "";
        ui->btnDelete->setEnabled(false);
        ui->btnDownload->setEnabled(false);
        on_btnGet_clicked();
    }
}
//-------------------------LOG IN / SIGN UP-----------------------------
void MainWindow::on_actionIniciar_sesi_n_triggered()
{
    LogWindow newLog;
    newLog.setModal(true);
    newLog.exec();
    username = newLog.getUserLog();
    password = newLog.getPassLog();
    m_loaded=false;
    remove("./cookie.dat");
    disconnectManager();
    if(!loadLoginCookie())
    {
        login();
    }
}

void MainWindow::on_tableView_activated(const QModelIndex &index)
{
    qDebug()<<index;
}

//--------------------------------LOG OUT-----------------------------------------

void MainWindow::on_actionCerrar_sesi_n_triggered()
{
    remove("./cookie.dat");
    disconnectManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedLogOut(QNetworkReply*)));
    QString urlLogOut = "http://backcloud2019.herokuapp.com/logout";
    QNetworkRequest request(urlLogOut);
    manager->get(request);



}

void MainWindow::onManagerFinishedLogOut(QNetworkReply *reply)
{
    QString respuesta = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(respuesta.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(jsonObject["error"].toBool())
    {
        answer = QMessageBox::information(this, "Log Out", "Algo salio mal, vuelve a intentar:");
    }
    else
    {
        answer = QMessageBox::information(this, "Log Out", "Se ha cerrado sesión");
        QAbstractItemModel *m_listModel = new QStandardItemModel(this);
        m_listModel->insertColumns(0,4);
        m_listModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
        m_listModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Nombre"));
        m_listModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Alt Nombre"));
        m_listModel->setHeaderData(3, Qt::Horizontal, QObject::tr("size KB"));
        ui->tableView->reset();
        ui->tableView->setModel(m_listModel);
    }

}


//----------------------------ACTUALIZAR-------------------------------------
void MainWindow::onManagerFinishedActualizar(QNetworkReply *reply)
{
    QString data = reply->readAll();
    qDebug().noquote()<<data;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(jsonObject["error"].toBool())//|| jsonObject["eror"].toString()=="Actualizado"
    {
        answer = QMessageBox::information(this, "Error", "Ocurrio un error al actualizar tu contraseña");
    }
    else
    {
        answer = QMessageBox::information(this, "Éxito", "Tu contraseña ha sido actualizada");

    }
}



void MainWindow::on_actionActualizar_contrase_a_triggered()
{
    if(username=="" || password =="")
    {
        QMessageBox::information(this, "Error", "No has iniciado sesión");
        return;
    }
    disconnectManager();
    EditPassWindow editPass;
    editPass.setModal(true);
    editPass.exec();
    QString newPassword = editPass.getNewPass();
    qDebug()<<password;
    qDebug()<<newPassword;
    m_loaded=false;
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(onManagerFinishedActualizar(QNetworkReply*)));
    QString urlUpdate = "http://backcloud2019.herokuapp.com/actualizar";
    QNetworkRequest request(urlUpdate);
    QUrlQuery postData;
    postData.addQueryItem("pass",  password.toUtf8());
    postData.addQueryItem("Npass", newPassword.toUtf8());
    qDebug()<<postData.queryItemValue("pass");
    qDebug()<<postData.queryItemValue("pass");
    qDebug()<<postData.toString(QUrl::FullyEncoded).toUtf8();
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    manager->put(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    remove("./cookie.dat");
    /*disconnectManager();
    if(!loadLoginCookie())
    {
        login();
    }*/
}

void MainWindow::onManagerFinishedSend(QNetworkReply *reply)
{
    QString data = reply->readAll();
    qDebug().noquote()<<data;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QMessageBox::StandardButton answer;
    if(jsonObject["error"].toBool())//
    {
        answer = QMessageBox::information(this, "Error", "Ocurrio un error al subir el archivo");
    }
    else
    {
        answer = QMessageBox::information(this, "Éxito", "El archivo se ha subido");
        on_btnGet_clicked();

    }
}
