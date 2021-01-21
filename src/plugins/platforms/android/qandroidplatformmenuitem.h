/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QANDROIDPLATFORMMENUITEM_H
#define QANDROIDPLATFORMMENUITEM_H
#include <qpa/qplatformmenu.h>

QT_BEGIN_NAMESPACE

class QAndroidPlatformMenu;

class QAndroidPlatformMenuItem: public QPlatformMenuItem
{
public:
    QAndroidPlatformMenuItem();

    void setText(const QString &text) override;
    QString text() const;

    void setIcon(const QIcon &icon) override;
    QIcon icon() const;

    void setMenu(QPlatformMenu *menu) override;
    QAndroidPlatformMenu *menu() const;

    void setVisible(bool isVisible) override;
    bool isVisible() const;

    void setIsSeparator(bool isSeparator) override;
    bool isSeparator() const;

    void setFont(const QFont &font) override;

    void setRole(MenuRole role) override;
    MenuRole role() const;

    void setCheckable(bool checkable) override;
    bool isCheckable() const;

    void setChecked(bool isChecked) override;
    bool isChecked() const;

    void setShortcut(const QKeySequence &shortcut) override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const;

    void setIconSize(int size) override;

private:
    QString m_text;
    QIcon m_icon;
    QAndroidPlatformMenu *m_menu;
    bool m_isVisible;
    bool m_isSeparator;
    MenuRole m_role;
    bool m_isCheckable;
    bool m_isChecked;
    bool m_isEnabled;
};

QT_END_NAMESPACE

#endif // QANDROIDPLATFORMMENUITEM_H
