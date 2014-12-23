//
//  ngx_http_google_inject.h
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#ifndef _NGX_HTTP_GOOGLE_INJECT_H
#define _NGX_HTTP_GOOGLE_INJECT_H

#include "ngx_http_google_filter_module.h"

char * ngx_http_google_inject_subs (ngx_conf_t *);
char * ngx_http_google_inject_proxy(ngx_conf_t *);

#endif /* defined(_NGX_HTTP_GOOGLE_INJECT_H) */
