#include "ribs.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <netdb.h>

struct http_client_pool client_pool = {
    .timeout_handler.timeout = 1000, /* 1 sec */
    .timeout_handler_persistent.timeout = 1000*60*5 /* 5 minutes */
};

SSTR(URL_SCHEME_HTTP, "http://");
SSTR(URL_SCHEME_HTTPS, "https://");

SSTR(URI_PING, "/ping");
SSTR(URI_UP, "/up");
SSTR(URI_DOWN, "/down");
SSTR(URI_STATUS, "/status");

static struct vmbuf service_info_vect = VMBUF_INITIALIZER;
int global_status = 1; /* UP by default */

void *auto_up_worker_ptr = NULL;

#define HTTP_DEFAULT_PORT 80
#define HTTPS_DEFAULT_PORT 443

struct service_info {
    /* config */
    const char *scheme;
    const char *host;
    int port;
    const char *uri;
    struct in_addr addr;
    int unhealthy_count;
    int healthy_count;
    time_t interval;

    /* runtime */
    int status;
    int counter;
};

void parse_url(const char *url, const char *scheme, struct service_info *service_info, int default_port) {
    // printf("url: %s\n", url);
    service_info->scheme = scheme;
    service_info->host = url;
    char *p = strchrnul(url, '/');
    service_info->uri = strdup(*p ? p : "/");
    *p = 0;
    p = strchrnul(url, ':');
    service_info->port = *p ? atoi(p+1) : default_port;
    *p = 0;
}

const char *parse_params(const char *url, struct service_info *service_info) {
    const char *p = strchrnul(url, '|');
    if (0 == *p)
        return url;
    if (p == url)
        return url + 1;

    char st[p - url + 1], *tokens = st;
    memcpy(st, url, p - url);
    st[p-url] = 0;
    const char *delim = ":";
    int val;
    while (1) {
        val = atoi(strsep(&tokens, delim)); if (val) service_info->healthy_count = val;
        if (!tokens) break;
        val = atoi(strsep(&tokens, delim)); if (val) service_info->unhealthy_count = val;
        if (!tokens) break;
        val = atoi(strsep(&tokens, delim)); if (val) service_info->interval = (time_t)val * 1000 * 1000;
        break;
    }
    return p + 1;
}

static inline const char *status_to_string(int status) {
    return status ? "UP" : "DOWN";
}

void monitor_service_worker(void) {
    struct service_info *service_info = *(struct service_info **)timer_worker_get_user_data();
    // LOGGER_INFO("scheme: %s, host: %s, port: %d, uri: %s, addr: %s", service_info->scheme, service_info->host, service_info->port, service_info->uri, inet_ntoa(service_info->addr));
    struct http_client_context *cctx = http_client_pool_get_requestf(&client_pool, service_info->addr, service_info->port, service_info->host, NULL, "%s", service_info->uri);
    int fail = 1;
    if (NULL == cctx) {
        // LOGGER_ERROR("failed: %s%s:%d%s", service_info->scheme, service_info->host, service_info->port, service_info->uri);
    } else {
        yield();
        // LOGGER_INFO("status code: %d  [%s]", cctx->http_status_code, service_info->host);
        if (200 == cctx->http_status_code)
            fail = 0;
        http_client_free(cctx);
    }

    void update_service_status(struct service_info *service_info, int target_status, const char *transition_label, int threshold) {
        if (service_info->status != target_status) {
            ++service_info->counter;
            if (service_info->counter == threshold) {
                LOGGER_INFO("******* %s:%d%s addr: %s is %s", service_info->host, service_info->port, service_info->uri, inet_ntoa(service_info->addr), transition_label);
                service_info->status = target_status;
                service_info->counter = 0;
            }
        } else {
            service_info->counter = 0;
        }
    }
    update_service_status(service_info, !fail, status_to_string(!fail), fail ? service_info->unhealthy_count : service_info->healthy_count);
    timer_worker_schedule_next(service_info->interval);
}

