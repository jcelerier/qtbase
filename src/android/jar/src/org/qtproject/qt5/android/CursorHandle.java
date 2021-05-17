/****************************************************************************
**
** Copyright (C) 2016 Olivier Goffart <ogoffart@woboq.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Android port of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

package org.qtproject.qt5.android;

import android.content.Context;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.view.MotionEvent;
import android.widget.PopupWindow;
import android.app.Activity;
import android.util.TypedValue;
import android.view.ViewTreeObserver;

/* This view represents one of the handle (selection or cursor handle) */
class CursorView extends ImageView
{
    private CursorHandle mHandle;
    // The coordinare which where clicked
    private float m_offsetX;
    private float m_offsetY;
    private boolean m_pressed = false;

    CursorView (Context context, CursorHandle handle) {
        super(context);
        mHandle = handle;
    }

    // Called when the handle was moved programatically , with the delta amount in pixels
    public void adjusted(int dx, int dy) {
        m_offsetX += dx;
        m_offsetY += dy;
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        switch (ev.getActionMasked()) {
        case MotionEvent.ACTION_DOWN: {
            m_offsetX = ev.getRawX();
            m_offsetY = ev.getRawY() + getHeight() / 2;
            m_pressed = true;
            break;
        }

        case MotionEvent.ACTION_MOVE: {
            if (!m_pressed)
                return false;
            mHandle.updatePosition(Math.round(ev.getRawX() - m_offsetX),
                                    Math.round(ev.getRawY() - m_offsetY));
            break;
        }

        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_CANCEL:
            m_pressed = false;
            break;
        }
        return true;
    }

}

// Helper class that manages a cursor or selection handle
public class CursorHandle implements ViewTreeObserver.OnPreDrawListener
{
    private View m_layout = null;
    private CursorView m_cursorView = null;
    private PopupWindow m_popup = null;
    private int m_id;
    private int m_attr;
    private Activity m_activity;
    private int m_posX = 0;
    private int m_posY = 0;
    private int m_lastX;
    private int m_lastY;
    int tolerance;
    private boolean m_rtl;
    int m_yShift;

    public CursorHandle(Activity activity, View layout, int id, int attr, boolean rtl) {
        m_activity = activity;
        m_id = id;
        m_attr = attr;
        m_layout = layout;
        DisplayMetrics metrics = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        m_yShift = (int)TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_MM, 1f, metrics);
        tolerance = Math.min(1, (int)(m_yShift / 2f));
        m_lastX = m_lastY = -1 - tolerance;
        m_rtl = rtl;
    }

    private boolean initOverlay(){
        if (m_popup == null){

            Context context = m_layout.getContext();
            int[] attrs = {m_attr};
            TypedArray a = context.getTheme().obtainStyledAttributes(attrs);
            Drawable drawable = a.getDrawable(0);

            m_cursorView = new CursorView(context, this);
            m_cursorView.setImageDrawable(drawable);

            m_popup = new PopupWindow(context, null, android.R.attr.textSelectHandleWindowStyle);
            m_popup.setSplitTouchEnabled(true);
            m_popup.setClippingEnabled(false);
            m_popup.setContentView(m_cursorView);
            m_popup.setWidth(drawable.getIntrinsicWidth());
            m_popup.setHeight(drawable.getIntrinsicHeight());

            m_layout.getViewTreeObserver().addOnPreDrawListener(this);
        }
        return true;
    }

    // Show the handle at a given position (or move it if it is already shown)
    public void setPosition(final int x, final int y){
        initOverlay();

        final int[] layoutLocation = new int[2];
        m_layout.getLocationOnScreen(layoutLocation);

        // This value is used for handling split screen case
        final int[] activityLocation = new int[2];
        m_activity.getWindow().getDecorView().getLocationOnScreen(activityLocation);

        int x2 = x + layoutLocation[0] - activityLocation[0];
        int y2 = y + layoutLocation[1] + m_yShift - activityLocation[1];

        if (m_id == QtNative.IdCursorHandle) {
            x2 -= m_popup.getWidth() / 2 ;
        } else if ((m_id == QtNative.IdLeftHandle && !m_rtl) || (m_id == QtNative.IdRightHandle && m_rtl)) {
            x2 -= m_popup.getWidth() * 3 / 4;
        } else {
            x2 -= m_popup.getWidth() / 4;
        }

        if (m_popup.isShowing()) {
            m_popup.update(x2, y2, -1, -1);
            m_cursorView.adjusted(x - m_posX, y - m_posY);
        } else {
            m_popup.showAtLocation(m_layout, 0, x2, y2);
        }

        m_posX = x;
        m_posY = y;
    }

    public int bottom()
    {
        initOverlay();
        final int[] location = new int[2];
        m_cursorView.getLocationOnScreen(location);
        return location[1] + m_cursorView.getHeight();
    }

    public void hide() {
        if (m_popup != null) {
            m_popup.dismiss();
        }
    }

    // The handle was dragged by a given relative position
    public void updatePosition(int x, int y) {
        y -= m_yShift;
        if (Math.abs(m_lastX - x) > tolerance || Math.abs(m_lastY - y) > tolerance) {
            QtNative.handleLocationChanged(m_id, x + m_posX, y + m_posY);
            m_lastX = x;
            m_lastY = y;
        }
    }

    @Override
    public boolean onPreDraw() {
        // This hook is called when the view location is changed
        // For example if the keyboard appears.
        // Adjust the position of the handle accordingly
        if (m_popup != null && m_popup.isShowing())
            setPosition(m_posX, m_posY);

        return true;
    }
}