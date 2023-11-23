#include "opdb.h"
#include <QMessageBox>
#include <QDebug>

OpDB::OpDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpDB &OpDB::getInstance()
{
    // 单例模式
    static OpDB instance;
    return instance;
}


void OpDB::init()
{
    // 连接数据库
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\CppProjects\\QT\\TcpServer\\cloud.db");
    if (m_db.open()) {

        //查询
        QSqlQuery query;
        query.exec("select * from userInfo");
        qDebug() << "-------userInfo--------";
        while (query.next()) {
            QString data = QString("%1 %2 %3 %4").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() <<  data;
        }
        qDebug() << "-------friendInfo------";
        query.exec("select * from friendInfo");
        while (query.next()) {
            QString data = QString("%1 %2").arg(query.value(0).toString()).arg(query.value(1).toString());
            qDebug() <<  data;
        }
        qDebug() << "--------连接成功---------";
    } else {
        qDebug() << "连接失败";
    }
}

OpDB::~OpDB()
{
    m_db.close();
}


bool OpDB::handleRegist(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL) {
        return false;
    }
    QString data = QString("insert into userInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpDB::handleLogin(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL) {
        return false;
    }
    //查询账号密码及登录状态是否正确
    QString data = QString("select * from userInfo where name=\'%1\' and pwd=\'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
//    qDebug() << data;
    query.exec(data);
    // 找到
    if (query.next()){
        data = QString("update userInfo set online=1 where name=\'%1\' and pwd=\'%2\' ").arg(name).arg(pwd);
//        qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        return true;
    } else {
        return false;
    }

}

void OpDB::handleOffline(const char *name)
{
    if (name == NULL) {
        return;
    }
    //查询账号密码及登录状态是否正确
    QString data = QString("update userInfo set online=0 where name=\'%1\' ").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpDB::handleAllOnline()
{
    QString data = QString("select * from userInfo where online = 1 ");
    QSqlQuery query;
    query.exec(data);
    QStringList res;
    res.clear();

    while(query.next()) {
//        qDebug() << "查找在线用户成功";
        res.append(query.value(1).toString());
    }
    return res;

}

int OpDB::handleSearchUser(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    QString data = QString("select online from userInfo where name = \'%1\' ").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next()) {
        int ret = query.value(0).toInt();
        if (ret == 1) {
            return 1;
        } else if (ret == 0) {
            return 0;
        }

    }
    else {
        return -1;
    }

}

int OpDB::handleAddFriend(const char *username, const char *loginname)
{
    if (username == NULL || loginname == NULL) {
        return -1;
    }

    if (strcmp(username, loginname) == 0) {
        return -2; // 加的是自己
    }
    // 查看对方是不是好友
    QString data = QString("select * from friendInfo where (id = (select id from userInfo where name=\'%1\') and friendId = (select id from userInfo where name=\'%2\') ) "
                           "or (id = (select id from userInfo where name=\'%3\') and friendId = (select id from userInfo where name=\'%4\')) ").arg(username).arg(loginname).arg(loginname).arg(username);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()) {
        qDebug() << "已经是好友";
        return 0; // 双方已经是好友
    }
    else {
        // 查看对方是否在线
        QString data = QString("select online from userInfo where name = \'%1\' ").arg(username);
        QSqlQuery query;
        query.exec(data);
        if(query.next()) {
            int ret = query.value(0).toInt();
            if (ret == 1) {
                qDebug() << "对方在线";
                return 1; // 在线
            } else if (ret == 0) {
                qDebug() << "对方不在线";
                return 2; // 不在线
            }

        }
        else {
            qDebug() << "对方不存在";
            return 3; // 用户不存在
        }
    }
}

bool OpDB::handleAgreeAddFriend(const char *recvname, const char *sendname)
{
    // 处理同意添加好友(已经确定两人符合加好友的条件)
    if (recvname == NULL || sendname == NULL) {
        return false;
    }
    // 查找id
    int recv_id, send_id = 0;
    QString id1 = QString("select id from userInfo where name = \'%1\' ").arg(recvname);
    QSqlQuery query;
    query.exec(id1);
    if(query.next()) {
        recv_id = query.value(0).toInt();
    } else {
        return false;
    }

    QString id2 = QString("select id from userInfo where name = \'%1\' ").arg(sendname);
    query.exec(id2);
    if(query.next()) {
        send_id = query.value(0).toInt();
    } else {
        return false;
    }
    QString data = QString("insert into friendInfo(id, friendId) values(\'%1\',\'%2\')").arg(recv_id).arg(send_id);
    return query.exec(data);
}

QStringList OpDB::handleFlushFriend(const char *name)
{
    QStringList friendList;
    friendList.clear();
    if (name == NULL) {
        return friendList;
    }

    // all friend: name
    QString data = QString("select name from userInfo where id = ( select id from friendInfo where friendId=(select id from userInfo where name = \'%1\')) ").arg(name);

    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        QString name = query.value(0).toString();
        qDebug() << name;
        friendList.append(name);
    }
    data = QString("select name from userInfo where id = ( select friendId from friendInfo where id=(select id from userInfo where name = \'%1\')) ").arg(name);
    query.exec(data);
    while (query.next()) {
        QString name = query.value(0).toString();
        qDebug() << name;
        friendList.append(name);
    }

    return friendList;
}

QList<QPair<QString, int> > OpDB::handleFlushFriend2(const char *name)
{
    QList<QPair<QString, int>> friendList;
    friendList.clear();
    if (name == NULL) {
        return friendList;
    }
    // all friend: name+onine
    QString data = QString("select name,online from userInfo where (id in ( select id from friendInfo where friendId=(select id from userInfo where name = \'%1\')) "
                           "or id in ( select friendId from friendInfo where id=(select id from userInfo where name = \'%2\')))").arg(name).arg(name);
    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        QString name = query.value(0).toString();
        int online = query.value(1).toInt();
        qDebug() << name << online;
        friendList.append(qMakePair(name, online));

    }

    return friendList;
}

bool OpDB::handelDeleteFriend(const char *username, const char *friendname)
{
    if (username == NULL || friendname == NULL) {
        return false;
    }
    QString data = QString("delete from friendInfo where id = ( select id from userInfo where name= \'%1\') and friendId = ( select id from userInfo where name= \'%2\')").arg(friendname).arg(username);
    QSqlQuery query;
    query.exec(data);
    data = QString("delete from friendInfo where id = ( select id from userInfo where name= \'%1\') and friendId = ( select id from userInfo where name= \'%2\')").arg(username).arg(friendname);
    return true;
}
