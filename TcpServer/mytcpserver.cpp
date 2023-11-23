#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

// 每当有一个新的客户端连接时，都会产生一个Socket,这个函数会被调用
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New client connected...";

    // 创建一个新的MyTcpSocket对象来管理与该客户端的通信
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    // 将一个现有的套接字描述符关联到QTcpSocket对象
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    // 在后续中管理或监视所有的客户端连接
    m_tcpSocketList.append(pTcpSocket);

    // 关联客户端下线的信号和槽
    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*)), this, SLOT(deleteSocket(MyTcpSocket*)));
}

// 服务器进行转发
void MyTcpServer::resend(const char *username, PDU *pdu)
{
    if (username == NULL || pdu == NULL) {
        return;
    }
    QString strName = username;

    // 遍历存储在m_tcpSocketList中的所有TcpSocket对象
    for (int i = 0; i< m_tcpSocketList.size();i++){
        if (strName == m_tcpSocketList.at(i)->getName()) {
            // 如果匹配，向该TcpSocket对象写入指定的PDU数据
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }

    }

}

// 处理下线的客户端
void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    // 将注销的客户端socket从列表删除
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(;iter != m_tcpSocketList.end(); iter++){
        if (mysocket == *iter) {
            qDebug() << "客户端:" << mysocket->getName() << "下线";
            (*iter)->deleteLater(); // 延迟释放空间！！！！！！！！
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    // 显示目前还在线的客户端
    if (m_tcpSocketList.size() != 0 ) {
        qDebug() << "目前在线的客户端：" ;
        for (int i = 0; i< m_tcpSocketList.size();i++){
            qDebug() << m_tcpSocketList.at(i)->getName();
        }
    } else {
        qDebug() << "目前没有在线的客户端" ;
    }
}
