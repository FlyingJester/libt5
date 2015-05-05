#pragma once

#include "t5_stub.h"

#include <string>
#include <memory>
#include <list>

namespace t5 {

    class DataSource;

    // Heavyweight filesystem iterators. If you ONLY want to know what files
    //   exist in a certain directory, check out the fs_iterator in t5_fsops.h

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



    class T5_EXPORT Entry {
    protected:
        std::shared_ptr<EntryGuts> Eguts;
        std::shared_ptr<Directory> mParent;
        std::string mName;
        Entry(const std::string &path);
    public:

        class T5_EXPORT EntryIterator {
        public:
            std::list<Entry *>::iterator mSelf;

            EntryIterator();
            ~EntryIterator();
            EntryIterator &operator+(int i);
            EntryIterator &operator++();
            EntryIterator &operator--();
            EntryIterator operator++(int);
            EntryIterator operator--(int);
            EntryIterator &operator=(const EntryIterator &rValue);
            Entry &operator*() const;
            Entry *operator->() const;
            bool operator!=(const EntryIterator &rValue) const;
            bool operator==(const EntryIterator &rValue) const;
        };
        
        typedef EntryIterator iterator;
        
        Directory *Parent() const{
            return mParent.get();
        }

        const std::string &Name() const{
            return mName;
        }

        Entry();
        virtual ~Entry();

        static Entry* FromPath(const std::string &aPath);

        // An entry can work as a file, a directory, both or neither.
        virtual File *AsFile() {return nullptr;}
        virtual Directory* AsDirectory() {return nullptr;}

        virtual const std::string &GetPath() const; //Returns the path representation of the entry.

    };

    /////
    // A 'Directory' structure. This may be nothing of the sort. It is
    //  anything that holds multiple entries uniquely as its own (which
    //  inludes directories, but also archives and 'compressed folders').
    //
    class T5_EXPORT Directory : public Entry {
    protected:
        std::shared_ptr<DirectoryGuts> Dguts;

    public:

        typedef Entry::EntryIterator iterator;

    private:

    protected:
        Directory(const std::string &path);

    public:

        friend class Entry;

        iterator begin(void);
        iterator end(void);

        Directory();
        virtual ~Directory();

        virtual Directory* AsDirectory() {return this;}
    };

    class T5_EXPORT File : public Entry {

        File(const std::string &path);
    public:

        friend class Entry;

        File(){}
        virtual ~File(){}
        DataSource *AsSource();
        virtual File *AsFile() {return this;}
    };



}
}
