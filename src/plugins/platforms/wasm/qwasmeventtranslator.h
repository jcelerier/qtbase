// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWASMEVENTTRANSLATOR_H
#define QWASMEVENTTRANSLATOR_H

#include <QtCore/qobject.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <emscripten/html5.h>
#include "qwasmwindow.h"
#include <QtGui/qinputdevice.h>
#include <QHash>
#include <QCursor>
#include "qwasmevent.h"
#include "qwasmplatform.h"

QT_BEGIN_NAMESPACE

class QWindow;

class QWasmEventTranslator : public QObject
{
    Q_OBJECT

public:
    explicit QWasmEventTranslator();
    ~QWasmEventTranslator();

    static QCursor cursorForMode(QWasmCompositor::ResizeMode mode);

    QString getKeyText(const EmscriptenKeyboardEvent *keyEvent, Qt::Key key);
    Qt::Key getKey(const EmscriptenKeyboardEvent *keyEvent);

private:
    static quint64 getTimestamp();

    Qt::Key m_emDeadKey = Qt::Key_unknown;

};

QT_END_NAMESPACE
#endif // QWASMEVENTTRANSLATOR_H