void auto_up_worker(void) {
    global_status = 1;
}

int count_service_status(int status) {
    int count = 0;
    size_t num_services = vmbuf_num_elements(&service_info_vect, sizeof(struct service_info));
    struct service_info *service_info = vmbuf_mem(&service_info_vect), *service_info_vect_end = service_info + num_services;
    for (; service_info != service_info_vect_end; ++service_info) {
        if (service_info->status == status)
            ++count;
    }
    return count;
}

void dump_service_info(struct vmbuf *outbuf) {
    vmbuf_sprintf(outbuf, "Global status: %s\n\n", status_to_string(global_status));
    size_t num_services = vmbuf_num_elements(&service_info_vect, sizeof(struct service_info));
    struct service_info *service_info = vmbuf_mem(&service_info_vect), *service_info_vect_end = service_info + num_services;
    int up_count = 0, down_count = 0;
    for (; service_info != service_info_vect_end; ++service_info) {
        const char *full_host_url = ribs_malloc_sprintf("%s%s:%d%s", service_info->scheme, service_info->host, service_info->port, service_info->uri);
        vmbuf_sprintf(outbuf, "%-50s  %-20s  %s\n", full_host_url, inet_ntoa(service_info->addr), status_to_string(service_info->status));
        if (service_info->status)
            ++up_count;
        else
            ++down_count;
    }
    vmbuf_sprintf(outbuf, "\nup count: %d/%d (%.2f%%)\n", up_count, up_count + down_count, 100.0 * up_count / (up_count + down_count));
    vmbuf_sprintf(outbuf, "down count: %d/%d (%.2f%%)\n", down_count, up_count + down_count, 100.0 * down_count / (up_count + down_count));
    vmbuf_sprintf(outbuf, "ping status: %s\n\n", status_to_string(global_status && 0 == down_count));
}


void init_service_monitors(struct vmbuf *urls_buf) {
    char *urls = vmbuf_data(urls_buf);
    vmbuf_init(&service_info_vect, 0);
    while (*urls) {
        const char *url = urls;
        urls += strlen(urls) + 1;
        struct service_info service_info;
        memset(&service_info, 0, sizeof(service_info));
        service_info.healthy_count = 3;
        service_info.unhealthy_count = 3;
        service_info.counter = 0;
        service_info.status = 0;
        service_info.interval = 1000 * 1000 * 5;
        url = parse_params(url, &service_info);

        if (0 == SSTRNCMP(URL_SCHEME_HTTP, url)) {
            parse_url(url + SSTRLEN(URL_SCHEME_HTTP), URL_SCHEME_HTTP, &service_info, HTTP_DEFAULT_PORT);
        } else if (0 == SSTRNCMP(URL_SCHEME_HTTPS, url)) {
            // parse_url(url + SSTRLEN(URL_SCHEME_HTTPS), URL_SCHEME_HTTPS, &service_info, HTTPS_DEFAULT_PORT);
            LOGGER_ERROR("https is not supported yet: [%s]", url);
            exit(EXIT_FAILURE);
        } else {
            char * loc = strstr(url, "://");
            if (loc && loc - url <= 15) {
                LOGGER_ERROR("unknown url scheme: [%.*s]", (int)(loc - url + 3), url);
                exit(EXIT_FAILURE);
            }
            LOGGER_INFO("url scheme not specified, assuming HTTP for: [%s]", url);
            parse_url(url, URL_SCHEME_HTTP, &service_info, HTTP_DEFAULT_PORT);
        }

        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
        struct addrinfo *result;

        LOGGER_INFO("resolving: %s", service_info.host);
        int res = getaddrinfo(service_info.host, NULL, &hints, &result);
        if (0 != res) {
            LOGGER_ERROR("getaddrinfo: %s (%d) : [%s]", gai_strerror(res), res, service_info.host);
            exit(EXIT_FAILURE);
        }
        if (NULL == result) {
            LOGGER_ERROR("failed to resolve host name: [%s]", service_info.host);
            exit(EXIT_FAILURE);
        }
        service_info.addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
        freeaddrinfo(result);

        LOGGER_INFO("scheme: %s, host: %s, port: %d, uri: %s, addr: %s", service_info.scheme, service_info.host, service_info.port, service_info.uri, inet_ntoa(service_info.addr));
        LOGGER_INFO("healthy_count: %d, unhealthy_count: %d, interval: %ld", service_info.healthy_count, service_info.unhealthy_count, service_info.interval/(1000*1000));
        vmbuf_memcpy(&service_info_vect, &service_info, sizeof(service_info));
    }
    size_t num_services = vmbuf_num_elements(&service_info_vect, sizeof(struct service_info));
    LOGGER_INFO("monitoring %zu service%s", num_services, num_services > 1 ? "s" : "");
    struct service_info *service_info = vmbuf_mem(&service_info_vect), *service_info_vect_end = service_info + num_services;
    for (; service_info != service_info_vect_end; ++service_info) {
        void *user_data = timer_worker_init(8*1024*1024, sizeof(service_info), 1, monitor_service_worker);
        *(struct service_info **)user_data = service_info;
    }
    auto_up_worker_ptr = timer_worker_init(8*1024*1024, 0, 0, auto_up_worker);

}

