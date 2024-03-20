#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QtWidgets>
#include <QMessageBox>
#include <QTime>
#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHostAddress>


struct DataToSend { // Структура для передачи данных
    QString name;
    QString message;
    QColor color;
    QDateTime dateTime;

    // Оператор для сериализации
    friend QDataStream &operator << (QDataStream &out, const DataToSend &data) {
        out << data.name;
        out << data.message;
        out << data.color;
        out << data.dateTime;
        return out;
    }

    // Оператор для десериализации
    friend QDataStream &operator >> (QDataStream &in, DataToSend &data) {
        in >> data.name;
        in >> data.message;
        in >> data.color;
        in >> data.dateTime;
        return in;
    }

    // "friend" указывает на то, что эта функция будет дружественной для структуры,
    // и она имеет доступ к его закрытым членам.
};

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();


private slots:
    void on_pushButton_Listen_clicked();

    void on_pushButton_Send_clicked();

    void keyPressEvent(QKeyEvent * event);

    void server_New_Connect();
    void readData();
    void socket_Disconnect();
    void onTimerTimeout();

    void on_colorButton_clicked();

    void on_inputEdit_textChanged(const QString &arg1);

    void on_listClients_itemDoubleClicked(QListWidgetItem *item);

    void on_banList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::Widget *ui;
    QTcpServer *server;
    QTcpSocket *socket;

    QString userName = "Server";
    QColor colorSendMessage = Qt::blue;

    bool send = false;
    bool isClientConnected = false; //true - пользователь подключен
                            //false - пользователь отключен
    QTimer *timer = new QTimer(this);
    QTime lastMessageTime;
    int countMes = 0;
    QVector <QString> banClients;
    QVector <QString> connectedUsers;

};

#endif // WIDGET_H
