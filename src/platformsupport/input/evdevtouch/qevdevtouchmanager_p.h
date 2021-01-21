/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QEVDEVTOUCHMANAGER_P_H
#define QEVDEVTOUCHMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtInputSupport/private/devicehandlerlist_p.h>

#include <QObject>
#include <QHash>
#include <QSocketNotifier>

QT_BEGIN_NAMESPACE

class QDeviceDiscovery;
class QEvdevTouchScreenHandlerThread;

class QEvdevTouchManager : public QObject
{
public:
    QEvdevTouchManager(const QString &key, const QString &spec, QObject *parent = nullptr);
    ~QEvdevTouchManager();

    void addDevice(const QString &deviceNode);
    void removeDevice(const QString &deviceNode);

    void updateInputDeviceCount();

private:
    QString m_spec;
    QtInputSupport::DeviceHandlerList<QEvdevTouchScreenHandlerThread> m_activeDevices;
};

QT_END_NAMESPACE

#endif // QEVDEVTOUCHMANAGER_P_H
