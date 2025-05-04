#include "RelationPage.h"
#include <QSplitter>
#include <QStackedWidget>
#include <QBoxLayout>
#include <QStandardItem>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaTreeView.h>
#include <Ela/ElaText.h>
#include <Ela/ElaTheme.h>
#include <Ela/ElaScrollBar.h>
#include <Ela/ElaMessageBar.h>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaToolButton.h>
#include <Ela/ElaMenu.h>
#include "RelationNotifyWidget.h"
#include "FriendListModel.h"
#include "FriendTreeViewItem.h"
#include "UserInfoCard.h"
#include "net.h"

RelationPage::RelationPage(QWidget *parent) : QWidget(parent) {
    setWindowTitle("好友列表");
    initLayout();
    initContent();

    connect(friendNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(userNotifyPage_);
    });
    connect(groupNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(groupNotifyPage_);
    });
    connect(userListSwitchButton_, &QPushButton::clicked, [this]() {
        userListSwitchButton_->setIcon(*this->userBluePic_);
        groupListSwitchButton_->setIcon(*this->userGroupPic_);
        leftStacked_->setCurrentWidget(userListView_);
    });
    connect(groupListSwitchButton_, &QPushButton::clicked, [this]() {
        userListSwitchButton_->setIcon(*this->userPic_);
        groupListSwitchButton_->setIcon(*this->userGroupBluePic_);
        leftStacked_->setCurrentWidget(groupListView_);
    });
    connect(userListView_, &ElaTreeView::clicked, this, &RelationPage::showFriendInfo);
    connect(userInfoWidget_, &UserInfoCard::groupingChanged, userListModel_, &FriendListModel::updateFriendGrouping);
    connect(groupListView_, &ElaTreeView::clicked, this, &RelationPage::showGroupInfo);
    connect(eTheme, &ElaTheme::themeModeChanged, [this](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light){
            userListView_->setStyleSheet("ElaTreeView::item{height:50px;}");
            groupListView_->setStyleSheet("ElaTreeView::item{height:50px;}");
        }
        else{
            userListView_->setStyleSheet("ElaTreeView{background-color:#343434;}ElaTreeView::item{height:50px;}");
            groupListView_->setStyleSheet("ElaTreeView{background-color:#343434;}ElaTreeView::item{height:50px;}");
        }
    });
    connect(userInfoWidget_, &UserInfoCard::sendMessageClicked, this, &RelationPage::sendMessageClicked);
    userListSwitchButton_->click();
}

RelationPage::~RelationPage() {
    delete userPic_;
    delete userBluePic_;
    delete userGroupPic_;
    delete userGroupBluePic_;
}

