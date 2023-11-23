#include "tcpserver.h"

#include <QApplication>
#include "opdb.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化数据库
    OpDB::getInstance().init();

    TcpServer w;
    w.show();
    return a.exec();
}
