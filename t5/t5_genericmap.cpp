#include "t5_genericmap.h"

#include <cassert>
#include <cctype>
#include <stack>

namespace t5 {
namespace map{

Entry::Entry(const string &aName, Group * aParent){
    Parent = aParent;
    std::string::const_iterator lIterf = aName.begin();
    
    while(isspace(*lIterf))
        lIterf++;
        
    std::string::const_reverse_iterator lIterr = aName.rbegin();
    
    while(isspace(*lIterr))
        lIterr++;
    
    Name = std::string(lIterf, lIterr.base());
    
}

Entry::~Entry(){}
    
Entry *Group::RemoveElement(std::list<Entry *>::iterator aElement){
    Entry *rEntry = *aElement;
    Contents.erase(aElement);
    return rEntry;
}
        
Entry *Group::RemoveElement(Entry *aGive){
    Entry *rEntry = aGive;
    Contents.remove(aGive);
    return rEntry;
}
        
void Group::AcceptElement(Entry *aTake){
    Contents.push_back(aTake);
}
    
        
int Value::ToInt(){
    return atoi(RawValue.c_str());
}

double Value::ToDouble(){
    return (double)atof(RawValue.c_str());

}

bool Value::ToBool(){
    if(atoi(RawValue.c_str())==1)
        return true;
    
    if(RawValue=="0")
        return false;
    
    string lString = string(RawValue);
    
    std::transform(lString.begin(), lString.end(), lString.begin(), tolower);
    
    if(lString=="true")
        return true;
    if(lString=="false")
        return false;
        
    if(lString[0]=='y')
        return true;
    if(lString[0]=='n')
        return false;

    return atoi(lString.c_str());
}

string Value::GetAsINI(unsigned aStep){
    string lRepresentation = "";
    
    lRepresentation.append(Name);
    lRepresentation.push_back('=');
    lRepresentation.append(RawValue);
    
    return lRepresentation;
}

string Value::GetAsJSON(unsigned aStep){
    string lRepresentation = "";
    
    
    lRepresentation.append(aStep*2, ' ');
    lRepresentation.push_back('"');
    lRepresentation.append(Name);
    lRepresentation.push_back('"');
    lRepresentation.push_back(':');
    
    // Now ready for payload.
    
    bool HasNumOnly = true;
    for(int i = 0; i<RawValue.length(); i++){
        if(!isdigit(RawValue[i])){
            HasNumOnly = false;
            break;
        }
    }
    
    if(HasNumOnly||(RawValue=="true")||(RawValue=="false")||(RawValue=="null"))
        lRepresentation.append(RawValue);
    else{
        lRepresentation.push_back('"');
        lRepresentation.append(RawValue);
        lRepresentation.push_back('"');
    }
    
    lRepresentation.push_back(',');
    
    return lRepresentation;
}

string Array::GetAsINI(unsigned aStep){
    string lRepresentation = "";
    
    lRepresentation.append(Name);
    lRepresentation.push_back('=');
    for(std::list<Entry *>::iterator lIter = Contents.begin(); lIter!=Contents.end(); lIter++){
        lRepresentation.append((*lIter)->Name);
        lRepresentation.push_back(',');
    
    }
    
    return lRepresentation;
}

string Array::GetAsJSON(unsigned aStep){
    string lRepresentation = "";
    
    lRepresentation.append(aStep*2, ' ');
    lRepresentation.push_back('"');
    lRepresentation.append(Name);
    lRepresentation.push_back('"');
    lRepresentation.push_back(':');
    lRepresentation.push_back('[');
    
    for(std::list<Entry *>::iterator lIter = Contents.begin(); lIter!=Contents.end(); lIter++){
        
        { // Payload
            bool HasNumOnly = true;
            for(int i = 0; i<(*lIter)->Name.length(); i++){
                if(!isdigit((*lIter)->Name[i])){
                    HasNumOnly = false;
                    break;
                }
            }
    
            if(HasNumOnly||((*lIter)->Name=="true")||((*lIter)->Name=="false")||((*lIter)->Name=="null"))
                lRepresentation.append((*lIter)->Name);
            else{
                lRepresentation.push_back('"');
                lRepresentation.append((*lIter)->Name);
                lRepresentation.push_back('"');
            }
        } // Payload
        
        if(lIter!=Contents.end()--){
            lRepresentation.push_back(',');
            lRepresentation.push_back(' ');
        }
    }
    lRepresentation.push_back(']');
    lRepresentation.push_back(',');
    
    return lRepresentation;

}

Array::Array(string &name, string value, Group *group)
  : Entry(name, group){
    
    std::string::iterator lPreviousComma        = value.begin();
    std::string::iterator lPreviousNonws        = value.begin();
    std::string::iterator lIter                 = value.begin();
    while(lIter++!=value.end()){
        if(!isspace(*lIter)){
            lPreviousNonws = lIter;
        }
        if(*lIter==','){
            std::string::iterator lStartOfPayload = lPreviousComma;
            std::string::iterator lEndOfPayload   = lPreviousNonws;
            while((*lEndOfPayload)==',')
                lEndOfPayload--;

            while(((*lStartOfPayload) == '"') || isspace(*lStartOfPayload))
                lStartOfPayload++;
            
            while(((*lEndOfPayload)=='"') || isspace(*lEndOfPayload)){
                lEndOfPayload--;
            }
            
            assert(lEndOfPayload >= lStartOfPayload);
            assert(lEndOfPayload != value.end());
            
            Entry *lEntry = nullptr;
            std::string lString("");
                
            if((*lStartOfPayload=='[') && (*lStartOfPayload==']')){
            
                lEntry = new Array(lString, string(lStartOfPayload, lEndOfPayload), (Group *)this);
            
            }
            else if((*lStartOfPayload=='{') && (*lStartOfPayload=='}')){
            
                lEntry = new Group(lString, string(lStartOfPayload, lEndOfPayload), (Group *)this);
            
            }
            else{
            
                lEntry = new Value(lString, string(lStartOfPayload, lEndOfPayload), (Group *)this);
                
            }
            
            Contents.push_back(lEntry);
            
            lPreviousComma = lIter;
            
        }
        
    }

}


Group::Group(const string &lName, const string &lFrom, Group *group)
  : Entry(lName, group){
    
}

/////
// Read Functions:
// Adds to the mapping from a data source. The range of bytes from 
//  aRangeStart to aRangeEnd are considered the valid config data,
//  unless both are 0, in which case the entire DataSource is.
//  
        
// Attempt to guess the DataSource format, returning the guess.
Group::ConfigType Group::ReadDataSource(DataSource *aFrom , size_t aRangeStart, size_t aRangeLen){
    ConfigType lConf = AnalyzeDataSource(aFrom, aRangeStart, aRangeLen);
    switch(lConf){
    case iniConfig:
        ReadDataSourceINI(aFrom, aRangeStart, aRangeLen);
    break;
    case jsonConfig:
        ReadDataSourceJSON(aFrom, aRangeStart, aRangeLen);
    break;
    default:
        return nullConfig;
    }
    return lConf;
}

enum lType {GROUP, VALUE_NAME, VALUE_PAYLOAD, ARRAY};

// Read the DataSource as an INI config.
void Group::ReadDataSourceINI(DataSource *aFrom , size_t aRangeStart, size_t aRangeLen){
    if(aRangeLen==0)
        aRangeLen = aFrom->Length() - aRangeStart;
        
    aFrom->Seek(aRangeStart, DataSource::eStart);
    
    string lName;
    string lValue;
    lType lMode = VALUE_NAME;
    int Stepping[] = {0, 0};
    
    std::stack<Group *> lGroups = std::stack<Group *>();
    lGroups.push(this);
    
    Entry *lEntry = nullptr;
    
    for(int i = 0; i<aRangeLen; i++){
        char lChar = aFrom->Get<uint8_t>();
        
        if(lChar=='\n'){
            switch(lMode){
            case lType::GROUP:
            
                while(std::max<int>(Stepping[0], Stepping[1])<lGroups.size())
                    lGroups.pop();
                        
                { // lGroup
                    Group *lGroup = new Group(lName, lGroups.top());
                    lGroups.push(lGroup);
                    Contents.push_back(lGroup);
                } // lGroup
                
                break;
            
            case lType::VALUE_PAYLOAD:
                lEntry = new Value(lName, lValue, lGroups.top());
                lGroups.top()->Contents.push_back(lEntry);
                break;
            case lType::ARRAY:
                lEntry = new Array(lName, lValue, lGroups.top());
                lGroups.top()->Contents.push_back(lEntry);
                break;
            
            case lType::VALUE_NAME: // You're killing me.
            break;
            
            }
            
            lName.clear();
            lValue.clear();
            
            lMode = VALUE_NAME;
            
        }
        
        if(lChar=='['){
            if(!lName.empty())
                lName.clear();
                
            lMode = GROUP;
            Stepping[0]++;
            
            continue;
        }
        if(lChar==']'){
            Stepping[1]++;
            continue;
        }
        if(lChar=='='){
            lMode=VALUE_PAYLOAD;
            continue;
        }
        
        if(lChar=='='){
            lMode=VALUE_PAYLOAD;
            continue;
        }
        
        if(lChar==','){
            lMode=ARRAY;
            //No continue here, the commas are a part of the array payload.
        }
        
        switch(lMode){
            case GROUP:
            case VALUE_NAME:
                lName.push_back(lChar);
                break;
                
            case VALUE_PAYLOAD:
            case ARRAY:
                lValue.push_back(lChar);
                break;
        }
    
    }
    
}
        
// Read the DataSource as a JSON config.
void Group::ReadDataSourceJSON(DataSource *aFrom , size_t aRangeStart, size_t aRangeLen){
    if(aRangeLen==0)
        aRangeLen = aFrom->Length() - aRangeStart;
    
    aFrom->Seek(aRangeStart, DataSource::eStart);

    char lChar = aFrom->Get<uint8_t>();
    while((lChar!='{')&&(lChar!='['))
        lChar = aFrom->Get<uint8_t>();
                
    string lName;
    string lValue;
    enum {pNAME, pVALUE} lPart = pNAME;
    enum {tVALUE, tARRAY, tGROUP} lType = tGROUP;
    enum {NONE, SINGLE, DOUBLE} quotes = NONE;
    bool lNeedsNextElem = false;
    std::stack<Group *> lGroups = std::stack<Group *>();
    lGroups.push(this);
    
    for(int i = 0; i<aRangeLen; i++){
        char lChar = aFrom->Get<uint8_t>();
        
        if(lNeedsNextElem){
            if(lChar==',')
                lNeedsNextElem = false;
        }
        
        if(lNeedsNextElem)
            continue;
        
        if(lChar=='{'){
            lType = tGROUP;
        }
        
        if((lChar==',')||(lChar=='{')){
            switch(lType){
                case tGROUP:
                    lGroups.push(new Group(lName, lGroups.top()));
                    Contents.push_back(lGroups.top());
                    break;
                case tVALUE:
                    if(!lValue.empty())
                        lGroups.top()->Contents.push_back(new Value(lName, lValue, lGroups.top()));
                    break;
                case tARRAY:
                    lGroups.top()->Contents.push_back(new Value(lName, lValue, lGroups.top()));
                    break;
            }
            if(lType!=tARRAY){
                lType = tVALUE;
                lPart = pNAME;
            }
            
            lName.clear();
            lValue.clear();
            
            continue;
            
        }
        
        { // Handle Quote marks.
            if(lChar=='\''){
                if(quotes!=DOUBLE){
                    
                    if(quotes==SINGLE)
                        quotes=NONE;
                    else
                        quotes=SINGLE;
                        
                    continue;
                }
            }
            
            if(lChar=='"'){
                if(quotes!=SINGLE){
                    
                    if(quotes==DOUBLE)
                        quotes=NONE;
                    else
                        quotes=DOUBLE;
                        
                    continue;
                }
            }
        } // Handle Quote marks.
        
        if(quotes!=NONE){
            if(lPart == pNAME)
                lName.push_back(lChar);
            else if(lPart==pVALUE)
                lValue.push_back(lChar);
                
            continue;
        }
        else if(isspace(lChar))
            continue;
        
        if(lChar==':'){
            lPart = pVALUE;
            continue;
        }
        
        if(lChar=='['){
            lGroups.push(new Group(lName, lGroups.top()));
            Contents.push_back(lGroups.top());
            lType = tARRAY;
            continue;
        }
        if((lChar==']')||(lChar=='}')){
            if(lGroups.empty()){ // We've pulled the last object off.
                continue;
            }
            
            lNeedsNextElem = true;
            
            lGroups.pop();
            lType = tVALUE;
            lPart = pNAME;
            continue;
        }
        

    
        if(lPart == pNAME)
            lName.push_back(lChar);
        else if(lPart==pVALUE)
            lValue.push_back(lChar);
    
        
        
    }
    
    //Contents.erase(Contents.end());
    
}
        
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
void Group::WriteINI(DataSource *aTo){

    string lRepresentation = "";
    std::list<Entry *>::iterator lIter = Contents.begin();
    while(lIter!=Contents.end()){
        lRepresentation.append((*lIter)->GetAsINI(1));
        lIter++;
    }
    
    aTo->Write(lRepresentation.c_str(), lRepresentation.length());
    
}

string Group::GetAsINI(unsigned aStep){
    string lRepresentation = "";
    
   lRepresentation.append(aStep, '[');
    lRepresentation.append(Name);
    lRepresentation.append(aStep, ']');
    
    lRepresentation.push_back('\n');
    
    std::list<Entry *>::iterator lIter = Contents.begin();
    while(lIter!=Contents.end()){
    
        lRepresentation.append((*lIter)->GetAsINI(aStep+1));
        lRepresentation.push_back('\n');
        lIter++;
    
    }
    
    return lRepresentation;
}

string Group::GetAsJSON(unsigned aStep){
    string lRepresentation = "";
    aStep++;
    lRepresentation.append(aStep*2, ' ');
    lRepresentation.push_back('"');
    lRepresentation.append(Name);
    lRepresentation.append("\" : {");
    
    lRepresentation.push_back('\n');
    
    std::list<Entry *>::iterator lIter = Contents.begin();
    while(lIter!=Contents.end()){
    
        lRepresentation.append((*lIter)->GetAsJSON(aStep+1));
        lRepresentation.push_back('\n');
        lIter++;
    
    }
    
    lRepresentation.append(aStep*2, ' ');
    lRepresentation.append("},\n");
    return lRepresentation;
}

// Write as JSON
void Group::WriteJSON(DataSource *aTo){
    string lRepresentation = "{";
    
    //lRepresentation.push_back('"');
    //lRepresentation.append(Name);
    //lRepresentation.append("\" : {");
    
    lRepresentation.push_back('\n');
    
    std::list<Entry *>::iterator lIter = Contents.begin();
    while(lIter!=Contents.end()){
    
        lRepresentation.append((*lIter)->GetAsJSON(0));
        lRepresentation.push_back('\n');
        lIter++;
    
    }
    
    lRepresentation.push_back('}');
    
    aTo->Write(lRepresentation.c_str(), lRepresentation.length());
}
        
// Try to guess the DataSource format, and rewrite it. 
size_t Group::Rewrite(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar){

    return 0;
}
        
// Rewrite as INI-style
size_t Group::RewriteINI(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar){

    return 0;
}
        
// Rewrite as JSON
size_t Group::RewriteJSON(DataSource *aTo, size_t aStart, size_t aEnd, string BlankChar){

    return 0;
}
        
/////
// Attempt to identify the format of a DataSource. Same semantics as ReadDataSource.
Group::ConfigType Group::AnalyzeDataSource(DataSource *aFrom , size_t aRangeStart, size_t aRangeLen){
    
    if(aRangeLen==0)
        aRangeLen = aFrom->Length();
    
    aFrom->Seek(aRangeStart, DataSource::eStart);
    
    int INIvotes  = 0;
    int JSONvotes = 0;
    
    size_t read = 0;
    
    // What we are looking for:
    // Does every line have either a '[' and a ']', or a '='? INI.
    // Does every line have either a '{', a '}', or a ':'? JSON.

    char CurChar = '\0';
    
    bool FoundOpenBracket       = false;
    bool FoundCloseBracket      = false;
    bool FoundOpenCurlyBracket  = false;
    bool FoundCloseCurlyBracket = false;
    bool FoundColon             = false;
    bool FoundEquals            = false;
    bool FoundComma             = false;
    
    while(read < aRangeLen){
        aFrom->Read(&CurChar, 1);
        read++;
        
        switch(CurChar){
        case '[':
            FoundOpenBracket       = true;
            break;
        case ']':
            FoundCloseBracket      = true;
            break;
        case '{':
            FoundOpenCurlyBracket  = true;
            break;
        case '}':
            FoundCloseCurlyBracket = true;
            break;
        case ':':
            FoundColon  = true;
            break;
        case '=':
            FoundEquals = true;
            break;
        case ',':
            FoundComma  = true;
            break;
        case '\n':
        
            {
                bool CouldBeINI = (FoundOpenBracket&&FoundCloseBracket&&(!FoundComma))||FoundEquals;
                bool CouldBeJSON= (FoundOpenCurlyBracket||FoundCloseCurlyBracket)||FoundColon;
                
                if(CouldBeINI && (!CouldBeJSON))
                    INIvotes++;
                else if(CouldBeJSON && (!CouldBeINI))
                    JSONvotes++;
            }
            
            FoundOpenBracket       = false;
            FoundCloseBracket      = false;
            FoundOpenCurlyBracket  = false;
            FoundCloseCurlyBracket = false;
            FoundColon             = false;
            FoundEquals            = false;
        
        break;
        }
    
        // If we have hit ten more votes for one or the other, we know enough.
        if(std::abs(INIvotes-JSONvotes)>=10)
            break;
    
    }
    
    if(INIvotes > JSONvotes)
        return Group::ConfigType::iniConfig;
    if(INIvotes < JSONvotes)
        return Group::ConfigType::jsonConfig;
    
    return Group::ConfigType::nullConfig;
}
        
        


}
}