#include <stdlib.h>
#include <stdio.h>
#include <wiiuse/wpad.h>

#include <winyl/winyl.h>
#include <winyl/request.h>
#include <winyl/header.h>
#include <winyl/version.h>

#include "http.h"
#include "gui.h"
#include "util.h"

char winagent[32];

winyl_response get_http(char *url, int port, char *path) {
	winyl host = winyl_open(url, port);
	winyl_change_http(&host, WINYL_HTTP_1_0);

	winyl_add_header(&host, "User-Agent", winagent);
	if (host.error != 0) {
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			draw_body("Failed to create winyl host.");
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	winyl_response res = winyl_request(&host, path, 0);
	if (res.error != 0) {
		char wtf[64] = "";
		sprintf(wtf, "Unknown libwinyl error %d. CONTACT DEV!", res.error);

		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			switch (res.error) {
				case WINYL_ERROR_PORT:
					draw_body("Invalid port (not 0-65535)");
					break;
				case WINYL_ERROR_DNS:
					draw_body("Error calling net_gethostbyname()");
					break;
				case WINYL_ERROR_MALLOC:
					draw_body("Failed to allocate memory");
					break;
				default:
					draw_body(wtf);
					break;
			}
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	if (res.status != 200) {
		char err_text[256] = "";
		sprintf(err_text, "HTTP %d on %s:%d%s", res.status, url, port, path);
		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			draw_body(err_text);
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	winyl_close(&host);

	return res;
}