void RelationPage::initLayout() {
    QString buttonStyleSheet = "ElaPushButton{font-size:14px;}ElaPushButton:hover{background-color:#f1f3f5;}";
    QString switchButtonLightStyleSheet = "QPushButton{border:none;}QPushButton:hover{background-color:#f1f3f5;}";
    QString switchButtonDarkStyleSheet = "QPushButton{border:none;}QPushButton:hover{background-color:#444444;}";
    // left widget
    QWidget *leftWidget = new QWidget(this);
    leftWidget->setMinimumWidth(150);
    leftWidget->setMaximumWidth(300);

    QVBoxLayout *leftVLayout = new QVBoxLayout(leftWidget);
    leftVLayout->setContentsMargins(3, 2, 3, 2);

    // left add friend or group line and button
    searchEdit_ = new ElaLineEdit(this);
    addButton_ = new ElaToolButton(this);
    addButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addButton_->setElaIcon(ElaIconType::Plus);
    addButton_->setText("添加");

    ElaMenu *addMenu = new ElaMenu(this);
    QAction *addUserAction = new QAction("添加用户", this);
    QAction *addGroupAction = new QAction("添加群组", this);
    QAction *createGroupAction = new QAction("创建群组", this);
    QAction *createGroupingAction = new QAction("创建分组", this);
    connect(addUserAction, &QAction::triggered, this, &RelationPage::addUser);
    connect(addGroupAction, &QAction::triggered, this, &RelationPage::addGroup);
    connect(createGroupAction, &QAction::triggered, this, &RelationPage::createGroup);
    connect(createGroupingAction, &QAction::triggered, this, &RelationPage::createGrouping);
    addMenu->addAction(addUserAction);
    addMenu->addAction(addGroupAction);
    addMenu->addAction(createGroupAction);
    addMenu->addAction(createGroupingAction);
    addButton_->setMenu(addMenu);

    QHBoxLayout *addButtonHLayout = new QHBoxLayout();
    addButtonHLayout->setContentsMargins(0, 0, 0, 0);
    addButtonHLayout->addWidget(searchEdit_);
    addButtonHLayout->addWidget(addButton_);

    // left notify button
    friendNotifyButton_ = new ElaPushButton("好友通知", this);
    friendNotifyButton_->setStyleSheet(buttonStyleSheet);
    groupNotifyButton_ = new ElaPushButton("群组通知", this);
    groupNotifyButton_->setStyleSheet(buttonStyleSheet);

    // left switch button
    userPic_ = new QIcon(":/images/resource/pic/User.png");
    userBluePic_ = new QIcon(":/images/resource/pic/User-Blue.png");
    userGroupPic_ = new QIcon(":/images/resource/pic/UserGroup.png");
    userGroupBluePic_ = new QIcon(":/images/resource/pic/UserGroup-Blue.png");

    userListSwitchButton_ = new QPushButton(this);
    userListSwitchButton_->setIconSize(QSize(20, 20));
    userListSwitchButton_->setFixedHeight(30);
    userListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);

    groupListSwitchButton_ = new QPushButton(this);
    groupListSwitchButton_->setIconSize(QSize(20, 20));
    groupListSwitchButton_->setFixedHeight(30);
    groupListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);

    connect(eTheme, &ElaTheme::themeModeChanged,
            [this, switchButtonLightStyleSheet, switchButtonDarkStyleSheet](ElaThemeType::ThemeMode themeMode) {
                if (themeMode == ElaThemeType::ThemeMode::Light) {
                    userListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);
                    groupListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);
                } else {
                    userListSwitchButton_->setStyleSheet(switchButtonDarkStyleSheet);
                    groupListSwitchButton_->setStyleSheet(switchButtonDarkStyleSheet);
                }
            });

    QHBoxLayout *switchButtonHLayout = new QHBoxLayout();
    switchButtonHLayout->setContentsMargins(5, 0, 5, 0);
    switchButtonHLayout->addWidget(userListSwitchButton_);
    switchButtonHLayout->addWidget(groupListSwitchButton_);

    // left tree widget
    leftStacked_ = new QStackedWidget(this);
    leftStacked_->setAutoFillBackground(false);
    leftStacked_->setStyleSheet("QStackedWidget:{background-color: transparent;}");

    userListView_ = new ElaTreeView(this);
    ElaScrollBar *userScrollBar = new ElaScrollBar(userListView_->verticalScrollBar(), userListView_);
    userScrollBar->setIsAnimation(true);
    userListView_->setHeaderHidden(true);
    userListView_->setItemDelegate(new NoChildIndentDelegate(this));
    userListView_->setIconSize(QSize(40, 40));

    userListModel_ = new FriendListModel(this);
    userListView_->setModel(userListModel_);

    groupListView_ = new ElaTreeView(this);
    ElaScrollBar *groupScrollBar = new ElaScrollBar(groupListView_->verticalScrollBar(), groupListView_);
    groupScrollBar->setIsAnimation(true);
    groupListView_->setHeaderHidden(true);
    groupListView_->setItemDelegate(new NoChildIndentDelegate(this));
    groupListView_->setIconSize(QSize(40, 40));
    // TODO: group list model
    groupListModel_ = new FriendListModel(this);
    groupListView_->setModel(groupListModel_);

    leftStacked_->setContentsMargins(0, 0, 0, 0);
    leftStacked_->addWidget(userListView_);
    leftStacked_->addWidget(groupListView_);

    // left layout
    leftVLayout->addLayout(addButtonHLayout);
    leftVLayout->addWidget(friendNotifyButton_);
    leftVLayout->addWidget(groupNotifyButton_);
    leftVLayout->addSpacing(10);
    leftVLayout->addLayout(switchButtonHLayout);
    leftVLayout->addWidget(leftStacked_);

    // right user notify
    rightStacked_ = new QStackedWidget(this);
    rightStacked_->setAutoFillBackground(false);
    rightStacked_->setStyleSheet("QStackedWidget:{background-color: transparent;}");
    rightStacked_->setContentsMargins(0, 0, 0, 0);

    userNotifyPage_ = new RelationNotifyWidget("好友通知", this);
    rightStacked_->addWidget(userNotifyPage_);

    groupNotifyPage_ = new RelationNotifyWidget("群通知", this);
    rightStacked_->addWidget(groupNotifyPage_);

    // right user info
    userInfoWidget_ = new UserInfoCard(this);
    rightStacked_->addWidget(userInfoWidget_);

    // splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightStacked_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);
    connect(eTheme, &ElaTheme::themeModeChanged, [splitter](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light)
            splitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
        else
            splitter->setStyleSheet("QSplitter::handle{background-color:#444444;}");
    });

    // main layout
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->addWidget(splitter);
    setLayout(mainHLayout);
}

