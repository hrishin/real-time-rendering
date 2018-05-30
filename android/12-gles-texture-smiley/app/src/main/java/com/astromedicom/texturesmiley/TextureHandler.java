package com.astromedicom.texturesmiley;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLES32;
import android.opengl.GLUtils;

/**
 * Created by Hrishikesh on 19-05-2018.
 */

public class TextureHandler {
    private Context context;
    private int resourceId;
    private int[] texture = new int[1];

    public TextureHandler(Context context, int resourceId) {
        this.context = context;
        this.resourceId = resourceId;
    }

    public int loadTexture() {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled  = false;

        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), resourceId, options);

        // create texture object to apply to model
        GLES30.glGenTextures(1, texture, 0);

        // indicate that pixels rows are tightly packed
        GLES30.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);

        // bind texture
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texture[0]);

        // set up filters and wrap modes for this texture object
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR_MIPMAP_LINEAR);

        // load the bitmap into bound texture
        GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0);

        // generate mipmap
        GLES30.glGenerateMipmap(GLES30.GL_TEXTURE_2D);

        return (texture[0]);
    }


    public void bind(int texture0SamplerUniform) {
        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texture[0]);
        GLES30.glUniform1i(texture0SamplerUniform, 0);
    }

    public void delete() {
        // destroy texture
        if(texture[0] != 0) {
            GLES30.glDeleteTextures(1, texture, 0);
            texture[0] = 0;
        }
    }
}
