package com.astromedicom.eventhandler;

import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.widget.TextView;

/**
 * Created by Hrishikesh on 06-01-2018.
 */

class MyTextview extends TextView implements GestureDetector.OnGestureListener, GestureDetector.OnDoubleTapListener{
    private GestureDetector gestureDetector;

    public MyTextview(Context context) {
        super(context);
    }

    public MyTextview(Context context, String content, int color, int textSize) {
        super(context);
        setText(content);
        setTextColor(color);
        setTextSize(textSize);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int eventAction = event.getAction();
        if(!gestureDetector.onTouchEvent(event)) {
            super.onTouchEvent(event);
        }
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent motionEvent) {
        setText("Single tap");
        return true;
    }

    @Override
    public boolean onDoubleTap(MotionEvent motionEvent) {
        setText("Double tap");
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public boolean onDown(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public void onShowPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent motionEvent) {
        return true;
    }

    @Override
    public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        setText("Scroll");
        return true;
    }

    @Override
    public void onLongPress(MotionEvent motionEvent) {
        setText("Long press");
    }

    @Override
    public boolean onFling(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        setText("its fling");
        return true;
    }
}
