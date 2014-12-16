//
//  ngx_http_google_filter_module.c
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#include "ngx_http_google_util.h"
#include "ngx_http_google_inject.h"
#include "ngx_http_google_request.h"
#include "ngx_http_google_response.h"
#include "ngx_http_google_filter_module.h"

static char *
ngx_http_google_filter(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

static void *
ngx_http_google_filter_create_main_conf(ngx_conf_t * cf);

static void *
ngx_http_google_filter_create_loc_conf(ngx_conf_t* cf);

static char *
ngx_http_google_filter_merge_loc_conf(ngx_conf_t * cf, void * parent,
                                                       void * child);

static ngx_int_t
ngx_http_google_filter_post_config(ngx_conf_t * cf);

ngx_int_t
ngx_http_google_filter_get_var(ngx_http_request_t        * r,
                               ngx_http_variable_value_t * v, uintptr_t data);


static ngx_command_t
ngx_http_google_filter_commands[] = {
  {
    ngx_string("google"),
    NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
    ngx_http_google_filter,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, enable),
    NULL
  },
  {
    ngx_string("google_scholar"),
    NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, scholar),
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t
ngx_http_google_filter_module_ctx = {
  NULL,
  ngx_http_google_filter_post_config,
  ngx_http_google_filter_create_main_conf,
  NULL,
  NULL,
  NULL,
  ngx_http_google_filter_create_loc_conf,
  ngx_http_google_filter_merge_loc_conf,
};

ngx_module_t
ngx_http_google_filter_module = {
  NGX_MODULE_V1,
  &ngx_http_google_filter_module_ctx,
  ngx_http_google_filter_commands,
  NGX_HTTP_MODULE,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NGX_MODULE_V1_PADDING
};

static char *
ngx_http_google_filter(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
  if (ngx_conf_set_flag_slot(cf, cmd, conf)) return NGX_CONF_ERROR;
  
  ngx_http_google_loc_conf_t * glcf = conf;
  if (glcf->enable != 1) return NGX_CONF_OK;
  
  ngx_http_google_main_conf_t * gmcf;
  gmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_google_filter_module);
  gmcf->enable = 1;
  
#if (NGX_HTTP_SSL)
  ngx_http_ssl_srv_conf_t * sscf;
  sscf = ngx_http_conf_get_module_srv_conf(cf, ngx_http_ssl_module);
  if (sscf->enable) glcf->ssl = 1;
#endif
  
  ngx_str_t name;
  ngx_str_set(&name, "google");
  ngx_http_variable_t * var;
  
  var = ngx_pcalloc(cf->pool, sizeof(ngx_http_variable_t));
  var = ngx_http_add_variable(cf, &name, NGX_HTTP_VAR_NOCACHEABLE);
  var->get_handler = ngx_http_google_filter_get_var;
  
  // inject subs & proxy
  if (ngx_http_google_inject_subs (cf)) return NGX_CONF_ERROR;
  if (ngx_http_google_inject_proxy(cf)) return NGX_CONF_ERROR;
  
  return NGX_CONF_OK;
}

static void *
ngx_http_google_filter_create_main_conf(ngx_conf_t* cf)
{
  ngx_http_google_main_conf_t  *conf;
  
  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_google_main_conf_t));
  if (conf == NULL) return NULL;
  
  return conf;
}

static void *
ngx_http_google_filter_create_loc_conf(ngx_conf_t* cf)
{
  ngx_http_google_loc_conf_t  *conf;
  
  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_google_loc_conf_t));
  if (conf == NULL) return NULL;
  
  conf->ssl      = 0;
  conf->enable   = NGX_CONF_UNSET;
  
  return conf;
}

static char *
ngx_http_google_filter_merge_loc_conf(ngx_conf_t * cf, void * parent,
                                                       void * child)
{
  ngx_http_google_loc_conf_t *prev = parent;
  ngx_http_google_loc_conf_t *conf = child;
  
  ngx_conf_merge_value(conf->enable, prev->enable, 0);
  
  return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_google_filter_post_config(ngx_conf_t * cf)
{
  ngx_http_google_main_conf_t * gmcf;
  gmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_google_filter_module);
  if (!gmcf->enable) return NGX_OK;
  
  ngx_http_core_main_conf_t * cmcf;
  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
  
  ngx_http_handler_pt * h;
  h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
 *h = ngx_http_google_request_handler;
  
  // header filter chain
  gmcf->next_header_filter   = ngx_http_top_header_filter;
  ngx_http_top_header_filter = ngx_http_google_response_header_filter;
  
  return NGX_OK;
}

ngx_int_t
ngx_http_google_filter_get_var(ngx_http_request_t        * r,
                               ngx_http_variable_value_t * v, uintptr_t data)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  ngx_int_t ssl = glcf->ssl;
  if (ctx->type == ngx_http_google_type_scholar) ssl = 0;
  
  v->len = 7 + (unsigned)ctx->pass->len;
  if (ssl) v->len++;
  
  v->data = ngx_pcalloc(r->pool, v->len);
  ngx_snprintf(v->data, v->len, "%s%V",
               ssl ? "https://" : "http://", ctx->pass);
  
  return NGX_OK;
}


