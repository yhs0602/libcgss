package moe.mottomo.cgss.streams;

/**
 * Created by MIC on 2017-01-28.
 */
public final class MemoryStream extends Stream {

    public MemoryStream() {
        this(0);
    }

    public MemoryStream(long capacity) {
        this(capacity, true);
    }

    public MemoryStream(long capacity, boolean resizable) {
        initNewBuffer(capacity, resizable);
    }

    public MemoryStream(byte[] buffer) {
        this(buffer, true);
    }

    public MemoryStream(byte[] buffer, boolean writable) {
        if (buffer == null) {
            throw new IllegalArgumentException("MemoryStream.ctor:buffer");
        }
        initExtBuffer(buffer, writable);
    }

    @Override
    public native int read(byte[] buffer, int offset, int count);

    @Override
    public native int write(byte[] buffer, int offset, int count);

    @Override
    public native boolean isWritable();

    @Override
    public native boolean isReadable();

    @Override
    public native boolean isSeekable();

    @Override
    public native long getPosition();

    @Override
    public native void setPosition(long position);

    @Override
    public native long getLength();

    @Override
    public native void setLength(long length);

    @Override
    public native void flush();

    private native void initNewBuffer(long capacity, boolean resizable);

    private native void initExtBuffer(byte[] buffer, boolean writable);

    @Override
    protected native void dispose();

    private byte[] buffer;

}
