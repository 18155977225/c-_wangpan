#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"
#include <QFileInfoList>


MyTcpSocket::MyTcpSocket()
{
    m_strName = "";
    m_upload = false;


    // 数据到达时 触发信号 readyRead(), 然后进行接收消息recvMsg()
    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    // 客户端下线 触发信号 disconnected()
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));

    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(sendFileDataToClient()));


}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString copy_dir, QString dest_dir)
{
        QDir dir;
        dir.mkdir(dest_dir);

        dir.setPath(copy_dir);
        QFileInfoList fileInfoList = dir.entryInfoList();

        QString srcTmp;
        QString destTmp;
        for (int i=0; i< fileInfoList.size(); i++) {
            if (fileInfoList[i].fileName() == QString(".") || fileInfoList[i].fileName() == QString("..")) {
                continue;
            }
            if (fileInfoList[i].isFile()) {
                srcTmp = copy_dir+'/'+fileInfoList[i].fileName();
                destTmp = dest_dir+'/'+fileInfoList[i].fileName();
                QFile::copy(srcTmp, destTmp);
            } else if (fileInfoList[i].isDir()) {
                srcTmp = copy_dir+'/'+fileInfoList[i].fileName();
                destTmp = dest_dir+'/'+fileInfoList[i].fileName();
                copyDir(srcTmp, destTmp);
            }
        }

}


