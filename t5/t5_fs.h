#pragma once
#include <string>

namespace t5 {

    class DataSource;

namespace fs {
    class EntryGuts;
    class DirectoryGuts;

    class Directory;
    class File;

    /////
    // Returns a fs::Directory representation of the current workig directory.
    //
    Directory *GetCWD();

    /////
    // Generic FS entry. Could be a file, could be a directory, could
    //  be some crazy hardware node, who knows?
    //



    class Entry {
    protected:
        EntryGuts *Eguts;
        Directory *mParent;
        std::string mName;
    public:

        class EntryIterator {
            Entry *mSelf;
        public:
            EntryIterator();
            ~EntryIterator();
            void operator++();
            Entry *operator*();
            bool operator!=(const EntryIterator &rValue);
        };

        Directory *Parent() const{
            return mParent;
        }

        const std::string &Name() const{
            return mName;
        }

        Entry();
        virtual ~Entry();

        virtual void Flush(void) = 0;

        static Entry* FromPath();

        // An entry can work as a file, a directory, both or neither.
        File *AsFile();
        Directory* AsDirectory();

        virtual const std::string &GetPath(); //Returns the path representation of the entry.

    };

    /////
    // A 'Directory' structure. This may be nothing of the sort. It is
    //  anything that holds multiple entries uniquely as its own (which
    //  inludes directories, but also archives and 'compressed folders').
    //
    class Directory : public Entry {
    protected:
        DirectoryGuts *Dguts;

    public:

        typedef EntryIterator iterator;

    private:

    protected:

    public:

        iterator begin(void);
        iterator end(void);

        Directory();
        virtual ~Directory();


    };

    class File : public Entry {
        DataSource *AsSource();
    };



}
}
