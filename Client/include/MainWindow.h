#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Ela/ElaWindow.h>

class Net;
class User;
class UserPage;
class SettingPage;
class RelationPage;
class SessionPage;

class MainWindow :public ElaWindow{
    Q_OBJECT
public:
    explicit MainWindow(int64_t uid, QWidget *parent = nullptr);
    ~MainWindow();

    User* getUser() const { return user_; }
    void bindUser(int64_t uid);
    void updateUserInfo();

    Net* http() { return http_; }
    Net* ws() { return ws_; }

private:
    SessionPage *sessionPage_{nullptr};
    RelationPage *relationPage_{nullptr};
    UserPage *userPage_{nullptr};
    SettingPage *settingPage_{nullptr};
    QString settingKey_{};
    QString userKey_{};
    QString sessionKey_{};
    Net *http_{nullptr};
    Net *ws_{nullptr};
    User *user_{nullptr};

    void initContent();
};



#endif //MAINWINDOW_H
