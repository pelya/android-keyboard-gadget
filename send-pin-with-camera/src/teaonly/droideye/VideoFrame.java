package teaonly.droideye;

import android.util.Log;
import java.io.*;

class VideoFrame extends OutputStream {
    private byte[] buffer;
    private int bufferLength;
    private int currentLength;
    private int flag;

    public VideoFrame (int maxSize) {
        super();
        buffer = new byte[maxSize];
        bufferLength = maxSize;
        currentLength = 0;
        flag = 0;
    }

    public boolean acquire() {
        synchronized(this) {
            if ( flag == 0) {
                flag = 1;
                return true;
            } else {
                return false;
            }
        }
    }

    public void release() {
        synchronized(this) {
            if ( flag == 1) {
                flag = 0;
            }
        }
    }

    public InputStream getInputStream() {
        return videoInputStream;
    }

    public void reset() {
        currentLength = 0;
    }

    @Override
    public void write(int oneByte) throws IOException{
        if ( currentLength >= bufferLength) {
            IOException ex = new IOException("Buffer overflow");
            throw ex;
        } 
        buffer[currentLength] = (byte)(oneByte & 0xFF);
        currentLength++;
    }

    @Override
    public void close() throws IOException {
        super.close();
    }

    private InputStream videoInputStream = new InputStream() {
        private int rIndex = 0;
        
        @Override
        public int available() throws IOException{
            return currentLength - rIndex;                 
        }   

        @Override
        public int read (byte[] outBuffer, int offset, int length) throws IOException {
            int len = length;
            if ( length > available() )
                len = available();
            if ( len == 0)
                return 0;
            try{
                System.arraycopy(buffer, rIndex, outBuffer, offset, len);
            }catch(Exception ex) {
                return -1;
            }
            rIndex += len;
            return len;
        }

        @Override 
        public void close() throws IOException{
            rIndex = 0;
            release();
        }
        
        @Override
        public int read() throws IOException{
            if ( rIndex >= currentLength) {
                IOException ex = new IOException("Buffer overflow");
                throw ex;
            }
            int ret = buffer[rIndex];
            rIndex++;
            return ret;     
        }
    };
} 

