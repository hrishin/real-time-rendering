package com.astromedicom.eventhandler;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {

    private MyTextview myTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        removeTitleBar();
        makeFullScreen();
        makeLandscape();
        setBackground();
        createTextContent();
        setContentView(myTextView);
    }

    private void removeTitleBar() {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
    }

    private void makeFullScreen() {
        getWindow()
                .setFlags(
                        WindowManager.LayoutParams.FLAG_FULLSCREEN,
                        WindowManager.LayoutParams.FLAG_FULLSCREEN
                );
    }

    private void makeLandscape() {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    private void setBackground() {
        getWindow().getDecorView().setBackgroundColor(Color.BLACK);
    }

    private void createTextContent() {
        myTextView = new MyTextview(this, "Hello World by MyTextview", Color.GREEN, 60);
        myTextView.setGravity(Gravity.CENTER);
    }
}
