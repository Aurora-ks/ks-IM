#ifndef USERPAGE_H
#define USERPAGE_H

#include <Ela/ElaScrollPage.h>

class MainWindow;
class QLabel;
class ElaLineEdit;
class ElaRadioButton;
class ElaPushButton;

class UserPage : public ElaScrollPage{
public:
    UserPage(QWidget *parent = nullptr);
private slots:
    void updateInfo();
    void modifyUserInfo();
private:
    QLabel *avatar_{nullptr};
    ElaText *userID_{nullptr};
    ElaLineEdit *userName_{nullptr};
    ElaLineEdit *userEmail_{nullptr};
    ElaLineEdit *userPhone_{nullptr};
    ElaRadioButton *unkonwGender_{nullptr};
    ElaRadioButton *maleGender_{nullptr};
    ElaRadioButton *femaleGender_{nullptr};
    ElaPushButton *resetButton_{nullptr};
    ElaPushButton *saveButton_{nullptr};
    MainWindow *window_{nullptr};
    int gender_{0};

    void initLayout();
};



#endif //USERPAGE_H
