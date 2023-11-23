#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "privatechat.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

    void loadConfig();

    // 单例模式
    static TcpClient &getInstance();

    QTcpSocket &getTcpSocket();
    QString getLoginName();

    QString getCurPath();
    void setCurPath(QString path);

public slots:
    void showConnect();
    void recvMsg();

private slots:
//    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_loginButton_clicked();

private:
    Ui::TcpClient *ui;
    // 服务器的IP和端口
    QString m_strIP;
    quint16 m_usPort;
    // 连接服务器
    QTcpSocket m_tcpSocket;
    QString m_loginName;

    QString m_strPath;

    QFile m_file;
};
#endif // TCPCLIENT_H
