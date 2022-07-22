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
#ifndef PWMIMPL_H
#define PWMIMPL_H

#include "interface/pwm.h"
#include "sysfs/writer_impl.h"
#include <map>

class PwmImpl : SysfsWriterImpl, public Pwm
{
public:
    enum class Control { Manual = 1, Auto = 2 };
private:
    using string = std::string;
    using sv = std::string_view;

    static inline constexpr sv ENABLE_SUFFIX = "_enable";
    static inline constexpr sv MODE_SUFFIX = "_mode";

    std::mutex mutex_;
    std::map<string, double> valueCache_;
    fs::path enablePath_, modePath_;
    int minPwm_, maxPwm_;
    Mode mode_;
    uint32_t previousRawValue_;
    bool isStopped_;

    bool setControl(Control control);
    bool activateMode(Mode mode);

    double selectMaxValue(double val, const string& sourceName);
    uint32_t processFanStopCondition(int tempOffset);
public:
    PwmImpl(const fs::path& pwmPath, int min, int max, Mode mode = Mode::NoChange);
    PwmImpl(const fs::path& pwmPath);

    bool open() override;
    bool set(double val, int tempOffset, const string& sourceName) override;
    void reset() override;

    void setMin(int val) override
    {
        minPwm_ = val;
    }

    void setMax(int val) override
    {
        maxPwm_ = val;
    }

    void setMode(Mode mode) override
    {
        mode_ = mode;
    }

    bool exists() override
    {
        return fs::exists(getFilePath());
    }

    ~PwmImpl() override;

    DISABLE_OBJ_COPY(PwmImpl)
};

template<typename T>
inline Pwm::ptr make_pwm(const fs::path&);

template<>
inline Pwm::ptr make_pwm<PwmImpl>(const fs::path& pwmPath)
{
    return std::make_shared<PwmImpl>(pwmPath);
}

#endif // PWMIMPL_H
