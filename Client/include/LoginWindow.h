#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <Ela/ElaWidget.h>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaCheckBox.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaComboBox.h>
#include "RegisterWindow.h"

class LoginWindow : public ElaWidget {
public:
    explicit LoginWindow(QWidget *parent = nullptr);

    ~LoginWindow();

private slots:
    void togglePasswordVisibility();

    void login();

    void signUp();

private:
    ElaComboBox *usernameEdit_{nullptr};
    ElaLineEdit *passwordEdit_{nullptr};
    ElaCheckBox *autoLoginCheckBox_{nullptr};
    ElaCheckBox *rememberPasswordCheckBox_{nullptr};
    ElaPushButton *loginButton_{nullptr};
    ElaPushButton *registerButton_{nullptr};
    ElaIconButton *passwordVisibilityButton_{nullptr};
    RegisterWindow *registerWindow_{nullptr};

    void initUI();
};


#endif //LOGINWINDOW_H
