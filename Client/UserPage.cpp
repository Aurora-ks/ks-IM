#include "UserPage.h"
#include <QBoxLayout>
#include <QButtonGroup>
#include <Ela/ElaText.h>
#include <Ela/ElaRadioButton.h>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaPushButton.h>
#include "MainWindow.h"
#include "logger.h"
#include "user.h"

const int textSize = 15;

UserPage::UserPage(QWidget *parent): ElaScrollPage(parent) {
    setWindowTitle("用户信息");
    window_ = dynamic_cast<MainWindow *>(parent);
    initLayout();
    updateInfo();
}

UserPage::~UserPage() {
    delete avatar_;
}

void UserPage::initLayout() {
    //avatar and id
    QHBoxLayout *firstRowLayout = new QHBoxLayout();
    userID_ = new ElaText(this);
    userID_->setTextPixelSize(20);
    userID_->setAlignment(Qt::AlignCenter);
    userID_->setTextStyle(ElaTextType::TextStyle::Title);
    userID_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    avatar_ = new QLabel(this);
    avatar_->setFixedSize(100, 100);
    firstRowLayout->addWidget(avatar_);
    firstRowLayout->addSpacing(5);
    firstRowLayout->addWidget(userID_);
    firstRowLayout->addStretch();

    // gender
    QHBoxLayout *radioLayout = new QHBoxLayout();
    ElaText *genderText = new ElaText("性别：", textSize, this);
    unkonwGender_ = new ElaRadioButton("未知", this);
    maleGender_ = new ElaRadioButton("男", this);
    femaleGender_ = new ElaRadioButton("女", this);
    radioLayout->addWidget(genderText);
    radioLayout->addSpacing(5);
    radioLayout->addWidget(unkonwGender_);
    radioLayout->addWidget(maleGender_);
    radioLayout->addWidget(femaleGender_);
    radioLayout->addStretch();
    QButtonGroup *radioGroup = new QButtonGroup(this);
    radioGroup->addButton(unkonwGender_);
    radioGroup->addButton(maleGender_);
    radioGroup->addButton(femaleGender_);
    radioGroup->setExclusive(true);

    // username
    QHBoxLayout *usernameLayout = new QHBoxLayout();
    ElaText *usernameText = new ElaText("用户名：", textSize, this);
    userName_ = new ElaLineEdit(this);
    userName_->setFixedHeight(30);
    usernameLayout->addWidget(usernameText);
    usernameLayout->addSpacing(5);
    usernameLayout->addWidget(userName_);
    usernameLayout->addStretch();

    // email
    QHBoxLayout *emailLayout = new QHBoxLayout();
    ElaText *emailText = new ElaText("邮箱：", textSize, this);
    userEmail_ = new ElaLineEdit(this);
    userEmail_->setFixedHeight(30);
    emailLayout->addWidget(emailText);
    emailLayout->addSpacing(5);
    emailLayout->addWidget(userEmail_);
    emailLayout->addStretch();

    // phone
    QHBoxLayout *phoneLayout = new QHBoxLayout();
    ElaText *phoneText = new ElaText("手机号：", textSize, this);
    userPhone_ = new ElaLineEdit(this);
    userPhone_->setFixedHeight(30);
    phoneLayout->addWidget(phoneText);
    phoneLayout->addSpacing(5);
    phoneLayout->addWidget(userPhone_);
    phoneLayout->addStretch();

    // button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    resetButton_ = new ElaPushButton("重置", this);
    saveButton_ = new ElaPushButton("保存", this);
    buttonLayout->addWidget(resetButton_);
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addStretch();
    // 按钮的样式表
    QString buttonStyle = "ElaPushButton:hover {"
            "    background-color: #0056b3;"
            "}";
    resetButton_->setStyleSheet(buttonStyle);
    saveButton_->setStyleSheet(buttonStyle);
    connect(resetButton_, &ElaPushButton::clicked, this, [this]() {
        updateInfo();
    });
    connect(saveButton_, &ElaPushButton::clicked, this, [this]() {
        LOG_INFO("c[UserPage::saveButton] send user info modify request");
        //TODO: send request
    });

    // 整体布局
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("用户信息");
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,20,20);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(firstRowLayout);
    mainLayout->setSpacing(30);
    mainLayout->addLayout(radioLayout);
    mainLayout->setSpacing(30);
    mainLayout->addLayout(usernameLayout);
    mainLayout->setSpacing(30);
    mainLayout->addLayout(emailLayout);
    mainLayout->setSpacing(30);
    mainLayout->addLayout(phoneLayout);
    mainLayout->setSpacing(30);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
    addCentralWidget(centralWidget, true, true, 0);
    // setCustomWidget(customWidget);
}

void UserPage::updateInfo() {
    User *user = window_->getUser();
    avatar_->setPixmap(user->getAvatar().scaled(100, 100, Qt::KeepAspectRatio));
    userID_->setText(QString("UID:%1").arg(user->getUserID()));
    userName_->setText(user->getUserName());
    userEmail_->setText(user->getEmail());
    userPhone_->setText(user->getPhone());
    if (user->getGender() == User::Gender::MALE) {
        maleGender_->setChecked(true);
    } else if (user->getGender() == User::Gender::FEMALE) {
        femaleGender_->setChecked(true);
    } else {
        unkonwGender_->setChecked(true);
    }
    // update();
}
