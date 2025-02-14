#include "RegisterWindow.h"
#include <QBoxLayout>

RegisterWindow::RegisterWindow(QWidget *parent): ElaWidget(parent) {
    setWindowTitle("注册");
    setWindowModality(Qt::ApplicationModal);
    setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
    initUI();
}

RegisterWindow::~RegisterWindow() {
}

void RegisterWindow::togglePasswordVisibility() {
    QObject *obj = sender();
    ElaIconButton *target = qobject_cast<ElaIconButton *>(obj);
    if (target == showPasswordButton_) {
        if (passwordEdit_->echoMode() == QLineEdit::Password) {
            passwordEdit_->setEchoMode(QLineEdit::Normal);
            showPasswordButton_->setAwesome(ElaIconType::EyeSlash);
        } else {
            passwordEdit_->setEchoMode(QLineEdit::Password);
            showPasswordButton_->setAwesome(ElaIconType::Eye);
        }
    } else {
        if (passwordReEdit_->echoMode() == QLineEdit::Password) {
            passwordReEdit_->setEchoMode(QLineEdit::Normal);
            showPasswordReButton_->setAwesome(ElaIconType::EyeSlash);
        } else {
            passwordReEdit_->setEchoMode(QLineEdit::Password);
            showPasswordReButton_->setAwesome(ElaIconType::Eye);
        }
    }
}

void RegisterWindow::sendVerificationCode() {
}

void RegisterWindow::signUp() {
}

void RegisterWindow::initUI() {
    // 定义通用样式
    QString inputStyle = "ElaLineEdit {"
            "    height: 30px;"
            "}";
    QString buttonStyle = "ElaPushButton {"
            " font-size: 13px;"
            "}"
            "ElaPushButton:hover {"
            "    background-color: #0056b3;"
            "}";

    // 用户名输入框
    usernameEdit_ = new ElaLineEdit(this);
    usernameEdit_->setPlaceholderText("用户名");
    usernameEdit_->setStyleSheet(inputStyle);

    // 密码输入框和显示密码按钮
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordEdit_ = new ElaLineEdit(this);
    passwordEdit_->setPlaceholderText("密码");
    passwordEdit_->setEchoMode(ElaLineEdit::Password);
    passwordEdit_->setStyleSheet(inputStyle);

    showPasswordButton_ = new ElaIconButton(ElaIconType::Eye, this);
    showPasswordButton_->setStyleSheet("QPushButton {"
        "    padding: 3px;"
        "}");
    connect(showPasswordButton_, &QPushButton::clicked, this, &RegisterWindow::togglePasswordVisibility);
    passwordLayout->setSpacing(5);
    passwordLayout->addWidget(passwordEdit_);
    passwordLayout->addWidget(showPasswordButton_);

    QHBoxLayout *passwordReLayout = new QHBoxLayout();
    passwordReEdit_ = new ElaLineEdit(this);
    passwordReEdit_->setPlaceholderText("确认密码");
    passwordReEdit_->setEchoMode(ElaLineEdit::Password);
    passwordReEdit_->setStyleSheet(inputStyle);

    showPasswordReButton_ = new ElaIconButton(ElaIconType::Eye, this);
    showPasswordReButton_->setStyleSheet("QPushButton {"
        "    padding: 3px;"
        "}");
    connect(showPasswordReButton_, &QPushButton::clicked, this, &RegisterWindow::togglePasswordVisibility);
    passwordReLayout->setSpacing(5);
    passwordReLayout->addWidget(passwordReEdit_);
    passwordReLayout->addWidget(showPasswordReButton_);

    // 邮箱输入框和发送验证码按钮
    QHBoxLayout *emailLayout = new QHBoxLayout();
    emailEdit_ = new ElaLineEdit(this);
    emailEdit_->setPlaceholderText("邮箱");
    emailEdit_->setStyleSheet(inputStyle);

    sendCodeButton_ = new ElaPushButton("发送验证码", this);
    sendCodeButton_->setStyleSheet(buttonStyle);
    // sendCodeButton_->setFont(QFont("Microsoft YaHei", 10));
    connect(sendCodeButton_, &QPushButton::clicked, this, &RegisterWindow::sendVerificationCode);
    emailLayout->setSpacing(5);
    emailLayout->addWidget(emailEdit_);
    emailLayout->addWidget(sendCodeButton_);

    // 验证码输入框
    verificationCodeEdit_ = new ElaLineEdit(this);
    verificationCodeEdit_->setPlaceholderText("验证码");
    verificationCodeEdit_->setStyleSheet(inputStyle);

    // 注册按钮
    registerButton_ = new ElaPushButton("注册", this);
    registerButton_->setStyleSheet(buttonStyle);
    connect(registerButton_, &QPushButton::clicked, this, &RegisterWindow::signUp);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(usernameEdit_);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(passwordReLayout);
    mainLayout->addLayout(emailLayout);
    mainLayout->addWidget(verificationCodeEdit_);
    mainLayout->addWidget(registerButton_);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    setLayout(mainLayout);
}
