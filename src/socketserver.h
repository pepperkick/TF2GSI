#include <stdio.h>
#include <stdlib.h>
#include <libwebsockets.h>

#define MAX_MESSAGE_LENGTH 65536

class SocketServer {
public:
	static bool Start();
	static bool Stop();
	static void Loop();
	static void SetMessage(const char*);
	static int CallbackGameData(struct lws*, enum lws_callback_reasons, void*, void*, size_t);

private:
	constexpr static struct lws_protocols m_Protocols[] = {
		{ "game-data", CallbackGameData, 0 },
		{ NULL, NULL, 0 }
	};

	static int m_Port;
	static int m_TickRate;
	static int m_MessageLength;
	static bool m_IsRunning;
	static HWND m_Timer;
	static lws_context* m_Context;
	static char* m_Message[MAX_MESSAGE_LENGTH];
};