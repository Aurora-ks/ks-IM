#include "SettingPage.h"
#include <Ela/ElaToggleSwitch.h>
#include <Ela/ElaText.h>
#include <Ela/ElaComboBox.h>
#include <Ela/ElaScrollPageArea.h>
#include <Ela/ElaRadioButton.h>
#include <Ela/ElaTheme.h>
#include <Ela/ElaApplication.h>
#include <QBoxLayout>
#include "MainWindow.h"
#include "setting.h"

SettingPage::SettingPage(QWidget *parent): ElaScrollPage(parent) {
    setting *setting = setting::getDBInstance("./data/setting.db");

    MainWindow *window = dynamic_cast<MainWindow *>(parent);
    setWindowTitle("设置");
    ElaText *themeText = new ElaText("主题设置", this);
    themeText->setWordWrap(false);
    themeText->setTextPixelSize(18);

    themeComboBox_ = new ElaComboBox(this);
    themeComboBox_->addItem("日间模式");
    themeComboBox_->addItem("夜间模式");
    themeComboBox_->setCurrentIndex(eTheme->getThemeMode() == ElaThemeType::Light ? 0 : 1);

    ElaScrollPageArea *themeSwitchArea = new ElaScrollPageArea(this);
    QHBoxLayout *themeSwitchLayout = new QHBoxLayout(themeSwitchArea);
    ElaText *themeSwitchText = new ElaText("主题切换", this);
    themeSwitchText->setWordWrap(false);
    themeSwitchText->setTextPixelSize(15);
    themeSwitchLayout->addWidget(themeSwitchText);
    themeSwitchLayout->addStretch();
    themeSwitchLayout->addWidget(themeComboBox_);
    connect(themeComboBox_, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [this](int index) {
        if (index == 0)
            eTheme->setThemeMode(ElaThemeType::Light);
        else
            eTheme->setThemeMode(ElaThemeType::Dark);
    });
    connect(eTheme, &ElaTheme::themeModeChanged, this, [this, setting](ElaThemeType::ThemeMode themeMode) {
        themeComboBox_->blockSignals(true);
        if (themeMode == ElaThemeType::Light)
            themeComboBox_->setCurrentIndex(0);
        else
            themeComboBox_->setCurrentIndex(1);
        themeComboBox_->blockSignals(false);

        // TODO: use const expression
        auto err = setting->setValueDB("ThemeMode", QString::number(themeMode));
        if(err) qCritical() << "[SettingPage] 设置主题模式失败, err:" << err.text() << " type:" << err.type();
    });

    ElaText *helperText = new ElaText("应用程序设置", this);
    helperText->setWordWrap(false);
    helperText->setTextPixelSize(18);

    micaSwitchButton_ = new ElaToggleSwitch(this);
    ElaScrollPageArea *micaSwitchArea = new ElaScrollPageArea(this);
    QHBoxLayout *micaSwitchLayout = new QHBoxLayout(micaSwitchArea);
    ElaText *micaSwitchText = new ElaText("启用云母效果(跨平台)", this);
    micaSwitchText->setWordWrap(false);
    micaSwitchText->setTextPixelSize(15);
    micaSwitchLayout->addWidget(micaSwitchText);
    micaSwitchLayout->addStretch();
    micaSwitchLayout->addWidget(micaSwitchButton_);

    connect(micaSwitchButton_, &ElaToggleSwitch::toggled, this, [](bool checked) {
        eApp->setIsEnableMica(checked);
    });

    minimumButton_ = new ElaRadioButton("Minimum", this);
    compactButton_ = new ElaRadioButton("Compact", this);
    maximumButton_ = new ElaRadioButton("Maximum", this);
    autoButton_ = new ElaRadioButton("Auto", this);
    switch (window->getNavigationBarDisplayMode()) {
        case ElaNavigationType::Minimal:
            minimumButton_->setChecked(true);
            break;
        case ElaNavigationType::Compact:
            compactButton_->setChecked(true);
            break;
        case ElaNavigationType::Maximal:
            maximumButton_->setChecked(true);
            break;
        case ElaNavigationType::Auto:
            autoButton_->setChecked(true);
            break;
    }

    ElaScrollPageArea *displayModeArea = new ElaScrollPageArea(this);
    QHBoxLayout *displayModeLayout = new QHBoxLayout(displayModeArea);
    ElaText *displayModeText = new ElaText("导航栏模式选择", this);
    displayModeText->setWordWrap(false);
    displayModeText->setTextPixelSize(15);
    displayModeLayout->addWidget(displayModeText);
    displayModeLayout->addStretch();
    displayModeLayout->addWidget(minimumButton_);
    displayModeLayout->addWidget(compactButton_);
    displayModeLayout->addWidget(maximumButton_);
    displayModeLayout->addWidget(autoButton_);
    connect(minimumButton_, &ElaRadioButton::toggled, this, [window, setting](bool checked) {
        if (checked){
            window->setNavigationBarDisplayMode(ElaNavigationType::Minimal);
            // TODO: use const expression
            auto err = setting->setValueDB("NavigationDisplayMode", "0");
            if(err) qCritical() << "[SettingPage] 设置导航栏模式失败, err:" << err.text() << " type:" << err.type();
        }
    });
    connect(compactButton_, &ElaRadioButton::toggled, this, [window, setting](bool checked) {
        if (checked){
            window->setNavigationBarDisplayMode(ElaNavigationType::Compact);
            // TODO: use const expression
            auto err = setting->setValueDB("NavigationDisplayMode", "2");
            if(err) qCritical() << "[SettingPage] 设置导航栏模式失败, err:" << err.text() << " type:" << err.type();
        }
    });
    connect(maximumButton_, &ElaRadioButton::toggled, this, [window, setting](bool checked) {
        if (checked){
            window->setNavigationBarDisplayMode(ElaNavigationType::Maximal);
            // TODO: use const expression
            auto err = setting->setValueDB("NavigationDisplayMode", "1");
            if(err) qCritical() << "[SettingPage] 设置导航栏模式失败, err:" << err.text() << " type:" << err.type();
        }
    });
    connect(autoButton_, &ElaRadioButton::toggled, this, [window, setting](bool checked) {
        if (checked){
            window->setNavigationBarDisplayMode(ElaNavigationType::Auto);
            // TODO: use const expression
            auto err = setting->setValueDB("NavigationDisplayMode", "3");
            if(err) qCritical() << "[SettingPage] 设置导航栏模式失败, err:" << err.text() << " type:" << err.type();
        }
    });

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("设置");
    QVBoxLayout *centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 20, 20);
    centerLayout->addSpacing(30);
    centerLayout->addWidget(themeText);
    centerLayout->addSpacing(10);
    centerLayout->addWidget(themeSwitchArea);
    centerLayout->addSpacing(15);
    centerLayout->addWidget(helperText);
    centerLayout->addSpacing(10);
    centerLayout->addWidget(micaSwitchArea);
    centerLayout->addWidget(displayModeArea);
    centerLayout->addStretch();
    addCentralWidget(centralWidget, true, true, 0);

}

SettingPage::~SettingPage() {
}
