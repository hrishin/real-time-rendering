package com.astromedicom.helloworld2;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;

/**
 * Created by Hrishikesh on 06-01-2018.
 */

class MyTextview extends TextView{
    public MyTextview(Context context) {
        super(context);
    }

    public MyTextview(Context context, String content, int color, int textSize) {
        super(context);
        setText(content);
        setTextColor(color);
        setTextSize(textSize);
    }
}
