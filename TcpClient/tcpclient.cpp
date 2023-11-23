#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "protocol.h"
#include "opewidget.h"


TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    // 加载配置
    loadConfig();
    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
    // 连接成功触发 connected()信号
    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));
}

TcpClient::~TcpClient()
{
    delete ui;
}

// 加载配置(IP+端口）
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
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

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;

}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getLoginName()
{
    return m_loginName;
}

QString TcpClient::getCurPath()
{
    return m_strPath;
}

void TcpClient::setCurPath(QString path)
{
    m_strPath = path;
}



void TcpClient::showConnect()
{
    QMessageBox::information(this, "提示", "连接服务器成功");
}

// 客户端：打包数据->发送请求->接收数据->展示
// 接收服务器的数据
void TcpClient::recvMsg()
{
    // 接收的是非下载文件数据
    if (!OpeWidget::getInstance().getBook()->m_isDownload) {


        // 打印当前可读取的字节数
        qDebug()<< "客户端收到了消息（字节数）："<<m_tcpSocket.bytesAvailable();
        // 收数据
        uint uiPDULen = 0;
        // 尝试从套接字读取数据，并将读取的数据存入uiPDULen。这里假设发送方首先发送PDU的长度。
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
        // 计算实际消息的长度
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        // 读取消息内容并存入新创建的PDU对象中
        m_tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
    //    qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
        // 根据消息类型，执行相关操作
        switch (pdu->uiMsgType) {
        // 1.注册回复
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            // 注册成功
            if (strcmp(pdu->caData, REGIST_OK) == 0) {
                QMessageBox::information(this, "提示", REGIST_OK);;

            } else if (strcmp(pdu->caData, REGIST_FAILED) == 0){
                QMessageBox::warning(this, "警告", REGIST_FAILED);
            }

            break;
        }

        // 2.登录回复
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            // 登录成功
            if (strcmp(pdu->caData, LOGIN_OK) == 0) {
                // 客户端的文件根路径（即服务器端存放客户端文件的位置）
                m_strPath = QString("D:/ServerFiles/%1").arg(m_loginName);

                QMessageBox::information(this, "提示", LOGIN_OK);;
                // 显示主界面
                OpeWidget::getInstance().show();
                OpeWidget::getInstance().setWindowTitle("欢迎您："+m_loginName);
                this->hide();

            } else if (strcmp(pdu->caData, LOGIN_FAILED) == 0){
                QMessageBox::warning(this, "警告", LOGIN_FAILED);
            }

            break;
        }
        // 3.在线用户的回复
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            // 显示服务器返回的数据
            OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
            break;
        }
        // 4.搜索用户的回复
        case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
        {

            if (strcmp(SEARCH_USER_NOT_FOUND, pdu->caData) == 0) {
                QMessageBox::information(this, "搜索", QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_searchName));
            } else if(strcmp(SEARCH_USER_ONLINE, pdu->caData) == 0) {
                QMessageBox::information(this, "搜索", QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_searchName));
            } else if(strcmp(SEARCH_USER_OFFLINE, pdu->caData) == 0) {
                QMessageBox::information(this, "搜索", QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_searchName));
            }

            break;
        }
        // 5.1 添加好友的回复 （用户已经是好友、不在线、不存在情况，服务器告知主动加的一方
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
    //        QMessageBox::information(this, "添加好友", pdu->caData);

            if (strcmp(UNKNOW_ERROR, pdu->caData) == 0) {
                QMessageBox::warning(this, "提示", UNKNOW_ERROR);
            } else if( strcmp(ADD_USER_ALREADY, pdu->caData) == 0) {
                QMessageBox::warning(this, "提示", ADD_USER_ALREADY);
            } else if(strcmp(ADD_USER_OFFLINE, pdu->caData) == 0) {
                QMessageBox::warning(this, "提示", ADD_USER_OFFLINE);
            } else if(strcmp(ADD_USER_NOT_EXIST, pdu->caData) == 0) {
                QMessageBox::warning(this, "提示", ADD_USER_NOT_EXIST);
            } else if(strcmp(ADD_USER_NOT_ADD_YOURSELF, pdu->caData) == 0) {
                QMessageBox::warning(this, "提示", ADD_USER_NOT_ADD_YOURSELF);
            }

            break;
        }

        // 5.2 添加好友的请求（收到好友申请 ，被加方
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char recvName[32] = {'\0'};
            strncpy(recvName, pdu->caData, 32);
            char sendName[32] = {'\0'};
            strncpy(sendName, pdu->caData+32, 32);
            int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend ?").arg(sendName),
                                     QMessageBox::Yes, QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, recvName, 32);
            memcpy(respdu->caData+32, sendName, 32);
            if (ret == QMessageBox::Yes) {
                // 同意添加
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;

            } else  { // 拒绝添加
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        // 成功添加好友的回复（服务器告诉主动方加好友成功
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            QMessageBox::information(this, "提示", pdu->caData);
            break;
        }
        // 拒绝添加好友的回复（服务器告诉主动方加好友被拒绝
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            QMessageBox::information(this, "提示", pdu->caData);
            break;
        }


        // 查看在线好友的回复
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            // 显示服务器返回的数据
            OpeWidget::getInstance().getFriend()->showFriendList(pdu);
            break;
        }

        // 删除好友的回复(主动删
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            // 显示删除结果
            QMessageBox::information(this, "提示", pdu->caData);
            break;
        }

        // 删除好友的请求(被删方
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            // 提示谁删除了你
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QMessageBox::information(this, "提示", QString("%1 deleted you as friend").arg(caName));
            break;
        }
        // 私聊请求(接收方
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char send_name[32] = {'\0'};
            memcpy(send_name, pdu->caData, 32);
            QString str_name = send_name;

            if (PrivateChat::getInstance().isHidden()) {
                PrivateChat::getInstance().setWindowTitle(str_name);
                PrivateChat::getInstance().show();
            }

            PrivateChat::getInstance().setChatName(str_name); // 设置要聊天的名字
            PrivateChat::getInstance().receveMsg(pdu);
            break;
        }

        // 群聊请求
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
            break;
        }

        // 创建文件夹响应
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            break;
        }
        // 刷新文件响应
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            QString cur_enter_dir = OpeWidget::getInstance().getBook()->getCurEnterDir();
            // 刷新完：只要目前进入的目录名不为空，那么就要更新当前的路径
            if (!cur_enter_dir.isEmpty()) {
                m_strPath = m_strPath +"/"+ cur_enter_dir;
                qDebug() << m_strPath;
            }
            break;
        }
        // 删除目录响应
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
        {
            QMessageBox::information(this, "删除目录", pdu->caData);
            break;
        }
        // 删除文件响应
        case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
        {
            QMessageBox::information(this, "删除文件", pdu->caData);
            break;
        }

        // 重命名文件响应
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            QMessageBox::information(this, "重命名文件", pdu->caData);
            break;
        }
        // 进入文件夹响应
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            // 进入失败 清空当前进入文件夹名子
            OpeWidget::getInstance().getBook()->clearCurEnterDir();
            QMessageBox::information(this, "进入文件夹", pdu->caData);
            break;
        }
        // 上传文件响应
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {

            QMessageBox::information(this, "上传文件", pdu->caData);
            break;
        }
        // 下载文件响应
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug() << pdu->caData;
            // 接受客户端传回来的文件名和大小
            char filename[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld", filename, &(OpeWidget::getInstance().getBook()->m_total));


            if (strlen(filename) > 0 && OpeWidget::getInstance().getBook()->m_total >0) {
                // 接受数据改为接收文件模式
                OpeWidget::getInstance().getBook()->m_isDownload = true;
                // 打开文件
                m_file.setFileName(OpeWidget::getInstance().getBook()->getDownloadPath());
                if (!m_file.open((QIODevice::WriteOnly))) {
                    QMessageBox::information(this, "下载文件", "获取保存文件的路径错误");
                }

            }
            break;
        }

        // 分享文件响应
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {

            QMessageBox::information(this, "分享文件", pdu->caData);
            break;
        }

        // 分享文件通知
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char *file_path = new char[pdu->uiMsgLen];
            memcpy(file_path, pdu->caMsg, pdu->uiMsgLen);
            char *pos = strrchr(file_path, '/');
            if (pos != NULL) {
                pos++;
                QString note = QString("%1 share file: %2 \n Do you wand to receive?").arg(pdu->caData).arg(pos);
                int res = QMessageBox::question(this, "note", note);
                if (res == QMessageBox::Yes) {
                    // 确认接收文件---
                    // 打包 文件地址+客户端姓名 -->告诉服务器
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_ACCEPT;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString name = TcpClient::getInstance().getLoginName();
                    strcpy(respdu->caData, name.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);

                }
            }

            break;
        }
        // 移动文件响应
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {

            QMessageBox::information(this, "移动文件", pdu->caData);
            break;
        }

        default:
            break;
        }

        free(pdu);
        pdu = NULL;

    }
    else {  // 服务器传来的下载文件的数据
        // 分块接收
        const int bufferSize = 4096;
        char buffer[bufferSize];
        Book *book = OpeWidget::getInstance().getBook();
        qint64 bytesRead;
        while ((bytesRead = m_tcpSocket.read(buffer, bufferSize)) > 0) {
            qint64 bytesWritten = m_file.write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                qDebug() << "下载文件写入失败";
                break;
            }

            book->m_recev += bytesRead;
        }

        if (book->m_total == book->m_recev) {
            qDebug() << "total:" << book->m_total << "  receved:" << book->m_recev;
            m_file.close();
            book->m_total =0;
            book->m_recev = 0;
            book->m_isDownload = false;
            QMessageBox::information(this, "下载文件", "下载文件成功");
        } else if (book->m_total < book->m_recev) {
            qDebug() << "total:" << book->m_total << " receved:" << book->m_recev;
            m_file.close();
            book->m_total =0;
            book->m_recev = 0;
            book->m_isDownload = false;
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }

    }
}