void RelationPage::initContent() {
    getUserList();
    getGroupList();
}

void RelationPage::showFriendInfo(const QModelIndex &index) {
    if (!index.parent().isValid()) return;

    QStandardItem *friendItem = dynamic_cast<QStandardItemModel*>(userListView_->model())->itemFromIndex(index);
    if (!friendItem) return;

    FriendTreeViewItem *item = friendItem->data(FriendListModel::DataRole).value<FriendTreeViewItem*>();
    if(!item) return;
    userInfoWidget_->updateUserInfo(item);
    rightStacked_->setCurrentWidget(userInfoWidget_);
}

void RelationPage::showGroupInfo(const QModelIndex &index) {
    if (!index.parent().isValid()) return;

    QStandardItem *friendItem = dynamic_cast<QStandardItemModel*>(groupListView_->model())->itemFromIndex(index);
    if (!friendItem) return;

    FriendTreeViewItem *item = friendItem->data(FriendListModel::DataRole).value<FriendTreeViewItem*>();
    if(!item) return;
    userInfoWidget_->updateUserInfo(item);
    rightStacked_->setCurrentWidget(userInfoWidget_);
}

void RelationPage::addGroup() {
    QString groupId = searchEdit_->text();
    if(groupId.isEmpty()) return;
    searchEdit_->clear();

//    QJsonObject json;
//    json["user_id"] = User::GetUid();
//    json["group_id"] = groupId.toLongLong();
//    auto resp = Net::PostTo("/grp/join", QJsonDocument(json).toJson());
//    if(!resp){
//        qWarning() << "[Net] [RelationPage::addGroup] join group net work failed, err:" << resp.errorString();
//        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "加入群组失败", 2000);
//        return;
//    }
//
//    auto data = resp.data();
//    if(!data){
//        qWarning() << "[Net] [RelationPage::addGroup] join group error, code: " << data.code() << ", message: " << data.message();
//        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "加入群组失败", 2000);
//        return;
//    }
//
//    // 刷新群组列表
//    QMap<QString, QString> query;
//    query.insert("uid", QString::number(User::GetUid()));
//    resp = Net::GetTo("/grp/list", query);
//    if(!resp || !resp.data()) {
//        qWarning() << "[Net] [RelationPage::addGroup] refresh group list failed";
//        return;
//    }
//
//    json = resp.data();
//    groupListModel_->clear();
//
//    auto groupListData = json.dataArray();
//    for(const auto &value : groupListData){
//        if(value.isObject()){
//            auto obj = value.toObject();
//            QStandardItem *item = new QStandardItem();
//
//            QString groupName = obj.value("name").toString();
//            QString groupIcon = obj.value("icon").toString();
//            int64_t groupId = obj.value("id").toInteger();
//
//            item->setText(groupName);
//            item->setIcon(QIcon(groupIcon));
//            item->setData(groupId, Qt::UserRole);
//
//            groupListModel_->appendRow(item);
//        }
//    }
}

void RelationPage::addUser() {
    QString id = searchEdit_->text();
    if(id.isEmpty()) return;
    searchEdit_->clear();

    QJsonObject json;
    json["user_id"] = User::GetUid();
    json["friend_id"] = id.toLongLong();
    json["remark"] = "";
    auto resp = Net::PostTo("/rel/add-friend", QJsonDocument(json).toJson());
    if(!resp){
        qWarning() << "[Net] [RelationPage::addUser] add user net work failed, err:" << resp.errorString();
        return;
    }
    auto data = resp.data();
    if(!data){
        qWarning() << "[Net] [RelationPage::addUser] add user error, code: " << data.code() << ", message: " << data.message();
        return;
    }
    auto dataJson = data.dataJson();
    if(dataJson.isEmpty()){
        qWarning() << "[Net] [RelationPage::addUser] add user received invalid";
        return;
    }
    // TODO: refresh
    userListModel_->clear();
    getUserList();
}

