package remote.hid.keyboard.client;

import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.Gravity;
import android.widget.FrameLayout;
import java.io.*;
import java.util.*;
import java.nio.ByteBuffer;

import remote.hid.keyboard.client.MainActivity;

public class CameraFeed implements PreviewCallback {
	private static final String TAG = "SDL-CAMERA";

	private Camera camera = null;
	private SurfaceHolder surfaceHolder = null;
	private SurfaceView surfaceView = null;

	private List<Camera.Size> supportedSizes; 
	private Camera.Size procSize;
	private boolean inProcessing = false;

	static CameraFeed instance = null;

	public CameraFeed() {
	}

	public List<Camera.Size> getSupportedPreviewSize() {
		return supportedSizes;
	}

	public void StopPreview() {
		if ( camera == null)
			return;
		camera.stopPreview();
	}

	/*
	public void AutoFocus() {
		if ( camera == null)
			return;
		camera.autoFocus(afcb);
	}
	*/

	public void Rotate(int degree) {
		if ( camera == null)
			return;
		camera.setDisplayOrientation(degree);
	}

	public void Release() {
		if ( camera != null) {
			camera.setPreviewCallback(null);
			camera.stopPreview();
			camera.release();
			camera = null;
			
			MainActivity.instance.runOnUiThread(new Runnable() {
				public void run() {
					MainActivity.instance._videoLayout.removeView(surfaceView);
					surfaceView = null;
					surfaceHolder = null;
				}
			});
			while (surfaceHolder != null) {
				try { Thread.sleep(100); } catch (Exception e) {}
			}
		}
	}
	
	public void setupCamera(int wid, int hei, int fps) throws IOException {
		Log.d(TAG, "setupCamera: " + wid + "x" + hei + " FPS " + fps);

		camera = Camera.open(0);
		procSize = camera.new Size(0, 0);
		Camera.Parameters p = camera.getParameters();
		List<Integer> formats = p.getSupportedPreviewFormats();
		for (int i = 0; i < formats.size(); i++) {
			int f = formats.get(i);
			Log.d(TAG, "Camera supported format:" + f);
		}
		supportedSizes = p.getSupportedPreviewSizes();

		int maxDiff = 10000000;
		procSize = supportedSizes.get(0);
		for (Camera.Size size: supportedSizes) {
			Log.d(TAG, "Camera supported dimensions: " + size.width + " " + size.height);
			int diff = Math.abs(size.width - wid) + Math.abs(size.height - hei);
			if (maxDiff > diff) {
				maxDiff = diff;
				procSize = size;
			}
		}
		p.setPreviewSize(procSize.width, procSize.height);

		List<int[]> fpsRanges = p.getSupportedPreviewFpsRange();
		int[] fpsAct = fpsRanges.get(0)[0] > fps ? fpsRanges.get(0) : fpsRanges.get(fpsRanges.size() - 1);
		for (int[] ff: fpsRanges) {
			Log.d(TAG, "Camera supported FPS range: " + ff[0] + " " + ff[1]);
			if (ff[0] <= fps && ff[1] >= fps)
				fpsAct = ff;
		}
		p.setPreviewFpsRange(fpsAct[0], fpsAct[1]);

		//p.setFocusMode(p.FOCUS_MODE_CONTINUOUS_VIDEO); // Not supported on Nexus 7
		camera.setParameters(p);

		MainActivity.instance.runOnUiThread(new Runnable() {
			public void run() {
				surfaceView = new SurfaceView(MainActivity.instance);
				surfaceHolder = surfaceView.getHolder();
				surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
				MainActivity.instance._videoLayout.addView(surfaceView, new FrameLayout.LayoutParams(1, 1, Gravity.LEFT | Gravity.TOP));
				//MainActivity.instance._videoLayout.bringChildToFront(surfaceView);
			}
		});
		while (surfaceHolder == null) {
			try { Thread.sleep(100); } catch (Exception e) {}
		}
		try { Thread.sleep(100); } catch (Exception e) {}
		actualCameraSize(procSize.width, procSize.height);
		try { Thread.sleep(200); } catch (Exception e) {}

		camera.setPreviewDisplay(surfaceHolder);
		camera.setPreviewCallback(this);

		camera.startPreview();
	}

	public void onPreviewFrame(byte[] frame, Camera c) {
		//Log.d(TAG, "onPreviewFrame");
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
	static public void initCamera(int width, int height, int fps)
	{
		Log.d(TAG, "initCamera: " + width + "x" + height + " FPS " + fps);
		try {
			if (instance == null)
				instance = new CameraFeed();
			instance.setupCamera(width, height, fps * 1000);
		} catch (Exception e) {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			e.printStackTrace(pw);
			Log.w(TAG, "initCamera exception: " + sw.toString());
		}
		Log.d(TAG, "initCamera done");
	}

	// Called from native code
	static public void deinitCamera()
	{
		Log.d(TAG, "deinitCamera");
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