void usage() {
    printf("ribeye version 1.0\nWritten by Lior Amram, Dec 2016\n");
    printf("Usage: ribeye -u, --url <URL> [-u, --url <URL>] ...   can be specified multiple times\n");
    printf("                 [-p, --port]        port number, default: 8080\n");
    printf("                 [-t, --timeout]     http client timeout in seconds, default: 1 sec\n");
    printf("                 [-L, --logfile]     logfile name, default: ribeye.log\n");
    printf("                 [-P, --pidfile]     pidfile name, default: ribeye.pid\n");
    printf("                 [-B, --bindaddr]    bind ip address, default: 127.0.0.1\n");
    printf("                 [-d, --daemonize]   daemonize\n");
    printf("       ribeye --help\n\n");
    exit(EXIT_SUCCESS);
}

void ribeye_request_handler(void) {
    struct http_server_context *http = http_server_get_context();
    if (0 == SSTRCMP(URI_PING, http->uri)) {
        int down_count = global_status ? count_service_status(0) : -1;
        const char *status = down_count == 0 ? HTTP_STATUS_200 : HTTP_STATUS_503;
        return http_server_response_sprintf(status, HTTP_CONTENT_TYPE_TEXT_PLAIN, "%s\n", status);
    } else if (0 == SSTRCMP(URI_UP, http->uri)) {
        global_status = 1;
    } else if (0 == SSTRCMP(URI_DOWN, http->uri)) {
        global_status = 0;
        static struct hashtable query_params = HASHTABLE_INITIALIZER;
        hashtable_init(&query_params, 0);
        http_server_parse_query_params(&query_params);
        const char *t_str = hashtable_lookup_str(&query_params, "t", "0");
        int t = atoi(t_str);
        timer_worker_schedule_next2(auto_up_worker_ptr, t * 1000 * 1000);
    } else if (0 == SSTRCMP(URI_STATUS, http->uri)) {
        struct vmbuf *outbuf = &http->payload;
        dump_service_info(outbuf);
        return http_server_response(HTTP_STATUS_200, HTTP_CONTENT_TYPE_TEXT_PLAIN);
    } else {
        return http_server_response_sprintf(HTTP_STATUS_404, HTTP_CONTENT_TYPE_TEXT_PLAIN, "%s\n", HTTP_STATUS_404);
    }
    http_server_response_sprintf(HTTP_STATUS_200, HTTP_CONTENT_TYPE_TEXT_PLAIN, "global status: %d\n", global_status);
}

