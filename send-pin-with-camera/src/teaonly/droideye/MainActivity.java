package teaonly.droideye;
import teaonly.droideye.*;

import java.io.IOException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.lang.System;
import java.lang.Thread;
import java.util.*;
import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import  org.apache.http.conn.util.InetAddressUtils;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.res.Resources;
import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.PictureCallback;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.Paint;
import android.graphics.YuvImage;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.media.AudioFormat;
import android.media.MediaRecorder;
import android.media.AudioRecord;
import android.text.format.Formatter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.SurfaceView;
import android.util.Log;
import android.widget.LinearLayout; 
import android.widget.ImageButton;
import android.widget.Button;
import android.widget.TextView;

//import com.google.ads.*;

public class MainActivity extends Activity 
    implements View.OnTouchListener, CameraView.CameraReadyCallback, OverlayView.UpdateDoneCallback{
    private static final String TAG = "TEAONLY";

//    private AdView adView;

    boolean inProcessing = false;
    final int maxVideoNumber = 3;
    VideoFrame[] videoFrames = new VideoFrame[maxVideoNumber];
    byte[] preFrame = new byte[1024*1024*8];
    
    TeaServer webServer = null;
    private CameraView cameraView_;
    private OverlayView overlayView_;
    private Button btnExit;
    private TextView tvMessage1;
    private TextView tvMessage2;
    private int mQuality = 30;
    private ViewGroup mBlackScreen;

    private AudioRecord audioCapture = null;
    private StreamingLoop audioLoop = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);    
        win.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN); 

        setContentView(R.layout.main);

        //setup adView
