/* NetHack 3.6	uwpoption.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

namespace Nethack {

    bool Options::Load(std::string & filePath)
    {
        bool loaded = false;

        m_filePath = filePath;

        std::string optionsEqual = "OPTIONS=";

        m_options.clear();

        std::fstream input(m_filePath.c_str(), std::fstream::in);
        if (input.is_open())
        {
            std::string line;

            while (std::getline(input, line))
            {
                if (line.compare(0, optionsEqual.size(), optionsEqual.c_str()) != 0)
                    continue;

                std::string option = line.substr(optionsEqual.size(), std::string::npos);

                if (option.length() == 0)
                    continue;

                m_options.push_back(option);
            }

            input.close();
            loaded = true;
        }

        return loaded;
    }

    void Options::Store()
    {
        std::fstream output(m_filePath.c_str(), std::fstream::out | std::fstream::trunc);

        if (output.is_open())
        {
            for (auto & o : m_options)
            {
                output << "OPTIONS=";
                output << o;
                output << "\n";
            }

            output.close();
        }
    }

    std::string Options::GetString(void)
    {
        std::string options;

        for (auto & o : m_options)
        {
            if (options.length() > 0)
                options += ",";

            options += o;
        }

        return options;
    }

    void Options::RemoveOption(std::string & remove)
    {
        auto & iter = m_options.begin();
        while (iter != m_options.end()) {
            auto & option = *iter;
            if (option.compare(0, remove.length(), remove) == 0)
                iter = m_options.erase(iter);
            else
                iter++;
        }
    }

}