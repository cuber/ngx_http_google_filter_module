//
//  ngx_http_google_inject.c
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#include "ngx_http_google_inject.h"

extern ngx_module_t ngx_http_proxy_module;
extern ngx_module_t ngx_http_subs_filter_module;

static char *
ngx_http_google_injcet_args(ngx_conf_t  *  cf,
                            ngx_array_t ** args,
                            const char  *  v)
{
  ngx_str_t * p;
  if (!*args) *args = ngx_array_create(cf->pool, 8, sizeof(ngx_str_t));
  if (!*args) return NGX_CONF_ERROR;
  
  p = ngx_array_push(*args);
  if (!p) return NGX_CONF_ERROR;
  
  p->data = (u_char *)v;
  p->len  = strlen(v);
  
  return NGX_CONF_OK;
}

static char *
ngx_http_google_inject(ngx_conf_t   * cf,
                       ngx_module_t * md,
                       void         * lcf,
                       const char   * k,
                       const int      n,
                       va_list       ap)
{
  ngx_array_t * swap = cf->args;
  cf->args = NULL;
  
  // push command first
  if (ngx_http_google_injcet_args(cf, &cf->args, k)) {
    cf->args = swap;
    return NGX_CONF_ERROR;
  }
  
  int i;
  const char * arg;
  for (i = 0; i < n; i++) {
    arg = va_arg(ap, const char *);
    if (ngx_http_google_injcet_args(cf, &cf->args, arg)) {
      cf->args = swap;
      return NGX_CONF_ERROR;
    }
  }
  
  ngx_command_t * cmd;
  for (cmd = md->commands; cmd->name.len; cmd++) {
    if (!cmd->name.len) continue;
    if (ngx_strcasecmp(cmd->name.data, (u_char *)k)) continue;
    cmd->set(cf, cmd, lcf);
    break;
  }
  cf->args = swap;
  
  return NGX_CONF_OK;
}

static char *
ngx_http_google_inject_subs_args(ngx_conf_t * cf,
                                 const char * cmd,
                                 const int    n,
                                 ...)
{
  va_list ap;
  va_start(ap, n);
  
  void * lcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_subs_filter_module);
  char * rc  = ngx_http_google_inject(cf, &ngx_http_subs_filter_module,
                                      lcf, cmd, n, ap);
  va_end(ap);
  return rc;
}

static char *
ngx_http_google_inject_subs_domain(ngx_conf_t * cf)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_google_filter_module);
  
  ngx_http_core_srv_conf_t  *cscf;
  cscf = ngx_http_conf_get_module_srv_conf(cf, ngx_http_core_module);
  
  ngx_uint_t i, len = 512;
  char * sns_htp, * sns_ssl;
  ngx_http_server_name_t * sns = cscf->server_names.elts, * sn;
  
  for (i = 0; i < cscf->server_names.nelts; i++)
  {
    sn = sns + i;
    if (!sn->name.len) continue;
    
    sns_htp = ngx_pcalloc(cf->pool, len + 1);
    if (!sns_htp) return NGX_CONF_ERROR;
    
    sns_ssl = ngx_pcalloc(cf->pool, len + 1);
    if (!sns_ssl) return NGX_CONF_ERROR;
    
    ngx_snprintf((u_char *)sns_htp, len, "http://%V",  &sn->name);
    ngx_snprintf((u_char *)sns_ssl, len, "https://%V", &sn->name);
    
    if (glcf->ssl) {
      if (ngx_http_google_inject_subs_args(cf,
                                           "subs_filter", 2,
                                           sns_htp,
                                           sns_ssl)) return NGX_CONF_ERROR;
    } else {
      if (ngx_http_google_inject_subs_args(cf,
                                           "subs_filter", 2,
                                           sns_ssl,
                                           sns_htp)) return NGX_CONF_ERROR;
    }
  }
  
  return NGX_CONF_OK;
}

static char *
ngx_http_google_inject_proxy_args(ngx_conf_t * cf,
                                  const char * cmd,
                                  const int    n,
                                  ...)
{
  va_list ap;
  va_start(ap, n);
  
  void * lcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_proxy_module);
  char * rc  = ngx_http_google_inject(cf, &ngx_http_proxy_module,
                                      lcf, cmd, n, ap);
  va_end(ap);
  return rc;
}

char *
ngx_http_google_inject_subs(ngx_conf_t * cf)
{
  do {
    
    if (ngx_http_google_inject_subs_args(cf,
                                         "subs_filter_types", 1,
                                         "*"))
      break;
    
    if (ngx_http_google_inject_subs_args(cf,
                                         "subs_filter", 3,
                                         "([0-9A-Za-z.-]+\\.gstatic\\.com)",
                                         "$host/!$1",
                                         "igr"))
      break;
    
    if (ngx_http_google_inject_subs_args(cf,
                                         "subs_filter", 3,
                                         "((apis)\\.google\\.com)",
                                         "$host/!$1",
                                         "igr"))
      break;
    
    if (ngx_http_google_inject_subs_args(cf,
                                         "subs_filter", 3,
                                         "((www)|(images))\\.google\\.[0-9a-z.]+",
                                         "$host",
                                         "igr"))
      break;
    
    if (ngx_http_google_inject_subs_args(cf,
                                         "subs_filter", 3,
                                         "scholar\\.google\\.[0-9a-z.]+",
                                         "$host/scholar",
                                         "igr"))
      break;
    
    if (ngx_http_google_inject_subs_domain(cf))
      break;
    
    return NGX_CONF_OK;
  } while (0);
  return NGX_CONF_ERROR;
  
}

char *
ngx_http_google_inject_proxy(ngx_conf_t * cf)
{
  do {
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_buffers", 2,
                                          "32",
                                          "32k"))
      break;
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_buffer_size", 1,
                                          "128k"))
      break;
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_pass", 1,
                                          "$google"))
      break;
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_pass", 1,
                                          "$google"))
      break;
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_set_header", 2,
                                          "X-Real-Ip",
                                          "$remote_addr"))
      break;
    
    if (ngx_http_google_inject_proxy_args(cf,
                                          "proxy_set_header", 2,
                                          "X-Forwarded-For",
                                          "$proxy_add_x_forwarded_for"))
      break;
    
    return NGX_CONF_OK;
  } while (0);
  return NGX_CONF_ERROR;
}
