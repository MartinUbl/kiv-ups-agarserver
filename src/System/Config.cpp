#include "General.h"
#include "Config.h"
#include "Log.h"

#include <string>

Config::Config()
{
    int i;
    ConfigOption *opt;

    // load defaults to config map
    for (i = 0; i < sizeof(configOptions) / sizeof(ConfigOption); i++)
    {
        opt = &configOptions[i];

        if (opt->vtype == CONF_TYPE_INT)
            SetIntValue((RecognizedConfigOption)i, opt->value.intval);
        else if (opt->vtype == CONF_TYPE_STRING)
            SetStringValue((RecognizedConfigOption)i, opt->value.strval);
    }
}

Config::~Config()
{
    int i;
    ConfigOption *opt;

    // free memory allocated for string-type config values
    for (i = 0; i < sizeof(configOptions) / sizeof(ConfigOption); i++)
    {
        opt = &configOptions[i];

        if (opt->vtype == CONF_TYPE_STRING)
            delete[] opt->value.strval;
    }
}

bool Config::Load(char* configPath)
{
    FILE* cfile = nullptr;
    char* cfgfilename = nullptr;

    if (configPath)
        cfgfilename = configPath;
    else
        cfgfilename = CONFIG_PATH MAIN_CONFIG;

    cfile = fopen(cfgfilename, "r");

    sLog->Info("Loading main config file %s", cfgfilename);

    if (!cfile)
    {
        sLog->Error("CONFIG: Could not load main config file!");
        return false;
    }

    char buf[128];
    memset(buf, 0, sizeof(char) * 128);

    while (fgets(buf, 128, cfile))
        ParseConfigOption(buf);

    sLog->Info("Configuration file loaded successfully!\n");

    return true;
}

void Config::ParseConfigOption(char* line)
{
    int p1, p2, i;

    p1 = 0;

    while (line[p1] != '\0' && line[p1] != '=')
        p1++;

    p2 = p1;

    while (line[p2] != '\0')
        p2++;

    // invalid syntax
    if (p2 <= p1 + 1)
    {
        sLog->Error("CONFIG: invalid syntax (no right-hand value): %s", line);
        return;
    }

    if (line[p2 - 1] == '\n')
        line[p2 - 1] = '\0';

    for (i = 0; i < sizeof(configOptions) / sizeof(ConfigOption); i++)
    {
        if (!strncmp(configOptions[i].name.c_str(), line, p1))
        {
            if (configOptions[i].vtype == CONF_TYPE_STRING)
                SetStringValue((RecognizedConfigOption)i, std::string(line+p1+1, p2));
            else if (configOptions[i].vtype == CONF_TYPE_INT)
                SetIntValue((RecognizedConfigOption)i, std::stoi(std::string(line + p1 + 1, p2)));
        }
    }
}

std::string Config::GetStringValue(RecognizedConfigOption opt)
{
    return m_configOptions[opt].strval;
}

int Config::GetIntValue(RecognizedConfigOption opt)
{
    return m_configOptions[opt].intval;
}

void Config::SetStringValue(RecognizedConfigOption opt, std::string val)
{
    char *stored;

    stored = new char[val.size() + 1];
    val.copy(stored, val.size());
    stored[val.size()] = '\0';

    // clear any previously stored string values
    if (m_configOptions.find(opt) != m_configOptions.end())
        delete[] m_configOptions[opt].strval;

    m_configOptions[opt] = stored;
}

void Config::SetIntValue(RecognizedConfigOption opt, int val)
{
    m_configOptions[opt] = val;
}
