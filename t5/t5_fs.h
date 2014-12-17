#pragma once
#include <string>
#include <memory>
#include <list>

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
        std::shared_ptr<EntryGuts> Eguts;
        std::shared_ptr<Directory> mParent;
        std::string mName;
        Entry(const std::string &path);
    public:

        class EntryIterator {
        public:
            std::list<Entry *>::iterator mSelf;

            EntryIterator();
            ~EntryIterator();
            EntryIterator &operator+(int i);
            EntryIterator &operator++();
            EntryIterator &operator--();
            EntryIterator &operator=(const EntryIterator &rValue);
            Entry &operator*();
            Entry *operator->();
            bool operator!=(const EntryIterator &rValue);
            bool operator==(const EntryIterator &rValue);
        };

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
    class Directory : public Entry {
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

        Directory* AsDirectory() override {return this;}
    };

    class File : public Entry {

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
