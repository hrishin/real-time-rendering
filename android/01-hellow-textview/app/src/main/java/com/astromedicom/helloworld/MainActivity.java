package com.astromedicom.helloworld;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

public class MainActivity extends Activity {

    private TextView textView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        removeTitleBar();
        makeFullScreen();
        makeLandScape();
        setBackgroundColor();
        prepareTextContent();
        setContentView(textView);
    }

    private void setBackgroundColor() {
        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
    }

    private void prepareTextContent() {
        textView =  new TextView(this);
        textView.setText("Hello World!!");
        textView.setTextSize(60);
        textView.setTextColor(Color.GREEN);
        textView.setGravity(Gravity.CENTER);
    }

    private void makeLandScape() {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    private void makeFullScreen() {
        getWindow()
                .setFlags(
                    WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN
                );
    }

    private void removeTitleBar() {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
    }
}
