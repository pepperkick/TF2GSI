/*
MIT License

Copyright (c) 2018 Benjamin H�glinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "Windows.h"
#include <detours/detours.h>
#include "common.h"

#ifndef _MSC_VER
#define noexcept noexcept
#else
#define noexcept
#endif

enum class CallConvention
{
    stdcall_t,
    cdecl_t
};

template <CallConvention cc, typename retn, typename convention, typename ...args>
struct convention;

template <typename retn, typename ...args>
struct convention<CallConvention::stdcall_t, retn, args...>
{
    typedef retn (__stdcall *type)(args ...);
};

template <typename retn, typename ...args>
struct convention<CallConvention::cdecl_t, retn, args...>
{
    typedef retn (__cdecl *type)(args ...);
};

template <CallConvention cc, typename retn, typename ...args>
class Hook
{
    typedef typename convention<cc, retn, args...>::type type;

    size_t orig_;
    type detour_;

    bool is_applied_;
    bool has_open_transaction_;

    void transaction_begin()
    {
        const auto result = DetourTransactionBegin();

        if (result != NO_ERROR)
        {
            if (result == ERROR_INVALID_OPERATION)
            {
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "A pending transaction already exists\n");
				return;
            }

			PRINT_TAG();
			ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
			return;
        }

        has_open_transaction_ = true;
    }

    void transaction_commit()
    {
        const auto result = DetourTransactionCommit();

        if (result != NO_ERROR)
        {
            switch (result)
            {
            case ERROR_INVALID_DATA:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Target function was changed by third party between steps of the transaction\n");
				return;

            case ERROR_INVALID_OPERATION:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "No pending transaction exists\n");
				return;

            case ERROR_INVALID_BLOCK:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The function referenced is too small to be detoured\n");
				return;

            case ERROR_INVALID_HANDLE:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The ppPointer parameter is null or points to a null pointer\n");
				return;

            case ERROR_NOT_ENOUGH_MEMORY:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Not enough memory exists to complete the operation\n");
				return;

            default:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
				return;
            }
        }

        has_open_transaction_ = false;
    }

    static void update_thread(HANDLE hThread)
    {
        const auto result = DetourUpdateThread(hThread);

        if (result != NO_ERROR)
        {
            if (result == ERROR_NOT_ENOUGH_MEMORY)
            {
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Not enough memory to record identity of thread\n");
				return;
            }

			PRINT_TAG();
			ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
			return;
        }
    }

public:
    Hook() : orig_(0), detour_(0), is_applied_(false), has_open_transaction_(false)
    {
    }

    ~Hook()
    {
        if (has_open_transaction_)
        {
            const auto result = DetourTransactionAbort();

            if (result != NO_ERROR)
            {
                if (result == ERROR_INVALID_OPERATION)
                {
					PRINT_TAG();
					ConColorMsg(Color(255, 0, 0, 255), "No pending transaction exists\n");
					return;
                }
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
				return;
            }
        }

        remove();
    }

    template <typename T>
    void apply(T pFunc, type detour)
    {
        detour_ = detour;
        orig_ = static_cast<size_t>(pFunc);

        transaction_begin();
        update_thread(GetCurrentThread());
        const auto result = DetourAttach(reinterpret_cast<void **>(&orig_), reinterpret_cast<void *>(detour_));

        if (result != NO_ERROR)
        {
            switch (result)
            {
            case ERROR_INVALID_BLOCK:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The function referenced is too small to be detoured\n");
				return;

            case ERROR_INVALID_HANDLE:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The ppPointer parameter is null or points to a null pointer\n");
				return;
				
            case ERROR_INVALID_OPERATION:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "No pending transaction exists\n");
				return;

            case ERROR_NOT_ENOUGH_MEMORY:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Not enough memory exists to complete the operation\n");
				return;

            default:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
				return;
            }
        }

        transaction_commit();

        is_applied_ = true;
    }

    void remove()
    {
        if (!is_applied_)
            return;

        is_applied_ = false;

        transaction_begin();
        update_thread(GetCurrentThread());
        const auto result = DetourDetach(reinterpret_cast<void **>(&orig_), reinterpret_cast<void *>(detour_));

        if (result != NO_ERROR)
        {
            switch (result)
            {
            case ERROR_INVALID_BLOCK:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The function referenced is too small to be detoured\n");
				return;

            case ERROR_INVALID_HANDLE:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "The ppPointer parameter is null or points to a null pointer\n");
				return;

            case ERROR_INVALID_OPERATION:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "No pending transaction exists\n");
				return;

            case ERROR_NOT_ENOUGH_MEMORY:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Not enough memory exists to complete the operation\n");
				return;

            default:
				PRINT_TAG();
				ConColorMsg(Color(255, 0, 0, 255), "Unknown error\n");
				return;
            }
        }

        transaction_commit();
    }

    retn call_orig(args ... p)
    {
        return type(orig_)(p...);
    }

    bool is_applied() const
    {
        return is_applied_;
    }
};
