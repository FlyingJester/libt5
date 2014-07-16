#include "t5.h"

using t5::map::Group;

#include <string>

const std::string LongStart= "--";
const std::string HelpOpt  = "-h";
const std::string HelpLong = LongStart + std::string("help");
const std::string iniExt   = "ini";
const std::string jsonExt  = "json";
const std::string iniOpt   = LongStart + iniExt;
const std::string jsonOpt  = LongStart + jsonExt;

// Shown here as  replacement for stdout:
#include <cstdio>
#include <cstdlib>

t5::DataSource *OutSource = t5::DataSource::FromCFileHandle(stdout);
t5::DataSourcePusher *StdOut = OutSource->AsPusher();

inline int BadUsage(){
    StdOut->WriteS("\tUsage: -i [--ini|--json] [file] [-o [--ini|--json|--test] [output]]\n");
    return EXIT_SUCCESS;
}

inline int PrintHelp(){
    BadUsage();
    return EXIT_SUCCESS;
}

int rmain(int argc, const char * argv[]){

    //StdOut->WriteS("T5 Convert - Convert between JSON and INI files.\n");
    
    for(int i = 0; i<argc; i++){
        std::string opt = argv[i];
        if((opt==HelpOpt)||(opt==HelpLong))
            return PrintHelp();
    }
    if(argc<=1){
        BadUsage();
        
        const char *argv1[] = {
            "t5convert",
            "-i",
            "--json",
            "/Users/mozilla/Desktop/apache_builds.json",
            "-o",
            "--json",
            "/Users/mozilla/Desktop/apache_builds.t5.json"
        };
        
        argc = 7;
        
        argv = argv1;
    }
    std::string InputFile = "";
    std::string OutputFile = "";
    Group::ConfigType InType = Group::nullConfig;
    Group::ConfigType OutType = Group::nullConfig;
    
    enum {BAD, INPUT, OUTPUT} lMode = BAD;
    
    for(int i = 0; i<argc; i++){
        std::string opt = argv[i];
        
        if(opt=="-i"){
            lMode = INPUT;
            continue;
        }
        if(opt=="-o"){
            lMode = OUTPUT;
            continue;
        }
        
        if(opt==iniOpt){
            if(lMode==INPUT)
                InType = Group::iniConfig;
            if(lMode==OUTPUT)
                OutType = Group::iniConfig;
            
            continue;
        }
        
        if(opt==jsonOpt){
            if(lMode==INPUT)
                InType = Group::jsonConfig;
            if(lMode==OUTPUT)
                OutType = Group::jsonConfig;
            
            continue;
        }
        
        if((lMode==INPUT)&&(InputFile.empty())){
            InputFile = opt;
            continue;
        }
        if((lMode==OUTPUT)&&(OutputFile.empty())){
            OutputFile = opt;
            if(OutType!=Group::nullConfig)
                continue;
            
            if(OutputFile.rfind(".ini")==InputFile.length()-4)
                OutType = Group::jsonConfig;
            if(OutputFile.rfind(".json")==InputFile.length()-5)
                OutType = Group::jsonConfig;
            continue;
        }
        
        //return BadUsage();
        
    }
    
    t5::map::Group *rGroup = new t5::map::Group(InputFile, nullptr);
    t5::DataSource *ConfSource = t5::DataSource::FromPath(t5::DataSource::eRead, InputFile.c_str());
    t5::DataSource *ConfDestin = t5::DataSource::FromPath(t5::DataSource::eWrite, OutputFile.c_str());
    
    if(OutType==Group::nullConfig)
        OutType = Group::iniConfig;
    
    switch (InType){
        case Group::nullConfig:
        //StdOut->WriteS("Guessing config.\n");
            rGroup->ReadDataSource(ConfSource);
        break;
        case Group::iniConfig:
        //StdOut->WriteS("Reading INI.\n");
            rGroup->ReadDataSourceINI(ConfSource);
        break;
        case Group::jsonConfig:
        //StdOut->WriteS("Reading JSON.\n");
            rGroup->ReadDataSourceJSON(ConfSource);
        break;
    }
    
    
    //StdOut->WriteS((std::string("Writing to file ") + OutputFile + std::string("\n")).c_str());
    
    
    
    delete rGroup;
    delete ConfSource;
    delete ConfDestin;
    
    return 0;
    
    
    
    switch(OutType){
        case Group::iniConfig:
            StdOut->WriteS("Writing INI.\n");
            rGroup->WriteINI(ConfDestin);
        break;
        case Group::jsonConfig:
            StdOut->WriteS("Writing JSON.\n");
            rGroup->WriteJSON(ConfDestin);
        break;
        case Group::nullConfig:
        break;
    }
    
    
    //        rGroup->WriteINI(StdOut);
    //StdOut->Put('\n');

    //        rGroup->WriteJSON(StdOut);
            
    //StdOut->Put('\n');
    
    //StdOut->WriteS("Landed.\n");
    
    delete rGroup;
    delete ConfSource;
    delete ConfDestin;
    
    return 0;
    
}

int main(int argc, const char * argv[]){
    for(int i = 0; i<100; i++){
        rmain(argc, argv);
    }
}
