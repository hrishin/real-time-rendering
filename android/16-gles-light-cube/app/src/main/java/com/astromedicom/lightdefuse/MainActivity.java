package com.astromedicom.lightdefuse;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {

    private GLESView glesView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        removeTitleBar();
        makeFullScreen();
        makeLandscape();
        glesView = new GLESView(this);
        setContentView(glesView);
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
}
