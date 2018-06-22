package com.astromedicom.texturecheckboard;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLES32;
import android.opengl.GLUtils;
import android.util.Log;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.Arrays;

/**
 * Created by Hrishikesh on 19-05-2018.
 */

public class TextureHandler {
    private Context context;
    private int resourceId;
    private int[] texture = new int[1];
    private int checkImageWidth;
    private int checkImageHeight;
    private byte[] checkImage;

    public TextureHandler(Context context, int resourceId) {
        this.context = context;
        this.resourceId = resourceId;
    }

    public TextureHandler(Context context, int width, int height) {
        this.context = context;
        this.checkImageWidth = width;
        this.checkImageHeight = height;
        this.checkImage = new byte[checkImageWidth*checkImageHeight*4];
    }

    public int loadTexture() {
        makeCheckImage();

        // create texture object to apply to model
        GLES30.glGenTextures(1, texture, 0);

        // indicate that pixels rows are tightly packed
        GLES30.glPixelStorei(GLES30.GL_UNPACK_ALIGNMENT, 1);

        // bind texture
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texture[0]);

        // set up filters and wrap modes for this texture object
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_NEAREST);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);

        // load the bitmap into bound texture
        ByteBuffer checkImageBuffer = ByteBuffer.allocateDirect(checkImage.length);
        checkImageBuffer.order(ByteOrder.nativeOrder());
        checkImageBuffer.put(checkImage);
        checkImageBuffer.position(0);

        GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, checkImageWidth, checkImageHeight, 0,
                GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, checkImageBuffer);

        // generate mipmap
        GLES30.glGenerateMipmap(GLES30.GL_TEXTURE_2D);

        return (texture[0]);
    }

    private void makeCheckImage()
    {
        int i, j, k;
        byte
                c;
        int index = 0;

        for (i = 0; i < checkImageHeight; i++)
        {
            for (j = 0; j < checkImageWidth; j++)
            {
                c = (byte) (((i & 0x8) ^ (j & 0x8)) == 8 ? 255 : 0);

                for (k = 0 ; k < 3; k++)
                {
                    checkImage[index++] = c;
                }
                checkImage[index++] = (byte) 1; //alpha
            }
        }
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
