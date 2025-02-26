#include <QJsonObject>
#include <Ela/ElaMessageBar.h>
#include "MainWindow.h"
#include "logger.h"

MainWindow::MainWindow(QWidget *parent)
    : ElaWindow(parent),
      http_(NetType::HTTP),
      ws_(NetType::WS) {
    setNavigationBarDisplayMode(ElaNavigationType::Compact);
}

MainWindow::~MainWindow() {
    ws_.disconnect();
}

void MainWindow::bindUser(const QString &uid) {
    QMap<QString, QString> query;
    query["id"] = uid;
    auto resp = http_.getToUrl(QUrl(HTTP_PREFIX"/user"), query);
    LOG_INFO("u[{}] c[MainWindow::BindUser] send get user info request", uid.toStdString());

    if(!resp) {
        LOG_ERROR("[u{}] c[MainWindow::BindUser] get user info failed", uid.toStdString());
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "获取用户信息失败", 2000, this);
        return;
    }

    HttpJson data = resp.data();
    if(!data) {
        qWarning() << QString("u[%1] c[MainWindow::BindUser] get user info failed, code:%2, message:%3").arg(uid).arg(data.code()).arg(data.message());
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "获取用户信息失败", 2000, this);
        return;
    }
    QJsonObject usr = data.dataJson();
    user_.setUserID(QString::number(usr["id"].toInt()));
    user_.setUserName(usr["name"].toString());
    user_.setGender(static_cast<User::Gender>(usr["gender"].toInt()));
    user_.setEmail(usr["email"].toString());
    user_.setPhone(usr["phone"].toString());
    user_.setAvatarFromB64(usr["avatar"].toString());
}

void MainWindow::updateUserInfo() {
    // TODO: 读取图片
    setUserInfoCardPixmap(QPixmap(":/images/resource/pic/Avatar.png"));
    setUserInfoCardTitle(user_.getUserName());
    setUserInfoCardSubTitle(user_.getUserID());
    update();
}
