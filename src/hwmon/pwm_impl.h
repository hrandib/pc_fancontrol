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

#include "sysfs/writer_impl.h"
#include "interface/pwm.h"

class PwmImpl : SysfsWriterImpl, public Pwm
{
public:
    enum class Mode {
        Dc,
        Pwm,
        NoChange
    };
    enum class Control {
        Manual = 1,
        Auto = 2
    };

private:
    static inline constexpr sv ENABLE_SUFFIX = "_enable";
    static inline constexpr sv MODE_SUFFIX = "_mode";

    fs::path enablePath_, modePath_;
    uint_fast8_t minPwm_, maxPwm_;
    Mode mode_;

    bool setControl(Control control);
    bool setMode(Mode mode);
public:
    PwmImpl(const fs::path& pwmPath, uint_fast8_t min, uint_fast8_t max,
            Mode mode = Mode::NoChange);

    bool open() override;
    bool set(uint_fast8_t val) override;
    ~PwmImpl() override;
};

template<typename T>
inline Pwm::ptr make_pwm(const fs::path&, uint_fast8_t, uint_fast8_t,
                         PwmImpl::Mode mode = PwmImpl::Mode::NoChange);

template<>
inline Pwm::ptr make_pwm<PwmImpl>(const fs::path& pwmPath, uint_fast8_t min, uint_fast8_t max,
                                  PwmImpl::Mode mode) {
    return std::make_unique<PwmImpl>(pwmPath, min, max, mode);
}

#endif // PWMIMPL_H
