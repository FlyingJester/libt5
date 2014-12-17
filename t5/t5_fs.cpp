#include <list>
#include <stack>
#include <cassert>
#include "t5_fs.h"
#include "t5_fsops.h"
#include "t5_datasource.h"

#include <stack>

#ifdef _WIN32

#include <Windows.h>

#else

#include <dirent.h>

#endif

namespace t5 {
namespace fs {


    class EntryGuts{
    public:
        Entry *self;
        Entry *next;
        Entry *prev;

        bool CanBeFile;
        bool CanBeDirectory;

        DataSource *source; //Only valid if CanBeFile is true.

    };

    class DirectoryGuts{
    public:
        std::list<Entry *>Contents;
        bool IsRaw;
        bool IsArchive;
        bool IsInitialized;

    };

    Entry::Entry(const std::string &path)
      : mName(path){

    }

    File::File(const std::string &path)
       : Entry(path) {

    }

    Directory::Directory(const std::string &path)
      : Entry(path) {
        Dguts.reset(new DirectoryGuts);

        assert(t5::IsDir(path.c_str()));

        std::stack<std::string> entries;

#ifdef _WIN32


#else

        DIR *dirp = opendir(path.c_str());
        for(struct dirent *dir = readdir(dirp); dir!=nullptr; dir = readdir(dirp)){
            if((dir->d_type==DT_REG) || (dir->d_type==DT_DIR)){
                bool system_link = true;
                std::string file_name = dir->d_name;

                for(std::string::iterator iter = file_name.begin(); iter!=file_name.end(); iter++){
                    if(*iter!='.')
                        system_link = false;
                }

                if(!system_link){
                    printf("Reading %s.\n", dir->d_name);
                    entries.push(path+"/"+file_name);
                }
            }
        }

        closedir(dirp);

#endif

        while(!entries.empty()){
            printf("Reading %s.\n", entries.top().c_str());
            Entry *e = Entry::FromPath(entries.top());

            if(e){
                Dguts->Contents.push_back(e);
                printf("Adding entry %s (%s) (%p)\n", e->GetPath().c_str(), entries.top().c_str(), e);
            }
            else
                printf("Ignoring entry null (%s) (%p)\n", entries.top().c_str(), e);
            entries.pop();
        }

    }

    Entry* Entry::FromPath(const std::string &aPath){
        Entry *e = nullptr;

        if(t5::IsDir(aPath.c_str())){
            e = new Directory(aPath);
        }
        else if(t5::IsFile(aPath.c_str())){
            e = new File(aPath);
        }

        return e;

    }

    /////
    // Returns a new root from the next level up in aPath as aRoot.
    Entry *GetNextRoot(const Entry *aRoot, const std::string &aPath){
        std::stack<Entry *> roots;

        return nullptr;
    }

    Entry *PopElement(const std::string &Path){

        return nullptr;
    }

    Entry::Entry(){}
    Entry::~Entry(){}

    const std::string &Entry::GetPath() const{
        return mName;
    }

    Entry::EntryIterator::EntryIterator(){

    }

    Entry::EntryIterator::~EntryIterator(){

    }

    Entry::EntryIterator &Entry::EntryIterator::operator++(){
        mSelf++;
        return *this;
    }

    Entry::EntryIterator &Entry::EntryIterator::operator+(int i){

        for(int e = 0; e<i; e++) mSelf++;

        return *this;
    }

    Entry *Entry::EntryIterator::operator->(){
        return *mSelf;
    }

    Entry &Entry::EntryIterator::operator*(){
        return **mSelf;
    }

    bool Entry::EntryIterator::operator!=(const EntryIterator &rValue){
        return (*mSelf)->GetPath()!=const_cast<EntryIterator &>(rValue)->GetPath();
    }

    bool Entry::EntryIterator::operator==(const EntryIterator &rValue){
        return (*mSelf)->GetPath()==const_cast<EntryIterator &>(rValue)->GetPath();
    }


    Entry::EntryIterator &Entry::EntryIterator::operator=(const EntryIterator &rValue){

        return *this;
    }

    Entry::EntryIterator Directory::begin(void){
        Entry::EntryIterator rIter;
        rIter.mSelf = Dguts->Contents.begin();
        return rIter;
    }

    Entry::EntryIterator Directory::end(void){
        Entry::EntryIterator rIter;
        rIter.mSelf = Dguts->Contents.end();
        return rIter;
    }

    Directory::Directory(){

    }
    Directory::~Directory(){

    }




}
}
