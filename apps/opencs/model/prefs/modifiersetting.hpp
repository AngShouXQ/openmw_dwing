#ifndef CSM_PREFS_MODIFIERSETTING_H
#define CSM_PREFS_MODIFIERSETTING_H

#include "setting.hpp"

class QMutex;
class QObject;
class QWidget;
class QEvent;
class QPushButton;

namespace CSMPrefs
{
    class Category;
    class ModifierSetting : public Setting
    {
        Q_OBJECT

    public:
        ModifierSetting(Category* parent, QMutex* mutex, const std::string& key, const std::string& label);

        std::pair<QWidget*, QWidget*> makeWidgets(QWidget* parent) override;

        void updateWidget() override;

    protected:
        bool eventFilter(QObject* target, QEvent* event) override;

    private:
        bool handleEvent(QObject* target, int mod, int value);

        void storeValue(int modifier);
        void resetState();

        QPushButton* mButton;
        bool mEditorActive;

    private slots:

        void buttonToggled(bool checked);
    };
}

#endif
