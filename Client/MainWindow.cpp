#include <QJsonObject>
#include <Ela/ElaMessageBar.h>
#include "MainWindow.h"
#include "logger.h"
#include "net.h"
#include "user.h"
#include "UserPage.h"
#include "SettingPage.h"
#include "RelationPage.h"
#include "SessionPage.h"
#include "setting.h"

MainWindow::MainWindow(int64_t uid, QWidget *parent)
    : ElaWindow(parent),
      http_(new Net(NetType::HTTP)),
      user_(new User()){
    setWindowTitle("ks-im");
    setWindowIcon(QIcon(":/images/resource/pic/Cirno.png"));
    setting* setting = setting::getDBInstance(setting::GetUserPath() + "/setting.db");
    // TODO: use const expression
    auto [val, err] = setting->valueDB("NavigationDisplayMode", "2");
    if(err) qWarning() << "c[MainWindow::MainWindow] get setting failed";
    else{
        switch(val.toInt()) {
            case 0:
                setNavigationBarDisplayMode(ElaNavigationType::Minimal);
                break;
            case 1:
                setNavigationBarDisplayMode(ElaNavigationType::Maximal);
                break;
            case 2:
                setNavigationBarDisplayMode(ElaNavigationType::Compact);
                break;
            case 3:
                setNavigationBarDisplayMode(ElaNavigationType::Auto);
                break;
            default:
                qWarning() << "[MainWindow] load config NavigationDisplayMode invalid val:" << val;
        }
    }

    bindUser(uid);
    initContent();
    connect(this, &MainWindow::userInfoCardClicked, this, [this]() {
        this->navigation(userKey_);
    });
    connect(relationPage_, &RelationPage::sendMessageClicked, this, [this](FriendTreeViewItem *user) {
        navigation(sessionPage_->property("ElaPageKey").toString());
        sessionPage_->selectOrCreateSession(user);
    });
}

MainWindow::~MainWindow() {
    setting::close();
    delete http_;
    delete user_;
}

void MainWindow::bindUser(int64_t uid) {
    QMap<QString, QString> query;
    query["id"] = QString::number(uid);
    auto resp = http_->getToUrl(QUrl(HTTP_PREFIX"/user"), query);
    LOG_INFO("u[{}] c[MainWindow::BindUser] send get user info request", uid);

    if(!resp) {
        LOG_ERROR("u[{}] c[MainWindow::BindUser] get user info failed", uid);
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
    user_->setUserID(usr["id"].toInteger());
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
    setUserInfoCardSubTitle(user_->UserIDToString());
}

void MainWindow::initContent() {
    sessionPage_ = new SessionPage(this);
    relationPage_ = new RelationPage(this);
    userPage_ = new UserPage(this);
    settingPage_ = new SettingPage(this);
    addPageNode("Session", sessionPage_, ElaIconType::MessageLines);
    addPageNode("Relation", relationPage_, ElaIconType::CircleUser);
    addFooterNode("User", userPage_, userKey_, 0, ElaIconType::User);
    addFooterNode("Setting", settingPage_, settingKey_, 0, ElaIconType::GearComplex);
}
