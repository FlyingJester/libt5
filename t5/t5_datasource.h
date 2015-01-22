#pragma once

#include <string>
#include <cstdlib>
#include <cstring>

namespace t5 {

class DataConverter;
class DataSourcePusher;
class DataSourcePuller;

class DataSource {
public:
    enum {
        eRead       = 1<<0,
        eWrite      = 1<<1,
        eNewIf      = 1<<2,
        eNewOnly    = 1<<3,
        eAppend     = 1<<4,
    };

    enum {
        eStart,
        eEnd,
        eIndex,
    };

    // Creates a data source from a file. Uses the most native file reading API.
    static DataSource *FromPath(int aAccess, const char *path);
    // Creates a data source from a C file handle.
    static DataSource *FromCFileHandle(void /*FILE*/ *aFile);
    // Creates a data source that reads out what is written into it. Same functionally as a Unix Pipe.
    static DataSource *DataPipe(int aAccess);
    // Creates a simple push/pull data source from aFrom.
    static DataSource *FromSource(DataSource *aFrom);
    // Creates a one-way data source from another source.
    static DataSource *PusherFromSource(DataSource *aFrom);
    static DataSource *PullerFromSource(DataSource *aFrom);
    // Creates a data source with a dataconverter from source. Note that most converters only work one way.
    static DataSource *ConverterFromSource(DataSource *aFrom, DataConverter *aUsing);

    // Create a pusher or puller based on this data source.
    DataSourcePusher *AsPusher();
    DataSourcePuller *AsPuller();

    // Simple wrappers around stdout, stderr, and stdin.
    static DataSourcePusher *StdOut();
    static DataSourcePusher *StdErr();
    static DataSourcePuller *StdIn();

    template<class A /*Derived from DataSource*/>
    static A *OneWayFromCFile(void /*FILE*/ *aFile){
        t5::DataSource *lSource = t5::DataSource::FromCFileHandle(aFile);
        return new A(lSource);
    }
    
    #ifdef USE_ZLIB
    static DataSource *CreateZlib(DataSource *aFrom);
    #endif

protected:

    int mAccess;

public:
    virtual ~DataSource();
    DataSource(int aAccess);

    virtual void Read(void *aTo, size_t aLen) = 0;
    virtual void Write(const void *aTo, size_t aLen) = 0;

    template<typename T>
    size_t Put(T a){
        Write(&a, sizeof(T));
        return sizeof(T);
    }
    
    inline void WriteS(const char *aString){
        Write(aString, strlen(aString));
    }
    
    inline void WriteS(const std::string &aString){
        Write(aString.c_str(), aString.length());
    }

    template<typename T, int Base, int BufferSize /*Should be 32 or greater*/>
    void WriteWithNumericBase(T i){
        char buffer[BufferSize], rbuffer[BufferSize];
        int at_ = 0, rat_ = 0;

        memset(buffer,  0, BufferSize);
        memset(rbuffer, 0, BufferSize);

        while(i>0 && (at_<BufferSize)){
            char c = (i%Base)+'0';

            if(c>'9'){
                c-='0';
                c+='a';
            }

            rbuffer[at_++] = c;
            i/=Base;
            i-=i%Base; // Theoretically for floating point numbers.
        }

        // We have it backwards.
        while(at_>=0) buffer[rat_++] = rbuffer[at_--];

        Write(buffer, --rat_);

    }
    
    template<typename T, typename ...ARGS>
    void WriteF(T first, ARGS... args){
        WriteF(first);
        WriteF(args...);
    }

    template<typename T>
    void WriteF(T a){
        
        if(std::is_integral<T>::value)
            Put(a);
        else
            WriteS(a);
    }
    
    virtual size_t Tell() = 0;
    virtual size_t Length() = 0;
    virtual void Seek(size_t aTo, int aWhence) = 0;

    template<typename T>
    T Get(void){
        T rData;
        Read(&rData, sizeof(rData));
        return rData;
    }

    virtual char GetC(void){
        return Get<char>();
    }


};

class DataSourcePassThrough : public DataSource {
protected:
    DataSource *mSource;
public:
    virtual ~DataSourcePassThrough() {}
    DataSourcePassThrough(DataSource *aThrough);

    virtual void Read(void *aTo, size_t aLen);
    virtual void Write(const void *aTo, size_t aLen);
    virtual size_t Tell();
    virtual size_t Length();
    virtual void Seek(size_t aTo, int aWhence);
};

class DataSourcePusher : public DataSourcePassThrough {
public:
    virtual ~DataSourcePusher() {}
    DataSourcePusher(DataSource *aPushTo);

    virtual void Read(void *aTo, size_t aLen);
    virtual size_t Tell();
    virtual size_t Length();
    virtual void Seek(size_t aTo, int aWhence){};
};

class DataSourcePuller : public DataSourcePassThrough {
public:
    virtual ~DataSourcePuller() {}
    DataSourcePuller(DataSource *aPullFrom);

    virtual void Write(const void *aTo, size_t aLen);
    virtual size_t Tell();
    virtual size_t Length();
    virtual void Seek(size_t aTo, int aWhence){};
};

/////
// A synchronous data converter.
class DataConverter {
public:
    DataConverter() {};
    virtual ~DataConverter() {};

    virtual void Read(DataSource *aDataIn, size_t aLen) = 0;
    virtual void Write(DataSource *aDataIn, size_t aLen) = 0;
    virtual void Process(void) = 0;
    virtual size_t size(void);

};

}
