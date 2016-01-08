#ifndef AGAR_CONFIG_H
#define AGAR_CONFIG_H

#include "Singleton.h"

#include <map>

/* Default name of main configuration file */
#define MAIN_CONFIG "server.cfg"

/* All recongnized config options aliases */
enum RecognizedConfigOption
{
    CONF_BIND_IP = 0,
    CONF_PORT = 1,
    CONF_DEBUG_LOG = 2,
    CONF_LOG_FILE = 3,

    CONF_MAX
};

/* All recognized config value types */
enum ConfigValueType
{
    CONF_TYPE_STRING = 0,
    CONF_TYPE_INT = 1,
    CONF_TYPE_MAX
};

/* Union of config option values */
union ValueUnion
{
    const char* strval;
    int intval;

    ValueUnion() { };
    ValueUnion(const char* c) : strval(c) { };
    ValueUnion(int a) : intval(a) { };
};

/* Config option structure */
struct ConfigOption
{
    std::string name;
    ConfigValueType vtype;
    ValueUnion value;
};

/* Config options with types and default values */
static ConfigOption configOptions[] = {
    { "BIND_IP",    CONF_TYPE_STRING,       "0.0.0.0" } /* CONF_BIND_IP */,
    { "PORT",       CONF_TYPE_INT,          8969      } /* CONF_PORT */,
    { "DEBUG_LOG",  CONF_TYPE_INT,          0         } /* CONF_DEBUG_LOG */,
    { "LOG_FILE",   CONF_TYPE_STRING,       "server.log" } /* CONF_LOG_FILE */
};

class Config
{
    friend class Singleton<Config>;
    public:
        ~Config();

        /* Load config file from specified or default location */
        bool Load(char* configPath = nullptr);
        /* Gets string value from config */
        std::string GetStringValue(RecognizedConfigOption opt);
        /* Gets numeric value from config */
        int GetIntValue(RecognizedConfigOption opt);

    protected:
        /* Hidden singleton constructor */
        Config();

        /* Parse one line of config file */
        void ParseConfigOption(char* line);

        /* Sets string value internally */
        void SetStringValue(RecognizedConfigOption opt, std::string val);
        /* Sets numeric value internally */
        void SetIntValue(RecognizedConfigOption opt, int val);

    private:
        /* All config options loaded from file/set from defaults */
        std::map<RecognizedConfigOption, ValueUnion> m_configOptions;
};

#define sConfig Singleton<Config>::getInstance()

#endif
