#include "t5_fs.h"
#include <list>
#include <stack>
#include "t5_datasource.h"

namespace t5 {
namespace fs {

    
    class EntryGuts{
    public:
        Entry *self;
        Entry *next;
        Entry *prev;
    
        bool CanBeFile;
        bool CanBeDirectory;
    
        std::string path;
        
        DataSource *source; //Only valid if CanBeFile is true.
        
    };
    
    class DirectoryGuts{
    public:
        std::list<Entry *>Contents;
        bool IsRaw;
        bool IsArchive;
        
    };
    
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
    
    const std::string &Entry::GetPath(){
        return Eguts->path;
    }
    
    Directory::EntryIterator::EntryIterator(){
    
    }
    
    Directory::EntryIterator::~EntryIterator(){
    
    }
    
    void Directory::EntryIterator::operator++(){
        
    }
    
    Entry *Directory::EntryIterator::operator*(){
        return mSelf;
    }
    
    bool Directory::EntryIterator::operator!=(const EntryIterator &rValue){
        return mSelf==rValue.mSelf;
    }

    Directory::EntryIterator Directory::begin(void){
        Directory::EntryIterator rIter;

        return rIter;
    }
    
    Directory::EntryIterator Directory::end(void){
        Directory::EntryIterator rIter;
        return rIter;
    }
        
    Directory::Directory(){
    
    }
    Directory::~Directory(){
    
    }

    


}
}