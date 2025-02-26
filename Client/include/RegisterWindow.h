#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <Ela/ElaWidget.h>
#include <Ela/ElaLineEdit.h>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaRadioButton.h>
#include <QTimer>

class Net;

class RegisterWindow : public ElaWidget {
public:
    explicit RegisterWindow(QWidget *parent = nullptr);

    ~RegisterWindow();

private slots:
    void togglePasswordVisibility();

    void sendVerificationCode();

    void signUp();

    void onTimeout();
private:
    ElaRadioButton *unknowGenderRadio_{nullptr};
    ElaRadioButton *maleGenderRadio_{nullptr};
    ElaRadioButton *femaleGenderRadio_{nullptr};
    ElaLineEdit *usernameEdit_{nullptr};
    ElaLineEdit *passwordEdit_{nullptr};
    ElaLineEdit *passwordReEdit_{nullptr};
    ElaIconButton *showPasswordButton_{nullptr};
    ElaIconButton *showPasswordReButton_{nullptr};
    ElaLineEdit *emailEdit_{nullptr};
    ElaPushButton *sendCodeButton_{nullptr};
    ElaLineEdit *verificationCodeEdit_{nullptr};
    ElaPushButton *registerButton_{nullptr};
    QTimer *verifyButtonTimer_{nullptr};
    Net *http_;
    int verifyButtonInterval_{60}; // second
    int gender_;

    void initUI();
};

#endif //REGISTERWINDOW_H
