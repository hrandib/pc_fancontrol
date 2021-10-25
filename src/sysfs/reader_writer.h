/*
 * Copyright (c) 2020 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SYSFS_READER_H
#define SYSFS_READER_H

#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <string>

namespace fs = std::filesystem;

class SysfsReader
{
private:
    fs::path filePath_;
public:
    using string = std::string;
    using sv = std::string_view;
    using ptr = std::unique_ptr<SysfsReader>;

    SysfsReader(const fs::path& filePath) : filePath_{filePath}
    { }

    const fs::path& getFilePath()
    {
        return filePath_;
    }

    void concatWithFilePath(sv suffix)
    {
        filePath_ += suffix;
    }

    virtual bool open() = 0;
    virtual string read() = 0;

    int32_t readNumber()
    {
        return std::atoi(read().c_str());
    }

    virtual explicit operator bool() = 0;

    virtual ~SysfsReader();
};

class SysfsWriter
{
private:
    fs::path filePath_;
public:
    using string = std::string;
    using sv = std::string_view;
    using ptr = std::unique_ptr<SysfsWriter>;

    SysfsWriter(const fs::path& filePath) : filePath_{filePath}
    { }

    const fs::path& getFilePath()
    {
        return filePath_;
    }

    void concatToFilePath(sv suffix)
    {
        filePath_ += suffix;
    }

    virtual bool open() = 0;
    virtual bool write(uint32_t val) = 0;

    virtual ~SysfsWriter();
};

#endif // SYSFS_READER_H
