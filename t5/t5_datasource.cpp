#include "t5_datasource.h"
#include <cstdio>
#include <string>

#ifndef _WIN32
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/mman.h>
#endif
#ifdef USE_ZLIB
    #include <zlib.h>
#endif

namespace t5 {

    /////
    // Simple wrappers around stdout, stderr, and stdin.
    //
    DataSourcePusher *DataSource::StdOut(){
        static DataSourcePusher *stdout_ = OneWayFromCFile<DataSourcePusher>(stdout);
        return stdout_;
    }

    DataSourcePusher *DataSource::StdErr(){
        static DataSourcePusher *stderr_ = OneWayFromCFile<DataSourcePusher>(stderr);
        return stderr_;
    }

    DataSourcePuller *DataSource::StdIn(){
        static DataSourcePuller *stdin_ = OneWayFromCFile<DataSourcePuller>(stdin);
        return stdin_;
    }

    size_t DataConverter::size(void) {return 0;}

    /////
    // A class for data converters that uses buffers that are fast to allocate.
    //
    // This is useful for smaller-buffered and short lifetime converters,
    //  as well as converters such as zlib that allow for externally allocated
    //  buffers.
    //
    // On platforms with slow or simply no fast page allocation, these objects
    //  just use malloc and free.
    //
    template<size_t tBufferSize>
    class FastBufferConverter {
    public:
        uint8_t *mData;
#ifdef _WIN32
    private:
        uint8_t mSecretData[tBufferSize]
    public:
        FastBufferConverter(){
            mData = mSecretData;
        }
        ~FastBufferConverter(){
            mData = nullptr;
        }
#else
        FastBufferConverter(){
            mData = (uint8_t *)mmap(nullptr, tBufferSize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
        }
        ~FastBufferConverter(){
            munmap(mData, tBufferSize);
            mData = nullptr;
        }
#endif
    };

#ifdef USE_ZLIB
    class zDataConverter : public DataConverter {
    public:
        static const size_t cBufferSize = 0xFFFF;

    protected:

        enum {PIPE_IN, PIPE_OUT};
        DataSource *mPipe;

        FastBufferConverter<cBufferSize> mInBuffer;
        FastBufferConverter<cBufferSize> mOutBuffer;
        z_stream mStream;
    public:
        zDataConverter(){

            mPipe = DataSource::DataPipe(DataSource::eRead | DataSource::eWrite);

            mStream.zalloc = Z_NULL;
            mStream.zfree  = Z_NULL;
            mStream.opaque = Z_NULL;
        }

        virtual ~zDataConverter(){
            delete mPipe;
        }

        virtual void Write(DataSource *aDataIn, size_t aLen){
            assert(aDataIn!=nullptr);

            mPipe->Read(mOutBuffer.mData, aLen);

            aDataIn->Write(mOutBuffer.mData, aLen);
        }
    };

    // Everything read in by this converter is processed using INFLATE.
    class zInflator : public zDataConverter {

    public:
        zInflator()
        : zDataConverter()
        {
            int err = inflateInit(&mStream);
            assert(err == Z_OK);
        }

        void Read(DataSource *aDataIn, size_t aLen){
            assert(aDataIn!=nullptr);
            assert(aDataIn->Length()-aDataIn->Tell() >= aLen);
            size_t lProcessed = 0;



            mStream.avail_out = cBufferSize;

            while(lProcessed<aLen){
                size_t lToRead = std::min<size_t>(cBufferSize, aLen-lProcessed);

                aDataIn->Read(mInBuffer.mData, lToRead);

                mStream.avail_in = (unsigned)lToRead;
                mStream.next_in  = mInBuffer.mData;
                mStream.next_out = mOutBuffer.mData;

                inflate(&mStream, Z_NO_FLUSH);

                lProcessed += mStream.avail_out;

                /////
                // TODO: Use the PIPE_IN as the base for mOutBuffer's mapping. That way
                // we can just use a sendfile or whatever here and avoid th extra read.
                mPipe->Write(mOutBuffer.mData, mStream.avail_out);
            }

        }

    };

    // Everything read in by this converter is processed using DEFLATE.
    class zDeflator : public zDataConverter {

    public:
        zDeflator()
        : zDataConverter()
        {
            int err = deflateInit(&mStream, Z_BEST_COMPRESSION);
            assert(err == Z_OK);
        }

        virtual ~zDeflator(){
            deflateEnd(&mStream);
        }

