//
//  ngx_http_google_filter_module.h
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#ifndef _NGX_HTTP_GOOGLE_FILTER_MODULE_H
#define _NGX_HTTP_GOOGLE_FILTER_MODULE_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <assert.h>

typedef enum {
  ngx_http_google_type_main = 0,
  ngx_http_google_type_verify,
  ngx_http_google_type_scholar,
  ngx_http_google_type_redirect,
} ngx_http_google_type_t;

typedef struct {
  ngx_flag_t  enable;
  ngx_str_t   scholar, language;
  ngx_array_t * ssloff; /* array ot ngx_str_t */
} ngx_http_google_loc_conf_t;

typedef struct {
  ngx_int_t enable;
  ngx_http_output_header_filter_pt next_header_filter;
} ngx_http_google_main_conf_t;

typedef struct {
  ngx_int_t     ssl;
  ngx_str_t   * host, * pass, * lang, * uri, * arg;
  ngx_array_t * args, * cookies; /* array of ngx_keyval_t */
  ngx_http_google_type_t type;
  struct { ngx_int_t scholar; } enable;
} ngx_http_google_ctx_t;

extern ngx_module_t ngx_http_proxy_module;
extern ngx_module_t ngx_http_subs_filter_module;
extern ngx_module_t ngx_http_google_filter_module;

#endif /* defined(_NGX_HTTP_GOOGLE_FILTER_MODULE_H) */
