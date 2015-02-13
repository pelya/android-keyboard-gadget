package teaonly.droideye;

import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

import java.util.*;

import java.nio.ByteBuffer;

public class CameraFeed implements PreviewCallback {
	private static final String TAG = "SDL-CAMERA";

	private Camera camera_ = null;
	private SurfaceHolder surfaceHolder_ = null;
	private SurfaceView	  surfaceView_;

	private List<Camera.Size> supportedSizes; 
	private Camera.Size procSize_;
	private boolean inProcessing = false;

	static CameraFeed instance = null;

	public CameraFeed() {
		surfaceView_ = new SurfaceView(MainActivity.instance);
		surfaceHolder_ = surfaceView_.getHolder();
		surfaceHolder_.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}

	public List<Camera.Size> getSupportedPreviewSize() {
		return supportedSizes;
	}

	public void StopPreview() {
		if ( camera_ == null)
			return;
		camera_.stopPreview();
	}

	/*
	public void AutoFocus() {
		if ( camera_ == null)
			return;
		camera_.autoFocus(afcb);
	}
	*/

	public void Rotate(int degree) {
		if ( camera_ == null)
			return;
		camera_.setDisplayOrientation(degree);
	}

	public void Release() {
		if ( camera_ != null) {
			camera_.setPreviewCallback(null);
			camera_.stopPreview();
			camera_.release();
			camera_ = null;
		}
	}
	
	public void setupCamera(int wid, int hei) {
		camera_ = Camera.open(0);
		procSize_ = camera_.new Size(0, 0);
		Camera.Parameters p = camera_.getParameters();
		List<Integer> formats = p.getSupportedPreviewFormats();
		for (int i = 0; i < formats.size(); i++) {
			int f = formats.get(i);
			Log.d(TAG, "format:" + f);
		}
		supportedSizes = p.getSupportedPreviewSizes();

		try {
			camera_.setPreviewDisplay(surfaceHolder_);
		} catch ( Exception ex) {
			ex.printStackTrace();
		}

		int maxDiff = 10000000;
		procSize_ = supportedSizes.get(0);
		for (Camera.Size size: supportedSizes) {
			int diff = Math.abs(size.width - wid) + Math.abs(size.height - hei);
			if (maxDiff > diff) {
				maxDiff = diff;
				procSize_ = size;
			}
		}

		p.setPreviewSize(procSize_.width, procSize_.height);
		p.setFocusMode(p.FOCUS_MODE_CONTINUOUS_VIDEO);
		camera_.setParameters(p);

		actualCameraSize(procSize_.width, procSize_.height);

		camera_.setPreviewCallback(this);

		camera_.startPreview();
	}

	public void onPreviewFrame(byte[] frame, Camera c) {
		if ( !inProcessing ) {
			inProcessing = true;
			try {
				pushImage(frame);
			} catch (Exception e) {
				e.printStackTrace();
			}
			inProcessing = false;
		}
	}

	// Called from native code
	static public void initCamera(int width, int height)
	{
		if (instance == null)
			instance = new CameraFeed();
		instance.StopPreview();
		instance.setupCamera(width, height);
	}

	// Called from native code
	static public void deinitCamera()
	{
		if (instance == null)
			return;
		instance.Release();
	}

	native void actualCameraSize(int width, int height);
	native void pushImage(byte[] frame);

	// Conversion routines from YUV to RGB, not used here - this is done in C code

	//Method from Ketai project! Not mine! See below...
	void decodeNV21(int[] rgb, byte[] yuv420sp, int width, int height) {  
		final int frameSize = width * height;  

		for (int j = 0, yp = 0; j < height; j++) {
			int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
			for (int i = 0; i < width; i++, yp++) {
				int y = (0xff & ((int) yuv420sp[yp])) - 16;
				if (y < 0)
					y = 0;
				if ((i & 1) == 0) {
					v = (0xff & yuv420sp[uvp++]) - 128;
					u = (0xff & yuv420sp[uvp++]) - 128;
				}

				int y1192 = 1192 * y;
				int r = (y1192 + 1634 * v);
				int g = (y1192 - 833 * v - 400 * u);
				int b = (y1192 + 2066 * u);

				if (r < 0)
					r = 0;
				else if (r > 262143)
					r = 262143;
				if (g < 0)
					g = 0;
				else if (g > 262143)
					g = 262143;
				if (b < 0)
					b = 0;
				else if (b > 262143)
					b = 262143;

				rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
			}
		}
	}

	/**
	 * Converts semi-planar YUV420 as generated for camera preview into RGB565
	 * format for use as an OpenGL ES texture. It assumes that both the input
	 * and output data are contiguous and start at zero.
	 * 
	 * @param yuvs the array of YUV420 semi-planar data
	 * @param rgbs an array into which the RGB565 data will be written
	 * @param width the number of pixels horizontally
	 * @param height the number of pixels vertically
	 */
	//we tackle the conversion two pixels at a time for greater speed
	private void toRGB565(byte[] yuvs, int width, int height, byte[] rgbs) {
		//the end of the luminance data
		final int lumEnd = width * height;
		//points to the next luminance value pair
		int lumPtr = 0;
		//points to the next chromiance value pair
		int chrPtr = lumEnd;
		//points to the next byte output pair of RGB565 value
		int outPtr = 0;
		//the end of the current luminance scanline
		int lineEnd = width;
	
		while (true) {
	
			//skip back to the start of the chromiance values when necessary
			if (lumPtr == lineEnd) {
				if (lumPtr == lumEnd) break; //we've reached the end
				//division here is a bit expensive, but's only done once per scanline
				chrPtr = lumEnd + ((lumPtr >> 1) / width) * width;
				lineEnd += width;
			}
	
			//read the luminance and chromiance values
			final int Y1 = yuvs[lumPtr++] & 0xff;
			final int Y2 = yuvs[lumPtr++] & 0xff;
			final int Cr = (yuvs[chrPtr++] & 0xff) - 128;
			final int Cb = (yuvs[chrPtr++] & 0xff) - 128;
			int R, G, B;
	
			//generate first RGB components
			B = Y1 + ((454 * Cb) >> 8);
			if(B < 0) B = 0; else if(B > 255) B = 255;
			G = Y1 - ((88 * Cb + 183 * Cr) >> 8);
			if(G < 0) G = 0; else if(G > 255) G = 255;
			R = Y1 + ((359 * Cr) >> 8);
			if(R < 0) R = 0; else if(R > 255) R = 255;
			//NOTE: this assume little-endian encoding
			rgbs[outPtr++] = (byte) (((G & 0x3c) << 3) | (B >> 3));
			rgbs[outPtr++] = (byte) ((R & 0xf8) | (G >> 5));
	
			//generate second RGB components
			B = Y2 + ((454 * Cb) >> 8);
			if(B < 0) B = 0; else if(B > 255) B = 255;
			G = Y2 - ((88 * Cb + 183 * Cr) >> 8);
			if(G < 0) G = 0; else if(G > 255) G = 255;
			R = Y2 + ((359 * Cr) >> 8);
			if(R < 0) R = 0; else if(R > 255) R = 255;
			//NOTE: this assume little-endian encoding
			rgbs[outPtr++]	= (byte) (((G & 0x3c) << 3) | (B >> 3));
			rgbs[outPtr++]	= (byte) ((R & 0xf8) | (G >> 5));
		}
	}
}
