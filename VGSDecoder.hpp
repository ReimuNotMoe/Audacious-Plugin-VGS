/*
    This file is part of Audacious-Plugin-VGS.
    Copyright (C) 2016, 2017, 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once

#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <iostream>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>

#include <libaudcore/i18n.h>
#include <libaudcore/plugin.h>
#include <libaudcore/runtime.h>

#define PACKAGE "audacious-plugins"
#define EXPORT __attribute__((visibility("default")))

#include "bgm_decoder/vgsdec.h"
#include "bgm_decoder/vgsmml.h"

#define InputFileType_Bad       0
#define InputFileType_MML       1
#define InputFileType_BGM       2
#define InputFileType_VGS       3


class VGSMMLDec : public InputPlugin
{
public:
        static const char about[];
        static const char *const exts[];

        static constexpr PluginInfo info = {
                N_("VGS BGM/MML Decoder"),
                PACKAGE,
                about
        };

        static constexpr auto iinfo = InputInfo(FlagWritesTag)
                .with_priority(1)
                .with_exts(exts);

        constexpr VGSMMLDec() : InputPlugin(info, iinfo) {}

        bool init() override;
        void cleanup() override;

        bool is_our_file(const char *filename, VFSFile &file) override;
        bool read_tag (const char * filename, VFSFile & file, Tuple & tuple, Index<char> * image) override;
        Tuple read_tuple(const char *filename, VFSFile &file);
        //Index<char> read_image(const char *filename, VFSFile &file);
        //bool write_tuple(const char *filename, VFSFile &file, const Tuple &tuple);
        bool play(const char *filename, VFSFile &file) override;
};
