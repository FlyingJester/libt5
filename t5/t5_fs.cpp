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
#include <unistd.h>


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

    inline bool NotSystemLink(const std::string &file_name){
        for(std::string::const_iterator iter = file_name.cbegin(); iter!=file_name.cend(); iter++){
            if((*iter!='.') && (*iter!='/')
#ifdef _WIN32
            && (*iter!='\\')
#endif
            ){
                return true;
            }
        }
        return false;
    }

    Directory::Directory(const std::string &path)
      : Entry(path) {
        Dguts.reset(new DirectoryGuts);

        assert(t5::IsDir(path.c_str()));

        { //ensures no platform-specific variables leak out.
#ifdef _WIN32

            WIN32_FIND_DATA dirp;
            HANDLE dirent_h = FindFirstFile((path+"\\*").c_str(), &dirp);

            while((GetLastError()!=ERROR_FILE_NOT_FOUND) && (GetLastError()!=ERROR_NO_MORE_FILE)){
                if(!FindNextFile(dirent_h, &dirp)) break;
                std::string file_name = dirp.cFileName;

                if(NotSystemLink(file_name)){
                    Entry *e = Entry::FromPath(path+"/"+file_name);
                    if(e) Dguts->Contents.push_back(e);
                }

            }

            FindClose(dirent_h);
#else

            DIR *dirp = opendir(path.c_str());

            long name_max = pathconf(path.c_str(), _PC_NAME_MAX);
            if(!name_max) name_max = 0xFF;
            int len = offsetof(struct dirent, d_name)+name_max+1;
            struct dirent *dirent_p = (struct dirent *)malloc(len);

            for(readdir_r(dirp, dirent_p, &dirent_p); dirent_p!=nullptr; readdir_r(dirp, dirent_p, &dirent_p)){
                if((dirent_p->d_type==DT_REG) || (dirent_p->d_type==DT_DIR)){
                    std::string file_name = dirent_p->d_name;

                    if(NotSystemLink(file_name)){
                        Entry *e = Entry::FromPath(path+"/"+file_name);
                        if(e) Dguts->Contents.push_back(e);
                    }
                }
            }

            closedir(dirp);

#endif
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

    Entry *Entry::EntryIterator::operator->() const{
        return *mSelf;
    }

    Entry &Entry::EntryIterator::operator*() const{
        return **mSelf;
    }

    bool Entry::EntryIterator::operator!=(const EntryIterator &rValue) const{
        return mSelf!=rValue.mSelf;
    }

    bool Entry::EntryIterator::operator==(const EntryIterator &rValue) const{
        return mSelf==rValue.mSelf;
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
