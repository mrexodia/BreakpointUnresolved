#include "plugin.h"

static std::unordered_map<std::string, std::vector<std::string>> buMap;

PLUG_EXPORT void CBINITDEBUG(CBTYPE, PLUG_CB_INITDEBUG* info)
{
    buMap.clear();
}

PLUG_EXPORT void CBLOADDLL(CBTYPE, PLUG_CB_LOADDLL* info)
{
    char modname[MAX_MODULE_SIZE];
    if(!Script::Module::NameFromAddr(duint(info->LoadDll->lpBaseOfDll), modname))
        return;
    char* extensionPos = strrchr(modname, '.');
    if(extensionPos)
        *extensionPos = '\0';
    auto found = buMap.find(modname);
    if(found == buMap.end())
    {
        *extensionPos = '.';
        found = buMap.find(modname);
    }
    if(found == buMap.end())
        return;
    char command[MAX_MODULE_SIZE * 2];
    for(auto & api : found->second)
    {
        sprintf_s(command, "bp \"%s\":%s", found->first.c_str(), api.c_str());
        DbgCmdExec(command);
    }
    buMap.erase(found);
}

//WinDbg: https://goo.gl/gxYAGU
static bool cbCommandBu(int argc, char* argv[])
{
    if(argc < 3)
    {
        _plugin_logputs("[" PLUGIN_NAME "] Usage: bu module, api");
        return false;
    }
    if(DbgModBaseFromName(argv[1]))
    {
        char command[MAX_MODULE_SIZE * 2];
        sprintf_s(command, "bp \"%s\":%s", argv[1], argv[2]);
        _plugin_logprintf("[" PLUGIN_NAME "] Module already loaded, executing command \"%s\"\n", command);
        return DbgCmdExecDirect(command);
    }
    buMap[argv[1]].push_back(argv[2]);
    _plugin_logprintf("[" PLUGIN_NAME "] Added unresolved breakpoint \"%s\":%s\n", argv[1], argv[2]);
    return true;
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    _plugin_registercommand(pluginHandle, "bu", cbCommandBu, true);
    return true;
}

//Deinitialize your plugin data here (clearing menus optional).
bool pluginStop()
{
    return true;
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}
