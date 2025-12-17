//
// Created by screemer on 23.12.18.
//
#pragma once

#include "ini_file.h"

//******************
// Config
//******************
class Config {
  public:
    Inifile inifile;
    Config();
    void save();
};