// 测试发送消息
#if 0
void TcpClient::on_pushButton_clicked()
{
    // 获取输入的信息
    QString strMsg = ui->lineEdit->text();
    if (strMsg.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送消息不可为空");
    } else {
        // 发消息给服务器
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = 8888; // 消息类型
        // 拷贝数据
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        qDebug() << (char*)pdu->caMsg;
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        // 释放空间
        free(pdu);
        pdu = NULL;
    }
}
#endif


// 注册
void TcpClient::on_pushButton_2_clicked()
{
    QString strName = ui->lineEdit->text();
    QString strPwd = ui->lineEdit_2->text();
    // 判断是否为空
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        // 封装注册数据
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
        // 给服务器发注册消息
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);

        free(pdu);
        pdu = NULL;

    } else {
        QMessageBox::critical(this, "注册", "注册失败，用户名或密码不能为空！");
    }
}



// 登录
void TcpClient::on_loginButton_clicked()
{
    QString strName = ui->lineEdit->text();
    QString strPwd = ui->lineEdit_2->text();
    // 判断是否为空
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        m_loginName = strName;
        // 封装登录数据
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
        // 给服务器发登录消息
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);

        free(pdu);
        pdu = NULL;

    } else {
        QMessageBox::critical(this, "登录", "登录失败，用户名或密码不能为空！");
    }
}


// 注销
void TcpClient::on_pushButton_3_clicked()
{

}
