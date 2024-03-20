#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget) //конструктор
{
    ui->setupUi(this);
    setWindowTitle("Server");
    ui -> inputEdit -> setPlaceholderText("Введите сообщение...");
    ui -> nameEdit -> setPlaceholderText("Server");

    ui->pushButton_Send->setEnabled(false);

    server = new QTcpServer();
    server -> setMaxPendingConnections(1);
    connect(server,&QTcpServer::newConnection,this,&Widget::server_New_Connect);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
    timer -> start(3000);
}


void Widget::keyPressEvent(QKeyEvent *event) //обработка нажатия return
{
    if ((event -> key() == Qt::Key_Return) and send)
        on_pushButton_Send_clicked();
}

void Widget::on_colorButton_clicked() //выбор цвета
{
    colorSendMessage = QColorDialog::getColor(QColor(Qt::white),this,"Выберите цвет сообщений");
    ui -> colorButton ->setStyleSheet("color: " + colorSendMessage.name() + ";");
}


Widget::~Widget() //деструктор
{
    if(isClientConnected) {
        QString text = userName + " отключился";

        // Отправляем обычное сообщение
        DataToSend m_data;
        m_data.color = Qt::black;
        m_data.message = text;
        m_data.dateTime = QDateTime::currentDateTime();
        m_data.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << m_data; // Сериализация данных

        socket -> write(buffer_array);
        socket -> flush();

        socket -> disconnectFromHost();
        server -> close();
        server -> deleteLater();
    } else {
        server -> close();
        server -> deleteLater();
    }
    delete ui;
}

void Widget::on_pushButton_Listen_clicked() //кнопка подключения / отключения сервера
{
    if (ui->pushButton_Listen->text() == "Connect")
    {
        QHostAddress ip;
        if(!ip.setAddress(ui -> lineEdit_IP -> text())) {
            QMessageBox::warning(this, "Ошибка!", "Вы ввели некорректный IP адрес");
            return;
        }
        ushort port = ui->portServer->value();

        if(!server -> listen(QHostAddress::Any, port))
        {
            QMessageBox::critical(this,"Ошибка",server->errorString());
            return;
        }

        ui->pushButton_Listen->setText("Disconnect");
        ui -> textBrowser -> append("Вы включили сервер, теперь к вам может подключиться клиент!");

        ui -> groupBox -> setEnabled(false);
        if (ui -> inputEdit -> text().length() > 0 and isClientConnected) {
            ui -> pushButton_Send -> setEnabled(true);
        }
    }
    else
    {
        if(isClientConnected) {
            QString text = userName + " закрылся";

            // Отправляем обычное сообщение
            DataToSend m_data;
            m_data.color = Qt::black;
            m_data.message = text;
            m_data.dateTime = QDateTime::currentDateTime();
            m_data.name = userName;

            QByteArray buffer_array;
            QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
            dataStream << m_data; // Сериализация данных

            socket -> write(buffer_array);
            socket -> flush();

            socket -> disconnectFromHost();
            server -> close();
        } else {
            server -> close();
        }

        ui -> textBrowser -> append("Вы отключили сервер!");

        ui -> pushButton_Listen -> setText("Connect");

        ui -> pushButton_Send -> setEnabled(false);
        ui -> groupBox -> setEnabled(true);
    }
}

void Widget::on_pushButton_Send_clicked() //отправка сообщения
{
    QString text = userName + ": " + ui-> inputEdit -> text();

    // Отправляем обычное сообщение
    DataToSend m_data;
    m_data.color = colorSendMessage.name();
    m_data.message = text;
    m_data.dateTime = QDateTime::currentDateTime();
    m_data.name = userName;

    QByteArray buffer_array;
    QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
    dataStream << m_data; // Сериализация данных

    socket -> write(buffer_array);
    socket -> flush();

    QString messageText = text; // Текст сообщения
    QColor messageColor = colorSendMessage; // Цвет сообщения
    QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                   .arg(messageColor.name())
                                   .arg(m_data.dateTime.toString("hh:mm:ss"))
                                   .arg(messageText);

    ui -> textBrowser -> append(formattedMessage);

    ui->inputEdit->clear();
}

