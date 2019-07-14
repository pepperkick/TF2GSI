#include "socketserver.h"

#include "common.h"

#include <thread>
#include <chrono>

#include <libwebsockets.h>

void CALLBACK SocketServerLoopTimer(HWND, UINT, UINT, DWORD);
int SocketServer::m_Port = 5540;
int SocketServer::m_TickRate = 125;
int SocketServer::m_MessageLength = 1;
bool SocketServer::m_IsRunning = false;
lws_context* SocketServer::m_Context = nullptr;
char* SocketServer::m_Message[MAX_MESSAGE_LENGTH];

bool SocketServer::Start() {
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);
	info.port = m_Port;
	info.protocols = m_Protocols;

	m_Context = lws_create_context(&info);

	if (!m_Context) {
		LogError("Faild to start websocket server\n");

		return false;
	}

	m_IsRunning = true;

	LogSuccess("Websocket Server Started\n");

	while (m_IsRunning) {
		lws_service(m_Context, m_TickRate);
	}

	SocketServer::Stop();

	return true;
}

bool SocketServer::Stop() {
	m_IsRunning = false;

	lws_context_destroy(m_Context);

	LogInfo("Websocket Server Stopped\n");

	return true;
}

void SocketServer::SetMessage(const char* msg) {
	m_MessageLength = std::strlen(msg);

	if (m_MessageLength) {
		strncpy(((char*)m_Message), msg, MAX_MESSAGE_LENGTH);
	}
}

int SocketServer::CallbackGameData(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len) {
	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		break;
	case LWS_CALLBACK_ESTABLISHED:
		LogInfo("Websocket Client Connected\n");

		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_CLOSED:
		LogInfo("Websocket Client Disconnected\n");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (m_MessageLength) {
			unsigned char buf[LWS_PRE + MAX_MESSAGE_LENGTH];
			memset(&buf[LWS_PRE], 0, MAX_MESSAGE_LENGTH);
			strncpy((char*) buf + LWS_PRE, (const char*) m_Message, MAX_MESSAGE_LENGTH);

			int wroteLen = lws_write(wsi, &buf[LWS_PRE], m_MessageLength, LWS_WRITE_TEXT);

			if (wroteLen < m_MessageLength) {
				LogError("Error while writing to websocket, wrote length %d, actual length %d\n", wroteLen, m_MessageLength);
			}

			LogDebug("Wrote in websocket\n");
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(m_TickRate));

		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
		break;
	}

	return 0;
}