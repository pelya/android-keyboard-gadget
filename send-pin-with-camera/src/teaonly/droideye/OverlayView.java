package teaonly.droideye;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class OverlayView extends View {
    public static interface UpdateDoneCallback { 
        public void onUpdateDone(); 
    }  
   
    private UpdateDoneCallback updateDoneCb = null; 
    private Bitmap targetBMP = null;
    private Rect targetRect = null;
    private Paint paint = new Paint();
    

    public OverlayView(Context c, AttributeSet attr) {
        super(c, attr); 
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(2);
    }

    public void DrawResult(Bitmap bmp) {
        if ( targetRect == null)
            targetRect = new Rect(0, 0, bmp.getWidth(), bmp.getHeight());
        targetBMP = bmp;
        postInvalidate(); 
    }

    public void setUpdateDoneCallback(UpdateDoneCallback cb) {
        updateDoneCb = cb;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if ( targetBMP != null ) {            
            
            canvas.drawBitmap(targetBMP, null, targetRect, null);
                        
        }

		
		int picWidth = getWidth(); //cameraView_.Width();
		int picHeight = getHeight();  //cameraView_.Height(); 

		int y = picHeight / 2;
		for (int i = 0; i < MainActivity.PIN_LENGTH; i++)
		{
			int x = picWidth / 2 + (int)((i + 0.5f - MainActivity.PIN_LENGTH / 2) * MainActivity.SYMBOL_SPACING * picWidth);
			if (MainActivity.FRONT_CAMERA) // It's inverted
				x = picWidth - x;
			paint.setColor(MainActivity.pinEntered[i] ? Color.GREEN : Color.BLUE);
			canvas.drawCircle(x, y, 5, paint);
		}
		if ( updateDoneCb != null)
			updateDoneCb.onUpdateDone();
    }

}
