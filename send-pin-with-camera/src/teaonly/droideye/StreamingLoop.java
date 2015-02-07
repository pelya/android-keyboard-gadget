package teaonly.droideye;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.SystemClock;
import android.util.Log;


public class StreamingLoop
{
    private static String TAG = "TEAONLY";
	//Local data loopback
	private LocalSocket receiver,sender;			
	private LocalServerSocket lss;		
	private String localAddress;
    private boolean isConnected_ = false;

	public StreamingLoop (String addr)	
	{
		localAddress = addr;
        try {
			lss = new LocalServerSocket(localAddress);
		} catch (IOException e) {
			e.printStackTrace();
		}		
	}
	
    public InputStream getInputStream() throws IOException{
       return receiver.getInputStream(); 
    }

    public OutputStream getOutputStream() throws IOException{
       return sender.getOutputStream();
    }

	public void ReleaseLoop()
	{
		try {
			if ( receiver != null){
				receiver.close();
			}
			if ( sender != null){
				sender.close();
			}
		} catch (IOException e1) {
			e1.printStackTrace();
			Log.d(TAG, e1.toString());			
		}
		
		sender = null;
		receiver = null;
        isConnected_ = false;
	}

	public boolean InitLoop(int recvBufferSize, int sendBufferSize)
	{		
        receiver = new LocalSocket();
		try {
			receiver.connect(new LocalSocketAddress(localAddress));
			receiver.setReceiveBufferSize(recvBufferSize);
			//receiver.setSendBufferSize(512);
			sender = lss.accept();
			//sender.setReceiveBufferSize(512);
			sender.setSendBufferSize(sendBufferSize);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}		
		isConnected_ = true;
        return true;
	}

    public boolean isConnected() {
        return isConnected_;
    }
}
