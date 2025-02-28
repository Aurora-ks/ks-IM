#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <Ela/ElaScrollPage.h>

class ElaComboBox;
class ElaToggleSwitch;
class ElaRadioButton;

class SettingPage : public ElaScrollPage{
public:
    explicit SettingPage(QWidget* parent = nullptr);
    ~SettingPage();

private:
    ElaComboBox* themeComboBox_{nullptr};
    ElaToggleSwitch* micaSwitchButton_{nullptr};
    ElaRadioButton* minimumButton_{nullptr};
    ElaRadioButton* compactButton_{nullptr};
    ElaRadioButton* maximumButton_{nullptr};
    ElaRadioButton* autoButton_{nullptr};
};



#endif //SETTINGPAGE_H
