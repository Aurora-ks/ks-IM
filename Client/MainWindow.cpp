#include <QJsonObject>
#include <Ela/ElaMessageBar.h>
#include "MainWindow.h"
#include "logger.h"
#include "net.h"
#include "user.h"
#include "UserPage.h"
#include "SettingPage.h"
#include "RelationPage.h"

MainWindow::MainWindow(const QString &uid, QWidget *parent)
    : ElaWindow(parent),
      http_(new Net(NetType::HTTP)),
      ws_(new Net(NetType::WS)),
      user_(new User()){
    setWindowTitle("ks-im");
    setWindowIcon(QIcon(":/images/resource/pic/Cirno.png"));
    setNavigationBarDisplayMode(ElaNavigationType::Compact);
    bindUser(uid);
    initContent();
}

MainWindow::~MainWindow() {
    ws_->disconnect();
    delete http_;
    delete ws_;
    delete user_;
}

void MainWindow::bindUser(const QString &uid) {
    QMap<QString, QString> query;
    query["id"] = uid;
    auto resp = http_->getToUrl(QUrl(HTTP_PREFIX"/user"), query);
    LOG_INFO("u[{}] c[MainWindow::BindUser] send get user info request", uid.toStdString());

    if(!resp) {
        LOG_ERROR("u[{}] c[MainWindow::BindUser] get user info failed", uid.toStdString());
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
    user_->setUserID(QString::number(usr["id"].toInt()));
    user_->setUserName(usr["name"].toString());
    user_->setGender(static_cast<User::Gender>(usr["gender"].toInt()));
    user_->setEmail(usr["email"].toString());
    user_->setPhone(usr["phone"].toString());
    // TODO: 读取图片
    // user_->setAvatarFromB64(usr["avatar"].toString());
    user_->setAvatar(QPixmap(":/images/resource/pic/Avatar.png"));
    updateUserInfo();
}

void MainWindow::updateUserInfo() {
    setUserInfoCardPixmap(user_->getAvatar());
    setUserInfoCardTitle(user_->getUserName());
    setUserInfoCardSubTitle(user_->getUserID());
}

void MainWindow::initContent() {
    relationPage_ = new RelationPage(this);
    userPage_ = new UserPage(this);
    settingPage_ = new SettingPage(this);
    addPageNode("Relation", relationPage_, ElaIconType::CircleUser);
    addFooterNode("User", userPage_, userKey_, 0, ElaIconType::User);
    addFooterNode("Setting", settingPage_, settingKey_, 0, ElaIconType::GearComplex);
    connect(this, &MainWindow::userInfoCardClicked, this, [this]() {
        this->navigation(userKey_);
    });
}