void Widget::server_New_Connect() //подключение клиента к серверу
{
    if (isClientConnected) {
        QTcpSocket *socket = server -> nextPendingConnection();
        socket -> disconnectFromHost();
        socket -> close();
    }
    else {
        socket = server -> nextPendingConnection();
        if (banClients.indexOf(socket -> peerAddress().toString()) != -1) {
            socket -> disconnectFromHost();
            socket -> close();
            return;
        }
        QString text = "Вы подключились к серверу: " + userName;

        // Отправляем обычное сообщение
        DataToSend m_data;
        m_data.color = Qt::black;
        m_data.message = text;
        m_data.dateTime = QDateTime::currentDateTime();
        m_data.name = userName;

        QByteArray buffer_array;
        QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
        dataStream << m_data; // Сериализация данных

        socket -> write(buffer_array);

        isClientConnected = true;
        //добавили нового пользователя в list и вектор
        ui -> listClients -> addItem(socket -> peerAddress().toString());
        connectedUsers.push_back(socket -> peerAddress().toString());

        connect(socket, &QTcpSocket::readyRead, this, &Widget::readData);
        connect(socket, &QTcpSocket::disconnected, this, &Widget::socket_Disconnect);

        if (ui -> inputEdit -> text().length() > 0) {
            ui -> pushButton_Send -> setEnabled(true);
        }
    }
}

void Widget::socket_Disconnect() //сигнал отключения
{
    ui -> listClients -> takeItem(connectedUsers.indexOf(socket->peerAddress().toString()));
    connectedUsers.remove(connectedUsers.indexOf(socket -> peerAddress().toString()));

    ui -> pushButton_Send -> setEnabled(false);
    isClientConnected = false;
}

void Widget::readData() //чтение данных
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);

    //QByteArray recieveArr;
    DataToSend recieveData;

    if(in.status() == QDataStream::Ok)
    {
        in.startTransaction();
        in >> recieveData;
        if(!in.commitTransaction())
        {
            qDebug() << "сообщение не дошло";
        }

        QString messageText = recieveData.message;
        QColor messageColor = recieveData.color;

        QString formattedMessage = QString("<span style='color:%1;'>[%2] %3</span>")
                                       .arg(messageColor.name())
                                       .arg(recieveData.dateTime.toString("hh:mm:ss"))
                                       .arg(messageText);

        ui -> textBrowser -> append(formattedMessage);
    }
    countMes++;
}
void Widget::onTimerTimeout() //проверка спама каждые 5 секунд
{
    if (isClientConnected) {
        if (countMes > 5) {
            QString text = "Кто спамит - тот плохой";

            // Отправляем обычное сообщение
            DataToSend m_data;
            m_data.color = colorSendMessage.name();
            m_data.message = text;
            m_data.dateTime = QDateTime::currentDateTime();
            m_data.name = userName;

            QByteArray buffer_array;
            QDataStream dataStream(&buffer_array, QIODevice::WriteOnly);
            dataStream << m_data; // Сериализация данных

            socket -> write(buffer_array);
            socket -> flush();

            socket -> disconnectFromHost();
            isClientConnected = false;
        }
    }
    countMes = 0;
}


void Widget::on_inputEdit_textChanged(const QString &arg1) //изменение отправляемого сообщения
{
    if (arg1.trimmed().isEmpty() or ui->pushButton_Listen->text() == "Connect" or !isClientConnected) {
        ui -> pushButton_Send -> setEnabled(false);
        send = false;
    }
    else {
        ui -> pushButton_Send -> setEnabled(true);
        send = true;
    }
}

void Widget::on_listClients_itemDoubleClicked(QListWidgetItem *item) //бан выбранного пользователя
{
    ui->banList -> addItem(item->text());
    banClients.push_back((item->text()));

    socket -> disconnectFromHost();
    socket -> close();
    isClientConnected = false;
}

void Widget::on_banList_itemDoubleClicked(QListWidgetItem *item) //разбан выбранного пользователя
{
    ui -> banList -> takeItem(banClients.indexOf(item -> text()));
    banClients.remove(banClients.indexOf(item -> text()));

}

