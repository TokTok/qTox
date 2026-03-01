/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <cstdint>

class Friend;

class ICoreCallControl
{
public:
    ICoreCallControl() = default;
    virtual ~ICoreCallControl();
    ICoreCallControl(const ICoreCallControl&) = default;
    ICoreCallControl& operator=(const ICoreCallControl&) = default;
    ICoreCallControl(ICoreCallControl&&) = default;
    ICoreCallControl& operator=(ICoreCallControl&&) = default;

    virtual bool isCallStarted(const Friend* f) const = 0;
    virtual bool isCallActive(const Friend* f) const = 0;
    virtual bool isCallVideoEnabled(const Friend* f) const = 0;
    virtual bool isCallInputMuted(const Friend* f) const = 0;
    virtual bool isCallOutputMuted(const Friend* f) const = 0;

    virtual bool startCall(uint32_t friendNum, bool video) = 0;
    virtual bool answerCall(uint32_t friendNum, bool video) = 0;
    virtual bool cancelCall(uint32_t friendNum) = 0;
    virtual void toggleMuteCallInput(const Friend* f) = 0;
    virtual void toggleMuteCallOutput(const Friend* f) = 0;
};