//        LinearLayout layout = (LinearLayout)findViewById(R.id.layout_setup);
//        adView = new AdView(this, AdSize.BANNER, "a1507f940fc****");
//        layout.addView(adView);
//        adView.loadAd(new AdRequest());

        btnExit = (Button)findViewById(R.id.btn_exit);
        btnExit.setOnClickListener(exitAction);
        tvMessage1 = (TextView)findViewById(R.id.tv_message1);
        tvMessage2 = (TextView)findViewById(R.id.tv_message2);
        mBlackScreen = (ViewGroup) findViewById(R.id.black_screen);
        
        for(int i = 0; i < maxVideoNumber; i++) {
            videoFrames[i] = new VideoFrame(1024*1024*2);        
        }    

        System.loadLibrary("mp3encoder");
        System.loadLibrary("natpmp");

        initAudio();
        initCamera();
    }
    
    @Override
    public void onCameraReady() {
        if ( initWebServer() ) {
            int wid = cameraView_.Width();
            int hei = cameraView_.Height();
            cameraView_.StopPreview();
            cameraView_.setupCamera(wid, hei, previewCb_);
            cameraView_.StartPreview();
        }
    }
 
    @Override
    public void onUpdateDone() {
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        inProcessing = true;
        if ( webServer != null)
            webServer.stop();
        audioLoop.ReleaseLoop();
        audioCapture.release();
    }   

    @Override
    public void onStart(){
        super.onStart();
        try {
            InputStream in = getAssets().open("hid-gadget-test");
            OutputStream out = new BufferedOutputStream(new FileOutputStream(getFilesDir().getAbsolutePath() + "/hid-gadget-test"));
            copyFile(in, out);
            in.close();
            out.close();
            Runtime.getRuntime().exec("/system/bin/chmod 755 " + getFilesDir().getAbsolutePath() + "/hid-gadget-test").waitFor();
            in = getAssets().open("hid-gadget-test-" + android.os.Build.CPU_ABI);
            out = new BufferedOutputStream(new FileOutputStream(getFilesDir().getAbsolutePath() + "/hid-gadget-test"));
            copyFile(in, out);
            in.close();
            out.close();
            Runtime.getRuntime().exec("/system/bin/chmod 755 " + getFilesDir().getAbsolutePath() + "/hid-gadget-test").waitFor();
        } catch (Exception e) { }
    }   

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1){
          out.write(buffer, 0, read);
        }
    }

    @Override
    public void onResume(){
        super.onResume();
    }   
    
    @Override
    public void onPause(){  
        super.onPause();
//        inProcessing = true;
//        if ( webServer != null)
//            webServer.stop();
//        cameraView_.StopPreview(); 
//        //cameraView_.Release();
//        audioLoop.ReleaseLoop();
//        audioCapture.release();
//    
//        //System.exit(0);
//        finish();
    }  
    
    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override 
    public boolean onTouch(View v, MotionEvent evt) { 
        

        return false;
    }
  
    private void initAudio() {
        int minBufferSize = AudioRecord.getMinBufferSize(44100, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
        int minTargetSize = 4410 * 2;      // 0.1 seconds buffer size
        if (minTargetSize < minBufferSize) {
            minTargetSize = minBufferSize;
        }
        if (audioCapture == null) {
            audioCapture = new AudioRecord(MediaRecorder.AudioSource.MIC,
                                        44100,
                                        AudioFormat.CHANNEL_IN_MONO,
                                        AudioFormat.ENCODING_PCM_16BIT,
                                        minTargetSize);
        }

        if ( audioLoop == null) {  
            Random rnd = new Random();
            String etag = Integer.toHexString( rnd.nextInt() );
            audioLoop = new StreamingLoop("teaonly.droideye" + etag );
        }

    }

    private void initCamera() {
        SurfaceView cameraSurface = (SurfaceView)findViewById(R.id.surface_camera);
        cameraView_ = new CameraView(cameraSurface);        
        cameraView_.setCameraReadyCallback(this);

        overlayView_ = (OverlayView)findViewById(R.id.surface_overlay);
        overlayView_.setOnTouchListener(this);
        overlayView_.setUpdateDoneCallback(this);
    }
    
    public String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    //if (!inetAddress.isLoopbackAddress() && !inetAddress.isLinkLocalAddress() && inetAddress.isSiteLocalAddress() ) {
                    if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(inetAddress.getHostAddress()) ) {
                        String ipAddr = inetAddress.getHostAddress();
                        return ipAddr;
                    }
                }
            }
        } catch (SocketException ex) {
            Log.d(TAG, ex.toString());
        }
        return null;
    }   

    private boolean initWebServer() {
        String ipAddr = getLocalIpAddress();
        if ( ipAddr != null ) {
            try{
                webServer = new TeaServer(8080, this); 
                webServer.registerCGI("/cgi/query", doQuery);
                webServer.registerCGI("/cgi/setup", doSetup);
                webServer.registerCGI("/stream/live.jpg", doCapture);
                webServer.registerCGI("/stream/live.mp3", doBroadcast);
                webServer.registerCGI("/cgi/rotate", doRotate);
                webServer.registerCGI("/cgi/autofocus", doAutoFocus);
                webServer.registerCGI("/cgi/changequality", doChangeQuality);
                webServer.registerCGI("/cgi/dimscreen", doDimScreen);
            }catch (IOException e){
                webServer = null;
            }
        }
        if ( webServer != null) {
            tvMessage1.setText( getString(R.string.msg_access_local) + " http://" + ipAddr  + ":8080" );
            tvMessage2.setText( getString(R.string.msg_access_query));
            tvMessage2.setVisibility(View.VISIBLE);
            NatPMPClient natQuery = new NatPMPClient();
            natQuery.start();  
            return true;
        } else {
            tvMessage1.setText( getString(R.string.msg_error) );
            //tvMessage2.setVisibility(View.GONE);
            return false;
        }
          
    }
   
    private OnClickListener exitAction = new OnClickListener() {
        @Override
        public void onClick(View v) {
            // onPause();
            finish();
        }   
    };

    public static final float SYMBOL_SPACING = 0.02f; // Amount of space between PIN symbols
    public static final int BRIGHTNESS_THRESHOLD = 128; // From 0 to 255
    public static final int PIN_LENGTH = 4;
    public static final boolean FRONT_CAMERA = false; // Front camera has mirrored picture
    public static final int SEND_DELAY = 400; // Delay before sending another char, in milliseconds
    public static final boolean[] pinEntered = new boolean[PIN_LENGTH];
    int currentPin = 0;
    boolean sendEnter = false;
    long delay = 0;

    private PreviewCallback previewCb_ = new PreviewCallback() {
        public void onPreviewFrame(byte[] frame, Camera c) {
            if ( !inProcessing ) {
                inProcessing = true;
           
                int picWidth = cameraView_.Width();
                int picHeight = cameraView_.Height(); 
                ByteBuffer bbuffer = ByteBuffer.wrap(frame); 
                try {
                    bbuffer.get(preFrame, 0, picWidth*picHeight + picWidth*picHeight/2);

					int numFilled = 0;

					int y = picHeight / 2;
					for (int i = 0; i < PIN_LENGTH; i++)
					{
						int x = picWidth / 2 + (int)((i + 0.5f - PIN_LENGTH / 2) * SYMBOL_SPACING * picWidth);
						int p = 0;
						// Take 4 nearby pixels, to filter out noise
						p += preFrame[picWidth * y + x] & 0xFF;
						p += preFrame[picWidth * y + x + 1] & 0xFF;
						p += preFrame[picWidth * (y + 1) + x] & 0xFF;
						p += preFrame[picWidth * (y + 1) + x + 1] & 0xFF;
						p /= 4;
						pinEntered[i] = (p < BRIGHTNESS_THRESHOLD);

						if (pinEntered[i])
							numFilled ++;
					}
					overlayView_.invalidate();
					
					if ( System.currentTimeMillis() > delay ) {
						delay = System.currentTimeMillis() + SEND_DELAY;

						if (sendEnter && numFilled == 0) {
							sendEnter = false;
							currentPin++;
							runOnUiThread( new Runnable() {
								public void run() {
									tvMessage2.setText( "PIN: " + currentPin );
								}
							});
						}

						if (sendEnter && numFilled == PIN_LENGTH) {
							final String[] cmd = {"/system/bin/sh", "-c", "echo 'echo enter | " + getFilesDir().getAbsolutePath() + "/hid-gadget-test /dev/hidg0 keyboard' | su"};
							runOnUiThread( new Runnable() {
								public void run() {
									tvMessage2.setText( "PIN: " + currentPin + " - sending Enter" );
								}
							});
							Runtime.getRuntime().exec(cmd).waitFor();
						}

						if (!sendEnter && numFilled == PIN_LENGTH) {
							sendEnter = true;
						}

						if (!sendEnter && numFilled < PIN_LENGTH) {
							final int digit = (currentPin / (int)Math.pow(10, PIN_LENGTH - numFilled - 1)) % 10;
							final int pos = numFilled + 1;
							final String[] cmd = {"/system/bin/sh", "-c", "echo 'echo " + digit + " | " + getFilesDir().getAbsolutePath() + "/hid-gadget-test /dev/hidg0 keyboard' | su"};
							runOnUiThread( new Runnable() {
								public void run() {
									tvMessage2.setText( "PIN: " + String.format("%04d", currentPin) + " - sending digit #" + pos + " : " + digit );
								}
							});
							Runtime.getRuntime().exec(cmd).waitFor();
						}
					}


                } catch (Exception e) {
                    e.printStackTrace();
                }

                inProcessing = false;
            }
        }
    };
    
    private TeaServer.CommonGatewayInterface doQuery = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            String ret = "";
            List<Camera.Size> supportSize =  cameraView_.getSupportedPreviewSize();                             
            ret = ret + "" + cameraView_.Width() + "x" + cameraView_.Height() + "|";
            for(int i = 0; i < supportSize.size() - 1; i++) {
                ret = ret + "" + supportSize.get(i).width + "x" + supportSize.get(i).height + "|";
            }
            int i = supportSize.size() - 1;
            ret = ret + "" + supportSize.get(i).width + "x" + supportSize.get(i).height ;
            JSONObject obj = new JSONObject();
            try {
                obj.put("camsize", ret);
                obj.put("quality", mQuality);
            } catch (JSONException e) {
                e.printStackTrace();
            }
            return obj.toString();
        }
        
        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 

    private TeaServer.CommonGatewayInterface doSetup = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            int wid = Integer.parseInt(parms.getProperty("wid")); 
            int hei = Integer.parseInt(parms.getProperty("hei"));
            Log.d(TAG, ">>>>>>>run in doSetup wid = " + wid + " hei=" + hei);
            cameraView_.StopPreview();
            cameraView_.setupCamera(wid, hei, previewCb_);
            cameraView_.StartPreview();
            return "OK";
        }   
 
        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 

    private TeaServer.CommonGatewayInterface doBroadcast = new TeaServer.CommonGatewayInterface() {
        @Override
        public String run(Properties parms) {
            return null;
        }   
        
        
        @Override 
        public InputStream streaming(Properties parms) {
            if ( audioLoop.isConnected() ) {     
                return null;                    // tell client is is busy by 503
            }    
 
            audioLoop.InitLoop(128, 8192);
            InputStream is = null;
            try{
                is = audioLoop.getInputStream();
            } catch(IOException e) {
                audioLoop.ReleaseLoop();
                return null;
            }
            
            audioCapture.startRecording();
            AudioEncoder audioEncoder = new AudioEncoder();
            audioEncoder.start();  
            
            return is;
        }

    };

    private TeaServer.CommonGatewayInterface doCapture = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
           return null;
        }   
        
        @Override 
        public InputStream streaming(Properties parms) {
            VideoFrame targetFrame = null;
            for(int i = 0; i < maxVideoNumber; i++) {
                if ( videoFrames[i].acquire() ) {
                    targetFrame = videoFrames[i];
                    break;
                }
            }
            // return 503 internal error
            if ( targetFrame == null) {
                Log.d(TAG, "No free videoFrame found!");
                return null;
            }

            // compress yuv to jpeg
            int picWidth = cameraView_.Width();
            int picHeight = cameraView_.Height(); 
            YuvImage newImage = new YuvImage(preFrame, ImageFormat.NV21, picWidth, picHeight, null);
            targetFrame.reset();
            boolean ret;
            inProcessing = true;
            try{
                ret = newImage.compressToJpeg( new Rect(0,0,picWidth,picHeight), mQuality, targetFrame);
            } catch (Exception ex) {
                ret = false;    
            } 
            inProcessing = false;

            // compress success, return ok
            if ( ret == true)  {
                parms.setProperty("mime", "image/jpeg");
                InputStream ins = targetFrame.getInputStream();
                return ins;
            }
            // send 503 error
            targetFrame.release();

            return null;
        }
    }; 

    private TeaServer.CommonGatewayInterface doRotate = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            int degree = Integer.parseInt(parms.getProperty("degree")); 
            Log.d(TAG, ">>>>>>>run in doRotate degree = " + degree);
            cameraView_.StopPreview();
            cameraView_.Rotate(degree);
            cameraView_.setupCamera(cameraView_.Width(), cameraView_.Height(), previewCb_);
            cameraView_.StartPreview();
            return "OK";
        }   
 
        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 

    private TeaServer.CommonGatewayInterface doAutoFocus = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            Log.d(TAG, ">>>>>>>run in doAutoFocus");
            cameraView_.AutoFocus();
            return "OK";
        }   

        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 

    private TeaServer.CommonGatewayInterface doChangeQuality = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            try {
                int quality = Integer.parseInt(parms.getProperty("quality")); 
                Log.d(TAG, ">>>>>>>run in doChangeQuality quality = " + quality);
                mQuality = quality;
                return "OK";
            } catch (Exception e) {
                e.printStackTrace();
            }
            return "ERROR";
        }   
 
        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 

    private TeaServer.CommonGatewayInterface doDimScreen = new TeaServer.CommonGatewayInterface () {
        @Override
        public String run(Properties parms) {
            Log.d(TAG, ">>>>>>>run in doDimScreen");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    WindowManager.LayoutParams param = getWindow().getAttributes();
                    param.screenBrightness = 0.01f;
                    getWindow().setAttributes(param);
                    
                    mBlackScreen.setVisibility(View.VISIBLE);
//                    if (mWakeLock == null) {
//                        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
//                        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
//                    }
//                    if (!mWakeLock.isHeld())
//                        mWakeLock.acquire();
                }
            });
            return "OK";
        }   

        @Override 
        public InputStream streaming(Properties parms) {
            return null;
        }    
    }; 


    static private native int nativeOpenEncoder();
    static private native void nativeCloseEncoder();
    static private native int nativeEncodingPCM(byte[] pcmdata, int length, byte[] mp3Data);    
    private class AudioEncoder extends Thread {
        byte[] audioPackage = new byte[1024*16];
        byte[] mp3Data = new byte[1024*8];
        int packageSize = 4410 * 2;
        @Override
        public void run() {
            nativeOpenEncoder(); 
            
            OutputStream os = null;
            try {
                os = audioLoop.getOutputStream();
            } catch(IOException e) {
                os = null;
                audioLoop.ReleaseLoop();
                nativeCloseEncoder();
                return;
            }
            
            while(true) {

                int ret = audioCapture.read(audioPackage, 0, packageSize);
                if ( ret == AudioRecord.ERROR_INVALID_OPERATION ||
                        ret == AudioRecord.ERROR_BAD_VALUE) {
                    break; 
                }

                //TODO: call jni encoding PCM to mp3
                ret = nativeEncodingPCM(audioPackage, ret, mp3Data);          
                
                try {
                    os.write(mp3Data, 0, ret);
                } catch(IOException e) {
                    break;    
                }
            }
            
            audioLoop.ReleaseLoop();
            nativeCloseEncoder();
        }
    }

    
    static private native String nativeQueryInternet();    
    private class NatPMPClient extends Thread {
        String queryResult;
        Handler handleQueryResult = new Handler(getMainLooper());  
        @Override
        public void run(){
            queryResult = nativeQueryInternet();
            if ( queryResult.startsWith("error:") ) {
                handleQueryResult.post( new Runnable() {
                    @Override
                    public void run() {
                        tvMessage2.setText( getString(R.string.msg_access_query_error));                        
                    }
                });
            } else {
                handleQueryResult.post( new Runnable() {
                    @Override
                    public void run() {
                        tvMessage2.setText( getString(R.string.msg_access_internet) + " " + queryResult );
                    }
                });
            }
        }    
    }
}    

