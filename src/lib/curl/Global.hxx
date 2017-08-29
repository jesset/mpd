/*
 * Copyright (C) 2008-2016 Max Kellermann <max.kellermann@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CURL_GLOBAL_HXX
#define CURL_GLOBAL_HXX

#include "Multi.hxx"
#include "event/TimerEvent.hxx"
#include "event/DeferredMonitor.hxx"

class CurlSocket;
class CurlRequest;

/**
 * Manager for the global CURLM object.
 */
class CurlGlobal final : DeferredMonitor {
	CurlMulti multi;

	TimerEvent timeout_event;

public:
	explicit CurlGlobal(EventLoop &_loop);

	using DeferredMonitor::GetEventLoop;

	void Add(CURL *easy, CurlRequest &request);
	void Remove(CURL *easy);

	/**
	 * Check for finished HTTP responses.
	 *
	 * Runs in the I/O thread.  The caller must not hold locks.
	 */
	void ReadInfo();

	void Assign(curl_socket_t fd, CurlSocket &cs) {
		curl_multi_assign(multi.Get(), fd, &cs);
	}

	void SocketAction(curl_socket_t fd, int ev_bitmask);

	void InvalidateSockets() {
		SocketAction(CURL_SOCKET_TIMEOUT, 0);
	}

	/**
	 * This is a kludge to allow pausing/resuming a stream with
	 * libcurl < 7.32.0.  Read the curl_easy_pause manpage for
	 * more information.
	 */
	void ResumeSockets() {
		int running_handles;
		curl_multi_socket_all(multi.Get(), &running_handles);
	}

private:
	void UpdateTimeout(long timeout_ms);
	static int TimerFunction(CURLM *global, long timeout_ms, void *userp);

	/* callback for #timeout_event */
	void OnTimeout();

	/* virtual methods from class DeferredMonitor */
	void RunDeferred() override;
};

#endif
