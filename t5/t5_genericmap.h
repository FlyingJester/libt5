#pragma once

#include "t5_datasource.h"

#include <string>
#include <list>
#include <algorithm>
#include <iterator>

namespace t5 {
namespace map{
    typedef std::string string;
    
    class Group;
    
    class Entry {
    public:
        Entry(const string&, Group *);
        virtual ~Entry();
        string Name;
        Group *Parent;
        
        ////
        // Get the string representation of this in each format.
        virtual string GetAsINI(unsigned aStep) = 0;
        virtual string GetAsJSON(unsigned aStep) = 0;
        
        virtual inline bool SupportsAnonVals(){
            return false;
        }
        
    };
    
    class Group : public Entry {
    public:
    
        Group(const string &aName, Group *group)
          : Entry(aName, group){
        }
        
        Group(const string &lName, const string &lFrom, Group *group);
        
        template<class Container>
        Group(Container *C){
            Contents.resize(C->size());
            std::copy(C->begin(), C->end(), Contents.begin());
        }
        
        enum ConfigType {
            nullConfig,
            iniConfig,
            jsonConfig,
        };
        
        
        /////
        // Read Functions:
        // Adds to the mapping from a data source. The range of bytes from 
        //  aRangeStart to aRangeLen are considered the valid config data,
        //  unless both are 0, in which case the entire DataSource is.
        //  
        
        // Attempt to guess the DataSource format, returning the guess.
        ConfigType ReadDataSource(DataSource *aFrom , size_t aRangeStart = 0, size_t aRangeLen = 0);
        
        // Read the DataSource as an INI config.
        void ReadDataSourceINI(DataSource *aFrom , size_t aRangeStart = 0, size_t aRangeLen = 0);
        
        // Read the DataSource as a JSON config.
        void ReadDataSourceJSON(DataSource *aFrom , size_t aRangeStart = 0, size_t aRangeLen = 0);
        
        /////
        // Write Functions:
        //  These functions write to a DataSource in the specified format.
        //
        // The Write functions simply write out the mapping.
        //
        // The Rewrite functions update any values in the given range, and
        //  update were possible. Otherwise, the datasource may be overwritten.
        //
        
        // Write as INI-style
        void WriteINI(DataSource *aTo);
        
        // Write as JSON
        void WriteJSON(DataSource *aTo);
        
        // Try to guess the DataSource format, and rewrite it. 
        size_t Rewrite(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar = " ");
        
        // Rewrite as INI-style
        size_t RewriteINI(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar = " ");
        
        // Rewrite as JSON
        size_t RewriteJSON(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar = " ");
        
        /////
        // Attempt to identify the format of a DataSource. Same semantics as ReadDataSource.
        static ConfigType AnalyzeDataSource(DataSource *aFrom , size_t aRangeStart = 0, size_t aRangeLen = 0);
        
        /////
        // Remove either by iterator or pointer.
        // This is similar to 'forget' or 'release' in a smart pointer.
        
        Entry *RemoveElement(std::list<Entry *>::iterator aElement);
        Entry *RemoveElement(Entry *aGive);
        
        /////
        // Accept the given element.
        
        void AcceptElement(Entry *aTake);
        
        ////
        // Get the string representation of this in each format.
        virtual string GetAsINI(unsigned aStep);
        virtual string GetAsJSON(unsigned aStep);
        
        std::list<Entry *> Contents;
    
    };
    
    class Array : public Group {
    public:

        Array(string &name, Group *group);
        virtual string GetAsINI(unsigned aStep);
        virtual string GetAsJSON(unsigned aStep);
        
        virtual inline bool SupportsAnonVals(){
            return true;
        }
    };
    
    class Value : public Entry{
    public:
        string RawValue;
        Value(const string &name, const string &value, Group *group)
          : Entry(name, group)
          , RawValue(value)
        {}
        
        virtual int     ToInt();
        virtual double  ToDouble();
        virtual bool    ToBool();
                
        ////
        // Get the string representation of this in each format.
        virtual string GetAsINI(unsigned aStep);
        virtual string GetAsJSON(unsigned aStep);
        
    };

}
}
