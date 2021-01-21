/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QNSVIEW_H
#define QNSVIEW_H

#include <AppKit/AppKit.h>
#include <MetalKit/MetalKit.h>

#include <QtCore/private/qcore_mac_p.h>

QT_BEGIN_NAMESPACE
class QCocoaWindow;
class QCocoaGLContext;
class QPointF;
QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(QNSView) : NSView
@property (nonatomic, retain) NSCursor *cursor;
- (instancetype)initWithCocoaWindow:(QCocoaWindow *)platformWindow;
- (void)convertFromScreen:(NSPoint)mouseLocation toWindowPoint:(QPointF *)qtWindowPoint andScreenPoint:(QPointF *)qtScreenPoint;
@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(QNSView);

@interface QNSView (MouseAPI)
- (void)handleFrameStrutMouseEvent:(NSEvent *)theEvent;
- (void)resetMouseButtons;
@end

@interface QNSView (KeysAPI)
+ (Qt::KeyboardModifiers)convertKeyModifiers:(ulong)modifierFlags;
@end

@interface QNSView (ComplexTextAPI)
- (void)unmarkText;
- (void)cancelComposingText;
@end

@interface QNSView (QtExtras)
@property (nonatomic, readonly) QCocoaWindow *platformWindow;
@end

#endif //QNSVIEW_H
