#include "RelationPage.h"
#include <QSplitter>
#include <QStackedWidget>
#include <QBoxLayout>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaScrollPage.h>
#include <Ela/ElaTreeView.h>
#include <Ela/ElaText.h>
#include <Ela/ElaTheme.h>
#include "RelationNotifyWidget.h"

RelationPage::RelationPage(QWidget *parent) : QWidget(parent) {
    setWindowTitle("好友列表");
    initLayout();
    initContent();
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

    // left notify button
    friendNotifyButton_ = new ElaPushButton("好友通知", this);
    friendNotifyButton_->setStyleSheet(buttonStyleSheet);
    groupNotifyButton_ = new ElaPushButton("群组通知", this);
    groupNotifyButton_->setStyleSheet(buttonStyleSheet);

    connect(friendNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(userNotifyPage_);
    });
    connect(groupNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(groupNotifyPage_);
    });

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
    userListView_ = new ElaTreeView(this);
    groupListView_ = new ElaTreeView(this);
    leftStacked_->setContentsMargins(0, 0, 0, 0);
    leftStacked_->addWidget(userListView_);
    leftStacked_->addWidget(groupListView_);

    // left layout
    leftVLayout->addWidget(friendNotifyButton_);
    leftVLayout->addWidget(groupNotifyButton_);
    leftVLayout->addSpacing(10);
    leftVLayout->addLayout(switchButtonHLayout);
    leftVLayout->addWidget(leftStacked_);

    // right user notify
    rightStacked_ = new QStackedWidget(this);
    rightStacked_->setContentsMargins(0, 0, 0, 0);

    userNotifyPage_ = new RelationNotifyWidget("好友通知", this);
    rightStacked_->addWidget(userNotifyPage_);

    groupNotifyPage_ = new RelationNotifyWidget("群通知", this);
    rightStacked_->addWidget(groupNotifyPage_);

    // right user info
    userInfoWidget_ = new QWidget(this);
    rightStacked_->addWidget(userInfoWidget_);

    QVBoxLayout *userInfoVLayout = new QVBoxLayout(userInfoWidget_);
    QLabel *avatar = new QLabel(this);
    avatar->setPixmap(
        QPixmap(":/images/resource/pic/Avatar.png").scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    userInfoVLayout->addWidget(avatar);

    // splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
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
    userListSwitchButton_->click();
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->addWidget(splitter);
    setLayout(mainHLayout);
}

void RelationPage::initContent() {
}
