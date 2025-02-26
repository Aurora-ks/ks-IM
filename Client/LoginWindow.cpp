#include <QBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <QEvent>
#include <Ela/ElaMessageBar.h>
#include "LoginWindow.h"
#include "logger.h"
#include "net.h"
#include "setting.h"

const QString skAutoLogin = "AutoLogin";
const QString skLastLoginAccount = "LastLoginAccount";
const QString skRememberPassword = "RememberPassword";
const QString stAccounts = "Accounts";

LoginWindow::LoginWindow(QWidget *parent): ElaWidget(parent) {
    setWindowTitle("登录");
    setWindowButtonFlags(ElaAppBarType::CloseButtonHint | ElaAppBarType::MinimizeButtonHint);
    initUI();
    connect(autoLoginCheckBox_, &ElaCheckBox::clicked, [this](bool checked) {
        autoLogin_ = checked;
        if(checked) rememberPasswordCheckBox_->setChecked(true);
    });
    connect(rememberPasswordCheckBox_, &ElaCheckBox::clicked, [this](bool checked) {
        rememberPassword_ = checked;
    });
    connect(usernameEdit_, QOverload<const QString&>::of(&ElaComboBox::currentTextChanged), [this](const QString& text) {
        passwordEdit_->setText(accountList_.value(text));
    });
    usernameEdit_->installEventFilter(this);
    passwordEdit_->installEventFilter(this);
    autoLoginCheckBox_->installEventFilter(this);
    rememberPasswordCheckBox_->installEventFilter(this);
    registerButton_->installEventFilter(this);
    loginButton_->installEventFilter(this);

    loadConf();
    if(autoLogin_) {
        autoLoginTimer_ = new QTimer(this);
        autoLoginTimer_->setSingleShot(true);
        connect(autoLoginTimer_, &QTimer::timeout, this, &LoginWindow::login);
    }
    initContent();
}

LoginWindow::~LoginWindow() {
    delete registerWindow_;
    delete db_;
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
    LOG_INFO("send login request");
    Net http(NetType::HTTP, QUrl(HTTP_PREFIX"/user/login"));
    QJsonObject json;
    json["id"] = name.toInt();
    json["password"] = password;
    auto resp = http.post(QJsonDocument(json).toJson(QJsonDocument::Compact));
    if(!resp) {
        qWarning() << QString("u[%1], c[LoginWindow::login] login request failed, code:%2, err:%3").arg(name).arg(resp.statusCode()).arg(resp.errorString());
        ElaMessageBar::error(ElaMessageBarType::Top, "登录失败", "网络错误", 2000, this);
        return;
    }

    auto resJson = resp.data();
    if(!resJson) {
        qWarning() << "[LoginWindow::login] login falied, code:" << resJson.code() << ", message:" << resJson.message();
        ElaMessageBar::error(ElaMessageBarType::Top, "登录失败", "用户名或密码错误", 2000, this);
        return;
    }
    QJsonObject user = resJson.dataJson();
    storeConf();
    emit loginSuccess(QString::number(user["id"].toInt()));
}

void LoginWindow::signUp() {
    if (registerWindow_ == nullptr) registerWindow_ = new RegisterWindow();
    registerWindow_->show();
}

void LoginWindow::showEvent(QShowEvent *event) {
    ElaWidget::showEvent(event);
    if(autoLogin_) {
        ElaMessageBar::information(ElaMessageBarType::Top, "自动登录", "2秒后自动登录，点击任意按钮取消", 1000, this);
        autoLoginTimer_->start(2000);
    }
}

bool LoginWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress && autoLoginTimer_) {
        autoLoginTimer_->stop();
    }
    return ElaWidget::eventFilter(watched, event);
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

void LoginWindow::initContent() {
    autoLoginCheckBox_->setChecked(autoLogin_);
    rememberPasswordCheckBox_->setChecked(rememberPassword_);
    if(accountList_.empty()) return;
    for (auto i = accountList_.begin(); i != accountList_.end(); ++i) {
        if(i.key().isEmpty()) continue;
        usernameEdit_->addItem(i.key());
        passwordEdit_->setText(i.value());
    }
    // 设置当前选中的账户
    auto [lastLogin, err] = db_->valueDB(skLastLoginAccount, "");
    if(err) qWarning() << "登录配置读取[LastLoginAccount]错误：" << err.text();
    if(!lastLogin.isEmpty()) usernameEdit_->setCurrentText(lastLogin);
    else usernameEdit_->setCurrentIndex(0);
}

void LoginWindow::loadConf() {
    QString filePath("./data/login.db");
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    // 判断目录是否存在
    if(!dir.exists()) {
        if(dir.mkpath("."))
            qInfo() << "创建目录:" << dir.path();
        else
            qWarning() << "创建目录失败:" << dir.path();
    }

    db_ = new setting(filePath, SettingFileType::SQLite);
    auto [autoLogin, error1] = db_->valueDB(skAutoLogin, "false");
    if (error1) qWarning() << "登录配置读取[AutoLogin]错误：" << error1.text();
    if(autoLogin == "true") autoLogin_ = true;

    auto [rememberPassword, error2] = db_->valueDB(skRememberPassword, "false");
    if(error2) qWarning() << "登录配置读取[RememberPassword]错误：" << error2.text();
    if(rememberPassword == "true") rememberPassword_ = true;

    auto [accounts, error3] = db_->entries(stAccounts);
    if(error3) qWarning() << "登录配置读取[Accounts]错误：" << error3.text();
    else accountList_ = accounts;
}

void LoginWindow::storeConf() {
    if(!db_) return;
    SettingError error;
    error = db_->setValueDB(skAutoLogin, autoLogin_ ? "true" : "false");
    if(error) qWarning() << "存储登录配置[AutoLogin]错误：" << error.text();
    error = db_->setValueDB(skLastLoginAccount, usernameEdit_->currentText());
    if(error) qWarning() << "存储登录配置[LastLoginAccount]错误：" << error.text();
    error = db_->setValueDB(skRememberPassword, rememberPassword_ ? "true" : "false");
    if(error) qWarning() << "存储登录配置[RememberPassword]错误：" << error.text();
    if(!accountList_.contains(usernameEdit_->currentText()))
        accountList_.insert(usernameEdit_->currentText(), rememberPassword_ ? passwordEdit_->text() : "");
    else
        accountList_[usernameEdit_->currentText()] = rememberPassword_? passwordEdit_->text() : "";
    error = db_->setEntries(accountList_, stAccounts);
    if(error) qWarning() << "存储登录配置[Accounts]错误：" << error.text();
}
