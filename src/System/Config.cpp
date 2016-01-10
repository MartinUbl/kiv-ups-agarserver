#include "General.h"
#include "Config.h"
#include "Log.h"
#include "Helpers.h"

#include <string>

Config::Config()
{
    int i;
    ConfigOption *opt;

    // load defaults to config map
    for (i = 0; i < (int)(sizeof(configOptions) / sizeof(ConfigOption)); i++)
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
    for (i = 0; i < (int)(sizeof(configOptions) / sizeof(ConfigOption)); i++)
    {
        opt = &configOptions[i];

        if (opt->vtype == CONF_TYPE_STRING)
            delete[] opt->value.strval;
    }
}

bool Config::Load(char* configPath)
{
    FILE* cfile = nullptr;
    std::string cfgfilename;

    if (configPath)
        cfgfilename = configPath;
    else
        cfgfilename = CONFIG_PATH MAIN_CONFIG;

    cfile = fopen(cfgfilename.c_str(), "r");

    sLog->Info("Loading main config file %s", cfgfilename.c_str());

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
    std::string tmp;

    p1 = 0;

    while (line[p1] != '\0' && line[p1] != '=')
        p1++;

    p2 = p1;

    while (line[p2] != '\r' && line[p2] != '\n' && line[p2] != '\0')
        p2++;

    if (line[p2] == '\r' || line[p2] == '\n')
        p2++;

    // invalid syntax
    if (p2 <= p1 + 1)
    {
        sLog->Error("CONFIG: invalid syntax (no right-hand value): %s", line);
        return;
    }

    if (line[p2 - 1] == '\n' || line[p2 - 1] == '\r')
        line[p2 - 1] = '\0';
    if (line[p2 - 2] == '\n' || line[p2 - 2] == '\r')
        line[p2 - 2] = '\0';

    for (i = 0; i < (int)(sizeof(configOptions) / sizeof(ConfigOption)); i++)
    {
        if (!strncmp(configOptions[i].name.c_str(), line, p1))
        {
            try
            {
                tmp = std::string(line + p1 + 1, p2);

                if (configOptions[i].vtype == CONF_TYPE_STRING)
                    SetStringValue((RecognizedConfigOption)i, tmp);
                else if (configOptions[i].vtype == CONF_TYPE_INT)
                {
                    if (IsValidInteger(tmp.c_str()))
                        SetIntValue((RecognizedConfigOption)i, std::stoi(tmp));
                    else
                        throw new std::invalid_argument("");
                }
            }
            catch (...)
            {
                tmp = (configOptions[i].vtype == CONF_TYPE_STRING) ? configOptions[i].value.strval : std::to_string(configOptions[i].value.intval);
                sLog->Error("Invalid argument supplied for option %s, using default (%s)", configOptions[i].name.c_str(), tmp.c_str());
            }
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
