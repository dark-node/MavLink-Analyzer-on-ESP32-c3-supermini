#pragma once

/**
 * HTTP + WebSocket server (TZ §8): status API, log stream, STOP/START.
 */

namespace web_server {

bool init();
bool start();
void stop();
bool isActive();

/** Call each main loop iteration (WebSocket cleanup). */
void poll();

/** Push one new log line to connected WebSocket clients. */
void onLogLine(const char* line);

/** Notify clients before Exit / deep sleep (TZ §4.4). */
void notifyExitClients();

}  // namespace web_server
