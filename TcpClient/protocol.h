#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>


typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed :name existed"
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed :name error or pwd error or relogin "
#define SEARCH_USER_NOT_FOUND "Not found the people"
#define SEARCH_USER_ONLINE "online"
#define SEARCH_USER_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define ADD_USER_ALREADY "already is your friend"
#define ADD_USER_OFFLINE "the user offline"
#define ADD_USER_NOT_EXIST "the user not exist"
#define ADD_USER_NOT_ADD_YOURSELF "not add youself"


#define DEL_FRIEND_SECCESS "success delete friend"

#define USER_PAHT_NOT_EXIST "user path not exist"
#define DIR_NAME_EXIST "dir name exist"
#define DIR_CREATE_DONE "dir create success"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST, // 注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND, // 注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST, // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND, // 登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST, // 在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND, // 在线用户回复

    ENUM_MSG_TYPE_SEARCH_USER_REQUEST, // 搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USER_RESPOND, // 搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST, // 添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND, // 添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE, // 同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE, // 拒绝添加好友

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST, // 刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND, // 刷新好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST, // 删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND, // 删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, // 私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND, // 私聊回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST, // 群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND, // 群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST, // 创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND, // 创建文件夹回复

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST, // 查看文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND, // 查看文件回复

    ENUM_MSG_TYPE_DELETE_DIR_REQUEST, // 删除目录请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND, // 删除目录回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST, // 重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND, // 重命名文件回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST, // 进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND, // 进入文件夹回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST, // 上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND, // 上传文件回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST, // 下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND, // 下载文件回复

    ENUM_MSG_TYPE_DELETE_FILE_REQUEST, // 删除文件请求
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND, // 删除文件回复

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST, // 分享文件请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND, // 分享文件回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTE, // 分享文件通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_ACCEPT, // 分享文件通知-接收

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST, // 移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND, // 移动文件回复

    ENUM_MSG_TYPE_MAX = 0x00ffffff

};

// 协议设计 弹性结构体
struct PDU
{
    uint uiPDULen; // 总的协议数据单元大小
    uint uiMsgType; // 消息类型
    char caData[64];
    uint uiMsgLen; // 实际消息长度
    int caMsg[]; // 实际消息
};

struct FileInfo {
    char fileName[32]; // 文件名字
    int fileType; // 文件类型
};


// 初始化结构体PDU
PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