int main(int argc, char *argv[])
{
    int port = 8080;
    int daemon_mode = 0;
    struct vmbuf urls = VMBUF_INITIALIZER;
    vmbuf_init(&urls, 0);
    const char *pidfile = "ribeye.pid";
    const char *logfile = "ribeye.log";
    const char *bindaddr = "127.0.0.1";
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"url", required_argument, 0, 'u' },
            {"port", required_argument, 0, 'p'},
            {"timeout", required_argument, 0, 't'},
            {"logfile", required_argument, 0, 'L'},
            {"pidfile", required_argument, 0, 'P'},
            {"bindaddr", required_argument, 0, 'B'},
            {"daemonize", no_argument, 0, 'd'},
            {"help", no_argument, 0, 0 },
            {0, 0, 0, 0 }
        };

        int c = getopt_long(argc, argv, "u:dp:f:t:L:P:B:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                usage();
                break;
            case 'u':
                vmbuf_strcpy(&urls, optarg);
                vmbuf_chrcpy(&urls, 0);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                daemon_mode = 1;
                break;
            case 't':
                client_pool.timeout_handler.timeout = atoi(optarg) * 1000;
                if (1000 <= client_pool.timeout_handler.timeout)
                    break;
                dprintf(STDERR_FILENO, "\nERROR: timeout must be >= 1 second(s)\n\n");
                exit(EXIT_FAILURE);
            case 'L':
                logfile = optarg;
                break;
            case 'P':
                pidfile = optarg;
                break;
            case 'B':
                bindaddr = optarg;
                break;
            case '?':
                exit(EXIT_FAILURE);
            default:
                usage();
        }
    }
    /* terminate the list */
    vmbuf_chrcpy(&urls, 0);
    /*
     * server config
     */
    struct http_server server = HTTP_SERVER_INITIALIZER;
    /* port number */
    server.port = port,
    /* call simple_file_server upon receiving http request */
    server.user_func = ribeye_request_handler,
    /* set idle connection timeout to 60 seconds */
    server.timeout_handler.timeout = 60000,
    /* set fiber's stack size to automatic (0) */
    server.stack_size = 0,
    /* start the server with 100 stacks */
    /* more stacks will be created if necessary */
    server.num_stacks =  100,
    /* we expect most of our requests to be less than 8K */
    server.init_request_size = 8192,
    /* we expect most of our response headers to be less than
       8K */
    server.init_header_size = 8192,
    /* we expect most of our response payloads to be less than
       8K */
    server.init_payload_size = 8192,
    /* no limit on the request size, this should be set to
       something reasonable if you want to protect your server
       against denial of service attack */
    server.max_req_size = 0,
    /* no additional space is needed in the context to store app
       specified data (fiber local storage) */
    server.context_size = 0,
    server.bind_addr = htonl(inet_network(bindaddr));
    server.use_ssl = 0;

    /* initialize server, but don't accept connections yet */
    if (0 > http_server_init2(&server))
        exit(EXIT_FAILURE);

    // if (port == 0) {
    //     struct vmfile vmf = VMFILE_INITIALIZER;
    //     if (0 > vmfile_init(&vmf, "httpd.port", 4096))
    //         exit(EXIT_FAILURE);
    //     vmfile_sprintf(&vmf, "%d", server.port);
    //     vmfile_close(&vmf);
    // }

    if (0 > ribs_server_init(daemon_mode, pidfile, logfile, 1))
        exit(EXIT_FAILURE);

    /* initialize the event loop */
    if (0 > epoll_worker_init())
        exit(EXIT_FAILURE);

    LOGGER_INFO("bindaddr = %s", inet_ntoa((struct in_addr){server.bind_addr}));
    LOGGER_INFO("timeout is set to %ld second%s", client_pool.timeout_handler.timeout/1000, client_pool.timeout_handler.timeout > 1000 ? "s" : "");

    if (http_client_pool_init(&client_pool, 10, 10))
        exit(EXIT_FAILURE);
    /* start accepting connections, must be called after initializing
       epoll worker */
    if (0 > http_server_init_acceptor(&server))
        exit(EXIT_FAILURE);

    init_service_monitors(&urls);

    ribs_server_start();

    return 0;
}