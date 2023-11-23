#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opdb.h"
#include <QDir>
#include <QFile>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();

    void  copyDir(QString copy_dir, QString dest_dir);

signals:
    void offline(MyTcpSocket *mysocket);

public slots:
    // 接收信息
    void recvMsg();
    // 处理客户端下线
    void clientOffline();
    void sendFileDataToClient();

private:
    // 客户端socket名字
    QString m_strName;

    // 上传文件相关
    QFile m_file;
    qint64 m_total;
    qint64 m_recved;
    // 是否为上传文件数据状态
    bool m_upload;

    QTimer *m_timer;

};




#endif // MYTCPSOCKET_H
