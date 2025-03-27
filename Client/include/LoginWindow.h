#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <Ela/ElaWidget.h>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaCheckBox.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaComboBox.h>
#include "RegisterWindow.h"

class setting;

class LoginWindow : public ElaWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

    ~LoginWindow();
signals:
    void loginSuccess(int64_t uid);
private slots:
    void togglePasswordVisibility();

    void login();

    void signUp();
protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    ElaComboBox *usernameEdit_{nullptr};
    ElaLineEdit *passwordEdit_{nullptr};
    ElaCheckBox *autoLoginCheckBox_{nullptr};
    ElaCheckBox *rememberPasswordCheckBox_{nullptr};
    ElaPushButton *loginButton_{nullptr};
    ElaPushButton *registerButton_{nullptr};
    ElaIconButton *passwordVisibilityButton_{nullptr};
    RegisterWindow *registerWindow_{nullptr};
    QTimer *autoLoginTimer_{nullptr};
    setting *db_{nullptr};
    QMap<QString, QString> accountList_;
    bool autoLogin_{false};
    bool rememberPassword_{false};

    void initUI();
    void initContent();
    void loadConf();
    void storeConf();
};


#endif //LOGINWINDOW_H