// 接收客户端发的数据：解析数据->处理请求->返回数据
void MyTcpSocket::recvMsg()
{
    // 接收的不是上传文件的数据
    if (!m_upload) {

        // 打印当前可读取的字节数
        qDebug()<< "服务器收到了消息（字节数）："<< this->bytesAvailable();
        uint uiPDULen = 0;
        // 尝试从套接字读取数据，并将读取的数据存入uiPDULen。这里假设发送方首先发送PDU的长度。
        this->read((char*)&uiPDULen, sizeof(uint));
        // 计算实际消息的长度
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        // 读取消息内容并存入新创建的PDU对象中
        this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));



        // 根据请求的消息类型，执行相关操作，然后把结果返回给客户端
        switch (pdu->uiMsgType) {
        // 1.注册请求
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {

            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            // 获取处理数据
            bool res= OpDB::getInstance().handleRegist(caName, caPwd);
            // 封装数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            // 给客户端回复
            if (res) {
                qDebug()<< "有客户端注册了新账号："<< caName;

                strcpy(respdu->caData, REGIST_OK);

                // 创建文件夹（用户名命名)
                QDir dir;
                // D:/ServerFiles为服务器存放客户端文件的位置
                dir.mkdir(QString("D:/ServerFiles/%1").arg(caName));

            } else {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            this->write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            break;
        }

        // 2.登录请求
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            // 获取处理数据
            bool res= OpDB::getInstance().handleLogin(caName, caPwd);
            // 封装数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            // 给客户端回复
            if (res) {
                qDebug()<< "客户端："<< caName << "登录了";

                strcpy(respdu->caData, LOGIN_OK);
                m_strName = caName;

            } else {
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            // 发送结果给客户端
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            break;
        }
        // 3.all在线用户请求
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            // 获取处理数据
            QStringList res = OpDB::getInstance().handleAllOnline();
            // 整理数据
            uint uiMsgLen = res.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i = 0; i< res.size(); i++) {
                memcpy((char*)(respdu->caMsg)+i*32, res.at(i).toStdString().c_str(), res.at(i).size());
            }
            // 发给客户端
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            break;
        }
        // 4.搜索用户请求
        case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:
        {
            // 获取处理结果
            int res = OpDB::getInstance().handleSearchUser(pdu->caData);
            // 整理数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
            if (res == -1) {
                strcpy(respdu->caData, SEARCH_USER_NOT_FOUND);
            } else if (res == 1) {
                strcpy(respdu->caData, SEARCH_USER_ONLINE);
            } else if (res == 0) {
                strcpy(respdu->caData, SEARCH_USER_OFFLINE);
            }
            // 发给客户端
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            break;
        }

        // 5.添加好友请求(发送方
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char userName[32] = {'\0'};
            char loginName[32] = {'\0'};
            strncpy(userName, pdu->caData, 32);
            strncpy(loginName, pdu->caData+32, 32);
            // 获取处理结果
            int res = OpDB::getInstance().handleAddFriend(userName, loginName);
            // 整理数据
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            if (res == -1) {
                strcpy(respdu->caData, UNKNOW_ERROR);
            } else if (res == -2) { // 加的是自己
                strcpy(respdu->caData, ADD_USER_NOT_ADD_YOURSELF);
            } else if (res == 0) { // 已经是好友
                strcpy(respdu->caData, ADD_USER_ALREADY);
            } else if (res == 1) { // 在线且非好友
                // 让服务器转发好友申请给被加的用户
                MyTcpServer::getInstance().resend(userName, pdu); // 注意是pdu!!!

            } else if (res == 2) { // 离线
                strcpy(respdu->caData, ADD_USER_OFFLINE);
            } else if (res == 3) { // 不存在
                strcpy(respdu->caData, ADD_USER_NOT_EXIST);
            }
            // 发给客户端
            write((char*)respdu, respdu->uiPDULen); // respdu
            free(respdu);
            respdu = NULL;

            break;
        }
        // 同意添加好友（接收方
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            char recvName[32] = {'\0'};
            char sendName[32] = {'\0'};
            strncpy(recvName, pdu->caData, 32);
            strncpy(sendName, pdu->caData+32, 32);
            // 获取处理结果
            bool res = OpDB::getInstance().handleAgreeAddFriend(recvName, sendName);


            if (res) {
                // 处理成功 给客户端反馈
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
                qDebug() << "添加好友成功";
                strcpy(respdu->caData, "success add");
                // 发给客户端
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;

            } else {
                qDebug() << "添加好友失败";
            }



            break;
        }
        // 拒绝添加好友（被加方
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            qDebug() << "对方拒绝添加好友";
            // 告诉客户端
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;

            strcpy(respdu->caData, "refuse add");
            // 发给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        // 6.刷新好友请求
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char loginName[32] = {'\0'};
            strncpy(loginName, pdu->caData, 32);
            /*
            // 获取处理结果：只有name
            QStringList res = OpDB::getInstance().handleFlushFriend(loginName);

            // 整理数据
            uint uiMsgLen = res.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for (int i = 0; i< res.size(); i++) {
                memcpy((char*)(respdu->caMsg)+i*32, res.at(i).toStdString().c_str(), res.at(i).size());
            }
            // 发给客户端
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            */

            // 获取处理结果：name+online
            QList<QPair<QString, int>> res = OpDB::getInstance().handleFlushFriend2(loginName);
            // 封装成PDU并发送给客户端
            uint uiMsgLen = res.size() * 64;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;

            for (int i = 0; i < res.size(); i++) {

                QString name = res.at(i).first;
                QString online = (res.at(i).second == 0) ? "offline" : "online";
                qDebug() <<name << online;

                // 拼接成字符数据
                QString friend_data = QString("%1 :%2").arg(name).arg(online);
                memcpy((char*)(respdu->caMsg)+i*64, friend_data.toStdString().c_str(), friend_data.size());

            }

            // 发给客户端

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 7.删除好友请求
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char u_Name[32] = {'\0'};
            char f_Name[32] = {'\0'};
            strncpy(u_Name, pdu->caData, 32);
            strncpy(f_Name, pdu->caData+32, 32);

            // 获取处理结果：有name+online
            bool res = OpDB::getInstance().handelDeleteFriend(u_Name, f_Name);
            // 封装成PDU并发送给客户端
            if (res) {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
                strcpy(respdu->caData, DEL_FRIEND_SECCESS);

                // 发给客户端(主动删的一方）
                write((char*)respdu, respdu->uiPDULen); // respud
                free(respdu);
                respdu = NULL;

                // 通知被删的一方
                MyTcpServer::getInstance().resend(f_Name, pdu); // pdu
            } else {
                qDebug() << "删除失败" ;
            }



            break;
        }
        // 8.私聊请求
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char friend_Name[32] = {'\0'};
            memcpy(friend_Name, pdu->caData+32, 32);
            // 服务器转发给被私聊的客户端
            MyTcpServer::getInstance().resend(friend_Name, pdu); // pdu

            break;
        }
        // 9.群聊请求
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char send_name[32] = {'\0'};
            memcpy(send_name, pdu->caData, 32);
            // 查看发送发所有好友
            QList<QPair<QString, int>> res = OpDB::getInstance().handleFlushFriend2(send_name);
            for (int i = 0; i < res.size(); i++) {

                QString friend_name = res.at(i).first;
                int online = res.at(i).second;
                // 转发给在线好友
                if (online == 1) {
                    MyTcpServer::getInstance().resend(friend_name.toStdString().c_str(), pdu); // pdu
                }

            }


            break;
        }

        // 10.创建文件夹请求
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            bool isExist = dir.exists(userPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;


            if (isExist) { // 用户路径存在
                char newDir[32] = {'\0'};
                memcpy(newDir, pdu->caData+32, 32);
                QString newPath = userPath + "/" +newDir;
                qDebug() << newPath;
                isExist = dir.exists(newPath);
                if (isExist) { // 文件夹名已存在
                    strcpy(respdu->caData, DIR_NAME_EXIST);
                } else {
                    dir.mkdir(newPath);
                    strcpy(respdu->caData, DIR_CREATE_DONE);
                }
            } else { // 用户路径不存在
                strcpy(respdu->caData, USER_PAHT_NOT_EXIST);
            }
            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }

        // 11.刷新文件请求
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {

            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QDir dir(userPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int fileNum = fileInfoList.size();

            // 打包文件信息
            PDU *respdu = mkPDU(sizeof(FileInfo)*fileNum);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;


            for (int i = 0; i< fileNum; i++) {
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;

                memcpy(pFileInfo->fileName, fileInfoList[i].fileName().toStdString().c_str(), fileInfoList[i].fileName().size());
                if (fileInfoList[i].isDir()) {
                    pFileInfo->fileType = 0;
                } else if(fileInfoList[i].isFile()) {
                    pFileInfo->fileType = 1;
                }
            }

            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 12.删除目录请求
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
        {

            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QString dir_name = QString("%1").arg((char*)(pdu->caData));
            // 拼接删除的目录的路径
            QString del_path = QString("%1/%2").arg(userPath).arg(dir_name);
    //        qDebug() << del_path;

            QFileInfo fileInfo(del_path);
            bool res = false;
            // 如果是目录文件 才进行删除
            if (fileInfo.isDir()) {
                QDir dir(del_path);
                res = dir.removeRecursively();
            }

            // 打包删除目录结果
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
            if (res) {
                memcpy(respdu->caData, "delete dir success", strlen("delete dir success"));
            } else {
                memcpy(respdu->caData, "not a dir, delete fail", strlen("not a dir, delete fail"));
            }

            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }

        // 13.重命名文件请求
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QString old_file_name = QString("%1").arg((char*)(pdu->caData));
            QString new_file_name = QString("%1").arg((char*)(pdu->caData)+32);
            // 拼接新旧文件的路径
            QString old_path = QString("%1/%2").arg(userPath).arg(old_file_name);
            QString new_path = QString("%1/%2").arg(userPath).arg(new_file_name);
    //        qDebug() << old_path;
    //        qDebug() << new_path;

            QDir dir;
            bool res = dir.rename(old_path, new_path);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if (res) {
                memcpy(respdu->caData, "rename file success", strlen("rename file success"));
            } else {
                memcpy(respdu->caData, "rename file fail", strlen("rename file fail"));
            }
            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 14. 进入文件夹请求
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QString dir_name = QString("%1").arg((char*)(pdu->caData));

            // 拼接dir的路径
            QString path = QString("%1/%2").arg(userPath).arg(dir_name);
            QFileInfo fileInfo(path);

            PDU *respdu = NULL;
            // 如果是目录文件 才进行刷新
            if (fileInfo.isDir()) {
                QDir dir(path);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int fileNum = fileInfoList.size();

                // 打包文件信息
                respdu = mkPDU(sizeof(FileInfo)*fileNum);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo = NULL;


                for (int i = 0; i< fileNum; i++) {
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;

                    memcpy(pFileInfo->fileName, fileInfoList[i].fileName().toStdString().c_str(), fileInfoList[i].fileName().size());
                    if (fileInfoList[i].isDir()) {
                        pFileInfo->fileType = 0;
                    } else if(fileInfoList[i].isFile()) {
                        pFileInfo->fileType = 1;
                    }
                }


            } else if(fileInfo.isFile() ){ // 是一个普通文件
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, "enter dir fail, not dir");

            }
            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }

        // 15.上传文件请求
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char filename[32] = {'\0'};
            qint64 filesize = 0;

            sscanf(pdu->caData, "%s %lld", filename, &filesize);
            char *cur_path = new char[pdu->uiMsgLen];
            memcpy(cur_path, pdu->caMsg, pdu->uiMsgLen);


            // 拼接上传文件的路径
            QString file_path = QString("%1/%2").arg(cur_path).arg(filename);
            qDebug() <<file_path;

            delete []cur_path;
            cur_path = nullptr;

            m_file.setFileName(file_path);
            // 以只写的方式打开文件，若不存在，自动创建
            if (m_file.open(QIODevice::WriteOnly)) {
                m_upload = true;
                m_total = filesize;
                m_recved = 0;
            }
            break;
        }

        // 16.删除文件请求
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
        {

            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QString file_name = QString("%1").arg((char*)(pdu->caData));
            // 拼接删除的目录的路径
            QString del_path = QString("%1/%2").arg(userPath).arg(file_name);
    //        qDebug() << del_path;

            QFileInfo fileInfo(del_path);
            bool res = false;
            // 如果是目录文件 才进行删除
            if (fileInfo.isFile()) {
                QDir dir;
                res = dir.remove(del_path);
            }

            // 打包删除目录结果
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
            if (res) {
                strcpy(respdu->caData, "delete file success");
            } else {
                strcpy(respdu->caData, "not a file, delete fail");

            }

            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }

        // 17.下载文件请求
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {

            QString userPath = QString("%1").arg((char*)(pdu->caMsg));
            QString file_name = QString("%1").arg((char*)(pdu->caData));

            QString file_path = QString("%1/%2").arg(userPath).arg(file_name);
    //            qDebug() << file_path;

            QFileInfo fileInfo(file_path);
            qint64 file_size = fileInfo.size();


            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData, "%s %lld", file_name.toStdString().c_str(), file_size);
            qDebug() << respdu->caData;

            // 发送给客户端
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            // 打开文件
            m_file.setFileName(file_path);
            m_file.open(QIODevice::ReadOnly);
            // 开启定时器 时间到了 就开始发送文件的数据
            m_timer->start(1000);
            break;
        }


        // 18.共享文件请求
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {

            // 解析客户端发来的数据
            char send_name[32] = {'\0'};
            int num;
            sscanf(pdu->caData, "%s %d", send_name, &num);
            qDebug() << send_name << num;

            int size = num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            // 打包：send_name + file_path 给接受者客户端
            strcpy(respdu->caData, send_name);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg)+size, pdu->uiMsgLen-size);

            char rec_name[32] = {'\0'};
            // 转发给每个接收者
            for (int i = 0; i< num; i++) {
                memcpy(rec_name, (char*)(pdu->caMsg)+i*32, 32);
                MyTcpServer::getInstance().resend(rec_name, respdu);
            }

            free(respdu);
            respdu = nullptr;

            // 告诉分享方的客户端
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, "share file success");
            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = nullptr;

            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_ACCEPT:
        {
            QString recv_path = QString("D:/ServerFiles/%1").arg(pdu->caData);
            QString share_file_path = QString("%1").arg((char*)pdu->caMsg);
            int index = share_file_path.lastIndexOf('/');
            QString file_name = share_file_path.right(share_file_path.size()-index-1);
            recv_path = recv_path + '/' + file_name;

            QFileInfo fileInfo(share_file_path);
            if (fileInfo.isFile()) { // 复制文件
                QFile::copy(share_file_path, recv_path);
            } else if (fileInfo.isDir()) { // 复制文件夹
                copyDir(share_file_path, recv_path);
            }
            break;
        }

        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            // 解析客户端发来的数据
            char file_name[32] = {'\0'};
            int movePathSize;
            int destPathSize;
            sscanf(pdu->caData, "%d %d %s", &movePathSize, &destPathSize, file_name);

            char *move_file_path = new char[movePathSize+1];
            char *dest_dir_path = new char[destPathSize+1+32];
            memset(move_file_path, '\0', movePathSize+1);
            memset(dest_dir_path, '\0', destPathSize+1+32);

            memcpy(move_file_path, pdu->caMsg, movePathSize);
            memcpy(dest_dir_path, (char*)(pdu->caMsg)+(movePathSize+1), destPathSize);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

            QFileInfo fileInfo(dest_dir_path);
            if (fileInfo.isDir()) {
                strcat(dest_dir_path, "/");
                strcat(dest_dir_path, file_name);

                bool res = QFile::rename(move_file_path, dest_dir_path);
                if (res) {
                    strcpy(respdu->caData, "move success");
                } else {
                    strcpy(respdu->caData, "move fail");
                }
            } else if (fileInfo.isFile()) {
                strcpy(respdu->caData, "destinate path not a dir!");
            }

            write((char*)respdu, respdu->uiPDULen);

            free(respdu);
            respdu = nullptr;


            break;
        }


        default:
            break;
        }

        free(pdu);
        pdu = NULL;
    }

    // 接收的是上传文件的数据
    else {

        // 一次性读完
//        QByteArray buf = readAll();
//        m_file.write(buf);
//        m_recved += buf.size();

        // 分块读取
        const int bufferSize = 4096;
        char buffer[bufferSize];

        qint64 bytesRead;
        while ((bytesRead = read(buffer, bufferSize)) > 0) {
            qint64 bytesWritten = m_file.write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                qDebug() << "写入文件失败";
                break;
            }

            m_recved += bytesRead;
        }

        if(m_total == m_recved){
            qDebug() << "接收数据完整";


            PDU *res = mkPDU(0);
            res->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            qDebug() << "total: " << m_total << " recvSize: "<< m_recved;
            m_file.close();
            m_upload = false;

            strcpy(res->caData, "upload file success");
            write((char*)res, res->uiPDULen);
            free(res);
            res = NULL;


        } else if(m_recved > m_total) {
            qDebug() << "total: " << m_total << " recvSize: "<< m_recved;
            m_file.close();
            m_upload = false;
            PDU *res = mkPDU(0);
            res->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(res->caData, "upload file fail");
            write((char*)res, res->uiPDULen);
            free(res);
            res = NULL;

        }

     }


}


// 向客户端发送文件数据
void MyTcpSocket::sendFileDataToClient()
{
    m_timer->stop();

    char *buf = new char[4096];
    qint64 len = 0;
    qint64 sendSize = 0;

    while((len = m_file.read(buf, 4096)) > 0)
    {

        qint64 bytesWritten = write(buf, len);
        if(bytesWritten == -1)
        {
            qDebug() << "服务器发送文件数据失败";
        }
        sendSize += bytesWritten;

        if (len < 0)
        {
            qDebug() << "服务器读取文件数据失败";
            break;
        }
    }

    qDebug() << "sendSize:" << sendSize;
    m_file.close();
    delete[] buf;
    buf = NULL;


}

// 处理客户端下线（关闭窗口）
void MyTcpSocket::clientOffline()
{
    // 修改用户登录状态
    OpDB::getInstance().handleOffline(this->getName().toStdString().c_str());
    // 发出信号,让服务器把这个客户端的socket删除
    emit offline(this);

}
