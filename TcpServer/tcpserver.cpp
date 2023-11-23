#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    // 加载配置
    loadConfig();
    // 监听客户端的链接
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}



void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
//        qDebug() << strData;
        file.close();

        QStringList strList = strData.replace("\r\n", " ").split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
//        qDebug() << m_strIP << m_usPort;

    } else {
        QMessageBox::critical(this, "警告", "打开失败");
    }
}