void RelationPage::createGroup() {
    QString name = searchEdit_->text();
    if(name.isEmpty()) return;
    searchEdit_->clear();

    QJsonObject json;
    json["name"] = name;
    json["create_id"] = User::GetUid();
    json["public"] = 1;

    auto resp = Net::PostTo("/grp/new", QJsonDocument(json).toJson());
    if(!resp){
        qWarning() << "[Net] [RelationPage::createGroup] create group net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建群组失败", 2000);
        return;
    }

    auto data = resp.data();
    if(!data){
        qWarning() << "[Net] [RelationPage::createGroup] create group error, code: " << data.code() << ", message: " << data.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建群组失败", 2000);
        return;
    }
    // TODO: refresh
    groupListModel_->clear();
    getGroupList();
}

void RelationPage::createGrouping() {
    QString name = searchEdit_->text();
    if(name.isEmpty()) return;
    searchEdit_->clear();

    QJsonObject json;
    json["user_id"] = User::GetUid();
    json["name"] = name;

    auto resp = Net::PostTo("/rel/add-group", QJsonDocument(json).toJson());
    if(!resp){
        qWarning() << "[Net] [RelationPage::createGrouping] create grouping net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建分组失败", 2000);
        return;
    }

    auto data = resp.data();
    if(!data){
        qWarning() << "[Net] [RelationPage::createGrouping] create grouping error, code: " << data.code() << ", message: " << data.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建分组失败", 2000);
        return;
    }

    // 刷新分组列表
    userListModel_->clear();
    getUserList();

    // 更新UserInfoCard中的分组列表
    userInfoWidget_->updateGroupingList();
}

void RelationPage::getGroupList() {
    // 获取群组
    // TODO: 完善群组展示
    groupListModel_->addGroup(0, "我的群组");
    QMap<QString, QString> query;
    query["uid"] = QString::number(User::GetUid());
    auto resp = Net::GetTo("/grp/list", query);
    if(!resp){
        qWarning() << "[Net] [RelationPage::initContent] get group list net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取群组列表失败", 2000);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [RelationPage::initContent] get group list error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取群组列表失败", 2000);
        return;
    }
    auto groupListData = json.dataArray();
    for(const auto &value : groupListData){
        if(value.isObject()){
            auto obj = value.toObject();
            FriendTreeViewItem *item = new FriendTreeViewItem();
            item->setRelationId(obj.value("id").toInteger());
            int64_t groupId = obj.value("group_id").toInteger();
            // 这个id是分组的id，这里当成群组id
            item->setGroupId(0);
            item->setUser(User(groupId, "grp", 1, "", "", ""));
            groupListModel_->addFriend(item);
        }
    }
}

void RelationPage::getUserList() {
    // 获取组列表
    userListModel_->addGroup(0, "我的好友");
    QMap<QString, QString> query;
    query.insert("uid", QString::number(User::GetUid()));
    auto resp = Net::GetTo("/rel/friend_grouping", query);
    if(!resp){
        qWarning() << "[Net] [RelationPage::initContent] get grouping net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [RelationPage::initContent] get grouping error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto groupingData = json.dataArray();
    for(const auto &value : groupingData){
        if(value.isObject()){
            auto obj = value.toObject();
            QString name = obj.value("name").toString();
            int64_t  id = obj.value("id").toInteger();
            userListModel_->addGroup(id, name);
        }
    }
    // 获取好友列表
    resp = Net::GetTo("/rel/friend-list", query);
    if(!resp){
        qWarning() << "[Net] [RelationPage::initContent] get friend list net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
        return;
    }

    json = resp.data();
    if(!json){
        qWarning() << "[Net] [RelationPage::initContent] get friend list error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
        return;
    }

    auto friendListData = json.dataArray();
    for(const auto &value : friendListData){
        if(value.isObject()){
            auto obj = value.toObject();
            FriendTreeViewItem *item = new FriendTreeViewItem();
            item->setRelationId(obj.value("id").toInteger());
            item->setGroupId(obj.value("group_id").toInteger());
            item->setAlias(obj.value("alias").toString());

            auto userObj = obj.value("user").toObject();
            if (userObj.isEmpty()) {
                qWarning() << "[Net] [RelationPage::initContent] get friend list error, code: " << json.code() << ", message: " << json.message();
                ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
                return;
            }
            item->setUser(User(userObj["id"].toInteger(), userObj["name"].toString(), userObj["gender"].toInt(), userObj["icon"].toString(), userObj["emal"].toString(), userObj["phone"].toString()));
            userListModel_->addFriend(item);
        }
    }
}
