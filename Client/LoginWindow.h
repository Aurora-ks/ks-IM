#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <ElaWidget.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaCheckBox.h>
#include <ElaIconButton.h>
#include <ElaComboBox.h>
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
