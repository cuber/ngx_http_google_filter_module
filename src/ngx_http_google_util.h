//
//  ngx_http_google_util.h
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#ifndef _NGX_HTTP_GOOGLE_UTIL_H
#define _NGX_HTTP_GOOGLE_UTIL_H

#include "ngx_http_google_filter_module.h"

ngx_str_t
ngx_http_google_trim(char * str, size_t len);

ngx_array_t * /* ngx_str_t */
ngx_http_google_explode(ngx_http_request_t * r,
                        ngx_str_t          * v,
                        const char         * de);

ngx_str_t *
ngx_http_google_implode(ngx_http_request_t * r,
                        ngx_array_t        * a, /* ngx_str_t */
                        const char         * de);

ngx_array_t * /* ngx_keyval_t */
ngx_http_google_explode_kv(ngx_http_request_t * r,
                           ngx_str_t          * v,
                           const char         * de);

ngx_str_t *
ngx_http_google_implode_kv(ngx_http_request_t * r,
                           ngx_array_t        * a, /* ngx_keyval_t */
                           const char         * de);

ngx_int_t
ngx_http_google_debug(ngx_pool_t * pool, const char * fmt, ...);



#endif /* defined(_NGX_HTTP_GOOGLE_UTIL_H) */