        void Read(DataSource *aDataIn, size_t aLen){
            assert(aDataIn!=nullptr);
            assert(aDataIn->Length()-aDataIn->Tell() >= aLen);
            size_t lProcessed = 0;



            mStream.avail_out = cBufferSize;

            while(lProcessed<aLen){
                size_t lToRead = std::min<size_t>(cBufferSize, aLen-lProcessed);

                aDataIn->Read(mInBuffer.mData, lToRead);

                mStream.avail_in = (unsigned)lToRead;
                mStream.next_in  = mInBuffer.mData;
                mStream.next_out = mOutBuffer.mData;

                deflate(&mStream, (lToRead==cBufferSize)?Z_NO_FLUSH:Z_FINISH);

                lProcessed += mStream.avail_out;

                /////
                // TODO: Use the PIPE_IN as the base for mOutBuffer's mapping. That way
                // we can just use a sendfile or whatever here and avoid th extra read.
                mPipe->Write(mOutBuffer.mData, mStream.avail_out);
            }

        }

    };
#endif

    DataSource::~DataSource(){}

    DataSource::DataSource(int aAccess)
    : mAccess(aAccess){

    }

#ifndef _WIN32
    class FDFileSource : public DataSource {
    protected:
        int mFD;
    public:
        FDFileSource(int aAccess) : DataSource(aAccess) {mFD = -1;}
        FDFileSource(int aAccess, int aFD)
        : DataSource(aAccess)
        , mFD(aFD)
        {

        }

        FDFileSource(int aAccess, const char *aPath)
          : DataSource(aAccess) {

            int lFlags = 0;
            if((aAccess&eWrite) && (aAccess&eRead))
                lFlags |= O_RDWR;
            else{
                if(aAccess&eRead)
                    lFlags |= O_RDONLY;
                if(aAccess&eWrite)
                    lFlags |= O_WRONLY;
            }

            if(aAccess&eAppend)
                lFlags |= O_APPEND;

            mFD = open(aPath, lFlags);
            // Ensure that we can perform non-blocking reads.
            fcntl(mFD, F_SETFL, O_NONBLOCK);
        }

        virtual ~FDFileSource(){
            close(mFD);
        }

        virtual void Read(void *aTo, size_t aLen){
            read(mFD, aTo, aLen);
        }

        virtual void Write(const void *aTo, size_t aLen){
            write(mFD, aTo, aLen);
        }

        virtual size_t Tell(){
            return lseek(mFD, 0, SEEK_CUR);
        }

        virtual size_t Length(){
            struct stat lStat;
            fstat(mFD, &lStat);
            return lStat.st_size;
        }

        virtual void Seek(size_t aTo, int aWhence){
                    auto lWhence = SEEK_SET;
            switch(aWhence){
                case eStart:
                    lWhence = SEEK_SET;
                break;
                case eEnd:
                    lWhence = SEEK_END;
                break;
                case eIndex:
                    lWhence = SEEK_CUR;
                break;
            }
            lseek(mFD, aTo, lWhence);
        }

    };

    #include <cerrno>

    class mmapDataSource : public FDFileSource{
    uint8_t *BufferData;
    size_t BufferNeedle;
    size_t BufferSize;
    public:
        mmapDataSource(int aAccess, const char *aPath)
        : FDFileSource(aAccess,aPath)
        , BufferData((uint8_t *)mmap(nullptr, Length(),
            (((aAccess&eRead)==eRead)?PROT_READ:0)|(((aAccess&eRead)==eWrite)?PROT_WRITE:0),
            MAP_SHARED, mFD, 0))
        , BufferNeedle(0)
        , BufferSize(Length())
        {

        }
        virtual ~mmapDataSource(){
            munmap(BufferData, BufferSize);
        }

        virtual void Seek(size_t aTo, int aWhence){
            switch(aWhence){
                case eStart:
                BufferNeedle = aTo;
                break;
                case eEnd:
                BufferNeedle = BufferSize - aTo;
                break;
                case eIndex:
                BufferNeedle += aTo;
                break;
            }

        }

        virtual void Read(void *aTo, size_t aLen){

            if(BufferNeedle+aLen>BufferSize)
                return;

            memcpy(aTo, BufferData+BufferNeedle, aLen);
            BufferNeedle+=aLen;
        }

        virtual char GetC(void){
            BufferNeedle++;
            return BufferData[BufferNeedle-1];
        }

