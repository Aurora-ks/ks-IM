#include "UserInfoCard.h"
#include <QLabel>
#include <QBoxLayout>
#include <QMap>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaComboBox.h>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaText.h>
#include <Ela/ElaMessageBar.h>
#include "user.h"
#include "FriendTreeViewItem.h"
#include "net.h"

UserInfoCard::UserInfoCard(QWidget *parent) : QWidget(parent), user_(new FriendTreeViewItem) {
    initLayout();
    updateGrouping();
    connect(aliasLineEdit_, &ElaLineEdit::editingFinished, this, &UserInfoCard::updateAlias);
    connect(groupingComboBox_, &ElaComboBox::currentIndexChanged, this, &UserInfoCard::updateUserGrouping);
    connect(communicateButton_, &ElaPushButton::clicked, [this](){
       emit sendMessageClicked(user_);
    });
}

void UserInfoCard::initLayout() {
    groupingMap_.insert(0, "我的好友");

    // avatar
    avatar_ = new QLabel(this);
    avatar_->setFixedSize(100, 100);
    QHBoxLayout *avatarHLayout = new QHBoxLayout();
    avatarHLayout->setContentsMargins(0, 0, 0, 0);
    avatarHLayout->addWidget(avatar_);

    // name and id text
    name_ = new ElaText(this);
    name_->setTextPixelSize(14);
    name_->setTextStyle(ElaTextType::TextStyle::Title);
    id_ = new ElaText(this);
    id_->setTextPixelSize(12);
    id_->setTextStyle(ElaTextType::TextStyle::Subtitle);

    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->addWidget(name_);
    nameLayout->addWidget(id_);
    avatarHLayout->addLayout(nameLayout);

    // alias edit
    ElaText *aliasText = new ElaText("备注", 14, this);
    aliasLineEdit_ = new ElaLineEdit(this);
    aliasLineEdit_->setPlaceholderText("设置好友备注");

    QHBoxLayout *aliasHLayout = new QHBoxLayout();
    aliasHLayout->setContentsMargins(0, 0, 0, 0);
    aliasHLayout->addWidget(aliasText);
    aliasHLayout->addStretch();
    aliasHLayout->addWidget(aliasLineEdit_);

    // grouping combo box
    ElaText *groupingText = new ElaText("好友分组", 14, this);
    groupingComboBox_ = new ElaComboBox(this);
    groupingComboBox_->addItem("我的好友", 0);
    QHBoxLayout *groupingHLayout = new QHBoxLayout();
    groupingHLayout->setContentsMargins(0, 0, 0, 0);
    groupingHLayout->addWidget(groupingText);
    groupingHLayout->addStretch();
    groupingHLayout->addWidget(groupingComboBox_);

    // button
    communicateButton_ = new ElaPushButton("发消息", this);
    QHBoxLayout *communicateHLayout = new QHBoxLayout();
    communicateHLayout->setContentsMargins(0, 0, 0, 0);
    communicateHLayout->addWidget(communicateButton_);
    communicateHLayout->addStretch();

    // main layout
    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(50, 30, 50, 0);
    mainVLayout->addLayout(avatarHLayout);
    mainVLayout->addLayout(aliasHLayout);
    mainVLayout->addLayout(groupingHLayout);
    mainVLayout->addLayout(communicateHLayout);
    mainVLayout->addStretch();
}

void UserInfoCard::updateUserInfo(FriendTreeViewItem *user) {
    user_ = user;
    if(user_ == nullptr){
        qWarning("[UserInfoCard::updateUserInfo] user_ is nullptr");
        return;
    }

    if(user->getUser().getUserID() % 2 == 0){
        QPixmap pic(":/images/resource/pic/Cirno.png");
        avatar_->setPixmap(pic.scaled(100, 100, Qt::KeepAspectRatio));
    }else{
        QPixmap pic(":/images/resource/pic/Avatar.png");
        avatar_->setPixmap(pic.scaled(100, 100, Qt::KeepAspectRatio));
    }
    name_->setText(user_->getUser().getUserName());
    id_->setText(user_->getUser().UserIDToString());
    aliasLineEdit_->setText(user_->getAlias());
    groupingComboBox_->setCurrentText(groupingMap_.value(user_->getGroupId()));
}

