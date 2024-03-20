#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

#include <QtWidgets>
#include <QMessageBox>
#include <QTime>
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
    void on_pushButton_Connect_clicked() ;
    void on_pushButton_Send_clicked() ;

    void keyPressEvent(QKeyEvent * event);

    void readData();
    void socket_disconnect();

    void on_colorButton_clicked();

    void on_inputEdit_textChanged(const QString &arg1);

private:
    Ui::Widget *ui;
    QTcpSocket *m_socket;

    QString userName = "Client #"+ QString::number(rand()%1002+1);
    QColor colorSendMessage = Qt::red;

    bool send = false;
    bool isServerConnected = true;
};

#endif // WIDGET_H