        virtual size_t Tell(){
            return BufferNeedle;
        }

    };

    class PipeSource : public FDFileSource {
    public:
        enum {FD_IN, FD_OUT};

    protected:
        int mFDs[2];//mFDin, mFDout;

    public:
        PipeSource(int aAccess)
        : FDFileSource(0, aAccess) {
            pipe(mFDs);
            mFD = mFDs[FD_OUT];

            fcntl(mFDs[FD_IN],  F_SETFD, PROT_READ);
            fcntl(mFDs[FD_OUT], F_SETFD, PROT_WRITE);

        }

        virtual void Write(const void *aFrom, size_t aLen){
            write(mFDs[FD_IN], aFrom, aLen);
        }

    };


#endif

    class CFileSource : public DataSource {
    protected:
        FILE *mFile;
    public:
        CFileSource(int aAccess) : DataSource(aAccess) {mFile = NULL;}
        CFileSource(int aAccess, const char *aPath)
          : DataSource(aAccess) {
            std::string lFlags = "";
            if(aAccess&eRead)
                lFlags.append("r");
            if(aAccess&eWrite)
                lFlags.append("w");
            if(aAccess&eAppend)
                lFlags.append("a");


            mFile = fopen(aPath, lFlags.c_str());

        }

        CFileSource(int aAccess, FILE *aFile)
           : DataSource(aAccess){
            mFile = aFile;
        }

        ~CFileSource() {}


        virtual void Read(void *aTo, size_t aLen){
            fread(aTo, 1, aLen, mFile);
        }

        virtual void Write(const void *aTo, size_t aLen){
            fwrite(aTo, 1, aLen, mFile);
        }

        virtual size_t Tell(){
            return ftell(mFile);
        }

        virtual size_t Length(){
            auto lCur = ftell(mFile);

            fseek(mFile, 0, SEEK_END);
            auto rCur = ftell(mFile);

            fseek(mFile, lCur, SEEK_SET);
            return rCur;
        }

        virtual void Seek(size_t aTo, int aWhence){
            auto lWhence = SEEK_SET;
            switch(aWhence){
                case eStart:
                    lWhence = SEEK_SET;
                break;
                case eEnd:
                    lWhence = SEEK_END;
                break;
                case eIndex:
                    lWhence = SEEK_CUR;
                break;
            }

            fseek(mFile, aTo, lWhence);

        }

    };

    DataSourcePassThrough::DataSourcePassThrough(DataSource *aFrom)
      : DataSource(eRead|eWrite){
        mSource = aFrom;
    }
    void DataSourcePassThrough::Read(void *aTo, size_t aLen){mSource->Read(aTo, aLen);}
    void DataSourcePassThrough::Write(const void *aTo, size_t aLen){mSource->Write(aTo, aLen);}

    size_t DataSourcePassThrough::Tell(){return mSource->Tell();}
    size_t DataSourcePassThrough::Length(){return mSource->Length();}

    void DataSourcePassThrough::Seek(size_t aTo, int aWhence){mSource->Seek(aTo, aWhence);}


    DataSourcePuller::DataSourcePuller(DataSource *aFrom)
      : DataSourcePassThrough(aFrom){

      }

    DataSourcePusher::DataSourcePusher(DataSource *aFrom)
      : DataSourcePassThrough(aFrom){

      }

    void DataSourcePusher::Read(void *aTo, size_t aLen) {}
    void DataSourcePuller::Write(const void *aTo, size_t aLen){}
    size_t DataSourcePusher::Tell()     {return 0;}
    size_t DataSourcePusher::Length()   {return 0;}
    size_t DataSourcePuller::Tell()     {return 0;}
    size_t DataSourcePuller::Length()   {return 0;}

    DataSource* DataSource::FromPath(int aAccess, const char *aPath){
    #ifndef _WIN32
        return new FDFileSource(aAccess, aPath);
    #else
        return new CFileSource(aAccess, aPath);
    #endif
    }


    DataSource *DataSource::FromCFileHandle(void *aFile){
        return new CFileSource(eRead|eWrite, (FILE *)aFile);
    }

    DataSource *DataSource::DataPipe(int aAccess){
    #ifndef _WIN32
        return new PipeSource(aAccess);
    #endif
    }

    DataSourcePusher *DataSource::AsPusher(){
        return new DataSourcePusher(this);
    }
    DataSourcePuller *DataSource::AsPuller(){
        return new DataSourcePuller(this);
    }


}
