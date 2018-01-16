/*
 * Copyright (c) 2018 Ambroz Bizjak
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AIPSTACK_SIGNAL_BLOCKER_IMPL_LINUX_H
#define AIPSTACK_SIGNAL_BLOCKER_IMPL_LINUX_H

#include <aipstack/misc/NonCopyable.h>
#include <aipstack/platform_impl/SignalCommon.h>

namespace AIpStack {

class SignalBlockerImplLinux :
    private NonCopyable<SignalBlockerImplLinux>
{
public:
    SignalBlockerImplLinux () = default;

    ~SignalBlockerImplLinux () = default;

    void block (SignalType signals);

    void unblock (SignalType blocked_signals);

private:
    SignalType m_orig_blocked_signals;
};

using SignalBlockerImpl = SignalBlockerImplLinux;

#define AIPSTACK_SIGNAL_BLOCKER_IMPL_IMPL_FILE \
    <aipstack/platform_specific/SignalBlockerImplLinux_impl.h>

}

#endif