void UserInfoCard::updateGrouping() {
    QMap<QString, QString> query;
    query["uid"] = QString::number(User::GetUid());
    auto resp = Net::GetTo("/rel/friend_grouping", query);
    if(!resp){
        qWarning() << "[Net] [UserInfoCard::updateGrouping] update grouping net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [UserInfoCard::updateGrouping] update grouping error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto groupingData = json.dataArray();
    for(const auto &value : groupingData){
        if(value.isObject()){
            auto obj = value.toObject();
            QString name = obj.value("name").toString();
            int64_t  id = obj.value("id").toInteger();
            groupingComboBox_->addItem(name, id);
            groupingMap_.insert(id, name);
        }
    }
}

void UserInfoCard::updateAlias() {
    QString oldAlias = user_->getAlias();
    user_->setAlias(aliasLineEdit_->text());

    QJsonObject data;
    data["id"] = user_->getRelationId();
    data["alias"] = user_->getAlias();

    auto resp = Net::PostTo("/rel/modify-friend-alias", QJsonDocument(data).toJson());
    if(!resp){
        qWarning() << "[Net] [FriendListModel::updateFriendAlias] update alias net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "修改好友备注失败", 2000);
        user_->setAlias(oldAlias);
        aliasLineEdit_->setText(oldAlias);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [FriendListModel::updateFriendAlias] update alias error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "修改好友备注失败", 2000);
        user_->setAlias(oldAlias);
        aliasLineEdit_->setText(oldAlias);
        return;
    }
}

void UserInfoCard::updateUserGrouping(int index) {
    int64_t newGroupId = groupingComboBox_->itemData(index).toLongLong();
    int64_t oldGroupId = user_->getGroupId();
    if(newGroupId == oldGroupId) return;
    int64_t relationId = user_->getRelationId();

    QJsonObject data;
    data["id"] = relationId;
    data["group_id"] = newGroupId;

    auto resp = Net::PostTo("/rel/modify-friend-group", QJsonDocument(data).toJson());
    if(!resp){
        qWarning() << "[Net] [UserInfoCard::onGroupingComboBoxIndexChanged] update user grouping net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "修改好友分组失败", 2000);
        groupingComboBox_->setCurrentText(groupingMap_.value(oldGroupId));
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [UserInfoCard::onGroupingComboBoxIndexChanged] update user grouping error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "修改好友分组失败", 2000);
        groupingComboBox_->setCurrentText(groupingMap_.value(oldGroupId));
        return;
    }

    user_->setGroupId(newGroupId);
    emit groupingChanged(oldGroupId, user_);
}

void UserInfoCard::updateGroupingList() {
    // 清空现有分组
    groupingComboBox_->clear();
    groupingMap_.clear();

    // 添加默认分组
    groupingComboBox_->addItem("我的好友", 0);
    groupingMap_.insert(0, "我的好友");

    // 获取最新分组列表
    QMap<QString, QString> query;
    query["uid"] = QString::number(User::GetUid());
    auto resp = Net::GetTo("/rel/friend_grouping", query);
    if(!resp){
        qWarning() << "[Net] [UserInfoCard::updateGroupingList] update grouping list net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [UserInfoCard::updateGroupingList] update grouping list error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto groupingData = json.dataArray();
    for(const auto &value : groupingData){
        if(value.isObject()){
            auto obj = value.toObject();
            QString name = obj.value("name").toString();
            int64_t id = obj.value("id").toInteger();
            groupingComboBox_->addItem(name, id);
            groupingMap_.insert(id, name);
        }
    }

    // 如果当前有选中的用户，更新其分组显示
    if(user_ != nullptr) {
        groupingComboBox_->setCurrentText(groupingMap_.value(user_->getGroupId()));
    }
}