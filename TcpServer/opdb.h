#ifndef OPDB_H
#define OPDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class OpDB : public QObject
{
    Q_OBJECT
public:
    explicit OpDB(QObject *parent = nullptr);
    // 单例模式
    static OpDB& getInstance();

    void init();

    ~OpDB();

    // 处理注册
    bool handleRegist(const char * name, const char *pwd);
    // 处理登录
    bool handleLogin(const char * name, const char *pwd);
    // 处理注销
    void handleOffline(const char * name);
    // 处理所有在线用户
    QStringList handleAllOnline();
    // 处理搜索用户
    int handleSearchUser(const char * name);
    // 添加好友
    int handleAddFriend(const char * username, const char * loginname);
    // 同意添加好友
    bool handleAgreeAddFriend(const char * recvname, const char * sendname);

    // 刷新好友
    QStringList handleFlushFriend(const char * name);
    QList<QPair<QString, int>> handleFlushFriend2(const char * name);

    // 删除好友
    bool handelDeleteFriend(const char * username, const char * friendname);

signals:

public slots:

private:
    QSqlDatabase m_db;  // 连接数据库


};

#endif // OPDB_H
