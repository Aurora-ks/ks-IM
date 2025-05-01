#include "RegisterWindow.h"
#include <QBoxLayout>
#include <QTimer>
#include <QButtonGroup>
#include <Ela/ElaMessageBar.h>
#include <Ela/ElaText.h>
#include "logger.h"
#include "net.h"

RegisterWindow::RegisterWindow(QWidget *parent): ElaWidget(parent), http_(new Net(NetType::HTTP)) {
    setWindowTitle("注册");
    setWindowIcon(QIcon(":/images/resource/pic/Cirno.png"));
    setWindowModality(Qt::ApplicationModal);
    setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
    initUI();
    connect(unknowGenderRadio_, &ElaRadioButton::toggled, [this](bool checked) {
        if(checked) gender_ = 0;
    });
    connect(maleGenderRadio_, &ElaRadioButton::toggled, [this](bool checked) {
        if(checked) gender_ = 1;
    });
    connect(femaleGenderRadio_, &ElaRadioButton::toggled, [this](bool checked) {
        if(checked) gender_ = 2;
    });
    connect(showPasswordButton_, &ElaPushButton::clicked, this, &RegisterWindow::togglePasswordVisibility);
    connect(showPasswordReButton_, &ElaPushButton::clicked, this, &RegisterWindow::togglePasswordVisibility);
    connect(sendCodeButton_, &ElaPushButton::clicked, this, &RegisterWindow::sendVerificationCode);
    connect(registerButton_, &ElaPushButton::clicked, this, &RegisterWindow::signUp);
}

RegisterWindow::~RegisterWindow() {
    delete http_;
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
    QString email = emailEdit_->text();
    if(email.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "请输入邮箱", 2000, this);
        return;
    }
    // 设置定时器
    if (verifyButtonTimer_ == nullptr) {
        verifyButtonTimer_ = new QTimer(this);
        connect(verifyButtonTimer_, &QTimer::timeout, this, &RegisterWindow::onTimeout);
    }
    sendCodeButton_->setEnabled(false);
    verifyButtonInterval_ = 60;
    sendCodeButton_->setText(QString("%1s").arg(verifyButtonInterval_));
    verifyButtonTimer_->start(1000);
    // 发送请求
    QMap<QString, QString> query;
    query["email"] = email;
    auto resp = http_->getToUrl(QUrl(HTTP_PREFIX"/user/verify_code"), query);
    if(!resp) {
        LOG_WARN("[RegisterWindow::sendVerificationCode] request failed, code:{}, err:{}", resp.statusCode(), resp.errorString().toStdString());
        return;
    }
    if(!resp.data()) {
        LOG_WARN("[RegisterWindow::sendVerificationCode] request error, code:{}, msg:{}", resp.data().code(), resp.data().message().toStdString());
    }
}

void RegisterWindow::signUp() {
    QString name = usernameEdit_->text();
    QString password = passwordEdit_->text();
    QString passwordRe = passwordReEdit_->text();
    QString email = emailEdit_->text();
    QString verify = verificationCodeEdit_->text();
    if (name.isEmpty() || password.isEmpty() || passwordRe.isEmpty() || email.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "请填写完整信息", 2000, this);
        return;
    }
    if (password != passwordRe) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "两次密码不一致", 2000, this);
        return;
    }
    if(verify.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "请输入验证码", 2000, this);
        return;
    }

    QJsonObject req;
    req["name"] = name;
    req["password"] = password;
    req["email"] = email;
    req["verification"] = verify;
    req["gender"] = gender_;
    auto resp = http_->postToUrl(QUrl(HTTP_PREFIX"/user/register"), QJsonDocument(req).toJson());
    if(!resp) {
        LOG_WARN("[RegisterWindow::signUp] request failed, code:{}, err:{}", resp.statusCode(), resp.errorString().toStdString());
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "注册失败", 2000, this);
        return;
    }
    if(!resp.data()) {
        LOG_WARN("[RegisterWindow::signUp] request error, code:{}, msg:{}", resp.data().code(), resp.data().message().toStdString());
        ElaMessageBar::error(ElaMessageBarType::Top, "错误", "注册失败", 2000, this);
        return;
    }
    // TODO:优化展示
    int id = resp.data().dataJson()["id"].toInt();
    ElaMessageBar::success(ElaMessageBarType::Top, "注册成功", QString("用户ID：%1").arg(id), 5000, this);
}

void RegisterWindow::onTimeout() {
    --verifyButtonInterval_;
    if (verifyButtonInterval_ > 0) {
        sendCodeButton_->setText(QString("%1s").arg(verifyButtonInterval_));
    } else {
        verifyButtonTimer_->stop();
        sendCodeButton_->setEnabled(true);
        sendCodeButton_->setText("发送验证码");
    }
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
            "    background-color: #f1f3f5;"
            "}";
    // 性别选择
    QHBoxLayout *genderLayout = new QHBoxLayout();
    ElaText *genderText = new ElaText("性别", 16, this);
    unknowGenderRadio_ = new ElaRadioButton("未知", this);
    unknowGenderRadio_->setChecked(true);
    maleGenderRadio_ = new ElaRadioButton("男", this);
    femaleGenderRadio_ = new ElaRadioButton("女", this);

    genderLayout->setSpacing(5);
    genderLayout->addWidget(genderText);
    genderLayout->addStretch();
    genderLayout->addWidget(unknowGenderRadio_);
    genderLayout->addWidget(maleGenderRadio_);
    genderLayout->addWidget(femaleGenderRadio_);

    QButtonGroup *genderGroup = new QButtonGroup(this);
    genderGroup->addButton(unknowGenderRadio_);
    genderGroup->addButton(maleGenderRadio_);
    genderGroup->addButton(femaleGenderRadio_);
    genderGroup->setExclusive(true);

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

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(genderLayout);
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
