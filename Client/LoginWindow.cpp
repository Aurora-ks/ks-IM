#include <QBoxLayout>
#include <Ela/ElaMessageBar.h>
#include "LoginWindow.h"
#include "logger.h"


LoginWindow::LoginWindow(QWidget *parent): ElaWidget(parent) {
    setWindowTitle("登录");
    setWindowButtonFlags(ElaAppBarType::CloseButtonHint | ElaAppBarType::MinimizeButtonHint);
    initUI();
}

LoginWindow::~LoginWindow() {
    delete registerWindow_;
    logger::instance().shutdown();
}

void LoginWindow::togglePasswordVisibility() {
    if (passwordEdit_->echoMode() == QLineEdit::Password) {
        passwordEdit_->setEchoMode(QLineEdit::Normal);
        passwordVisibilityButton_->setAwesome(ElaIconType::EyeSlash);
    } else {
        passwordEdit_->setEchoMode(QLineEdit::Password);
        passwordVisibilityButton_->setAwesome(ElaIconType::Eye);
    }
}

void LoginWindow::login() {
    QString name = usernameEdit_->currentText();
    QString password = passwordEdit_->text();
    if (name.isEmpty() || password.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "请输入用户名或密码", 2000, this);
        return;
    }
    // 发送登录请求
}

void LoginWindow::signUp() {
    if (registerWindow_ == nullptr) registerWindow_ = new RegisterWindow();
    registerWindow_->show();
}

void LoginWindow::initUI() {
    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(30, 20, 30, 20);
    // 输入框的样式表
    QString inputStyle = "ElaComboBox, ElaLineEdit{"
            "    height: 30px;"
            "}";

    // 用户名输入框
    usernameEdit_ = new ElaComboBox(this);
    usernameEdit_->setEditable(true);
    usernameEdit_->lineEdit()->setPlaceholderText("用户名");
    usernameEdit_->addItem("user");
    usernameEdit_->addItem("user2");
    usernameEdit_->setStyleSheet(inputStyle);
    mainLayout->addWidget(usernameEdit_);

    // 密码输入框
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordEdit_ = new ElaLineEdit(this);
    passwordEdit_->setPlaceholderText("密码");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setStyleSheet(inputStyle);
    passwordVisibilityButton_ = new ElaIconButton(ElaIconType::Eye, this);
    passwordVisibilityButton_->setStyleSheet("ElaIconButton {"
        "    padding: 3px;"
        "}");
    connect(passwordVisibilityButton_, &ElaIconButton::clicked, this, &LoginWindow::togglePasswordVisibility);
    passwordLayout->setSpacing(5);
    passwordLayout->addWidget(passwordEdit_);
    passwordLayout->addWidget(passwordVisibilityButton_);
    mainLayout->addLayout(passwordLayout);

    // 自动登录和记住密码复选框
    QHBoxLayout *checkBoxLayout = new QHBoxLayout();
    checkBoxLayout->setContentsMargins(10, 0, 0, 10);
    autoLoginCheckBox_ = new ElaCheckBox("自动登录", this);
    rememberPasswordCheckBox_ = new ElaCheckBox("记住密码", this);
    checkBoxLayout->addWidget(autoLoginCheckBox_);
    checkBoxLayout->addWidget(rememberPasswordCheckBox_);
    mainLayout->addLayout(checkBoxLayout);

    // 注册按钮和登录按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    registerButton_ = new ElaPushButton("注册", this);
    loginButton_ = new ElaPushButton("登录", this);
    // 按钮的样式表
    QString buttonStyle = "ElaPushButton:hover {"
            "    background-color: #0056b3;"
            "}";
    registerButton_->setStyleSheet(buttonStyle);
    loginButton_->setStyleSheet(buttonStyle);
    connect(registerButton_, &ElaPushButton::clicked, this, &LoginWindow::signUp);
    connect(loginButton_, &ElaPushButton::clicked, this, &LoginWindow::login);
    buttonLayout->addWidget(registerButton_);
    buttonLayout->addWidget(loginButton_);

    mainLayout->addLayout(buttonLayout);

    // 设置布局
    setLayout(mainLayout);
}
