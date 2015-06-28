//
//  ngx_http_google_util.c
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#include "ngx_http_google_util.h"

ngx_str_t
ngx_http_google_trim(char * str, size_t len)
{
  ngx_str_t v;
  v.data = (u_char *)str;
  v.len  = len;
  
  if (!v.data || !v.len) return v;
  while (v.len > 0 && v.data[0]         == ' ') { v.len--; v.data++; }
  while (v.len > 0 && v.data[v.len - 1] == ' ') { v.len--; }
  return v;
}

ngx_array_t * /* ngx_str_t */
ngx_http_google_explode(ngx_http_request_t * r,
                        ngx_str_t          * v, const char * de)
{
  ngx_str_t   * s;
  ngx_array_t * ss = ngx_array_create(r->pool, 4, sizeof(ngx_keyval_t));
  if (!ss) return NULL;
  
  char * dup = ngx_pcalloc(r->pool, v->len + 1);
  if (!dup) return NULL;
  memcpy(dup, v->data, v->len);
  
  char * pch, * brkt; size_t len;
  pch = strtok_r(dup, de, &brkt);
  for (; pch; pch = strtok_r(NULL, de, &brkt))
  {
    if (!(len = strlen(pch))) continue;
    s = ngx_array_push(ss);
    if (!s) return NULL;
   *s = ngx_http_google_trim(pch, strlen(pch));
  }
  
  return ss;
}

ngx_str_t *
ngx_http_google_implode(ngx_http_request_t * r,
                        ngx_array_t        * a, /* ngx_str_t */
                        const char         * de)
{
  u_char * buf;
  size_t   len = 0, delen = strlen(de);
  
  ngx_uint_t i;
  ngx_str_t * s, * hd = a->elts, * str;
  
  for (i = 0; i < a->nelts; i++) {
    s = hd + i;
    len += s->len + delen;
  }
  
  str = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!str) return NULL;
  
  buf = str->data = ngx_pcalloc(r->pool, len);
  if (!buf) return NULL;
  
  for (i = 0; i < a->nelts; i++) {
    s = hd + i;
    if (buf > str->data) buf = ngx_copy(buf, de, delen);
    buf = ngx_copy(buf, s->data, s->len);
  }
  
  str->len = buf - str->data;
  return str;
}

ngx_array_t * /* ngx_keyval_t */
ngx_http_google_explode_kv(ngx_http_request_t * r,
                           ngx_str_t          * v, const char * de)
{
  ngx_keyval_t * kv, tkv;
  ngx_array_t  * kvs = ngx_array_create(r->pool, 4, sizeof(ngx_keyval_t));
  if (!kvs) return NULL;
  
  char * dup = ngx_pcalloc(r->pool, v->len + 1);
  if (!dup) return NULL;
  memcpy(dup, v->data, v->len);
  
  char * pch, * sep, * brkt;
  pch = strtok_r(dup, de, &brkt);
  
  for (; pch; pch = strtok_r(NULL, de, &brkt))
  {
    sep = strchr(pch, '=');
    if (!sep) continue;
    *sep++ = '\0';
    tkv.key   = ngx_http_google_trim(pch, strlen(pch));
    tkv.value = ngx_http_google_trim(sep, strlen(sep));
    if (!tkv.key.len) continue;
    kv = ngx_array_push(kvs);
    if (!kv) return NULL;
    *kv = tkv;
  }
  
  return kvs;
}

ngx_str_t *
ngx_http_google_implode_kv(ngx_http_request_t * r,
                           ngx_array_t        * a, /* ngx_keyval_t */
                           const char         * de)
{
  u_char * buf;
  size_t   len = 0, delen = strlen(de);
  
  ngx_uint_t i;
  ngx_str_t * str;
  ngx_keyval_t * kv, * hd = a->elts;
  
  for (i = 0; i < a->nelts; i++) {
    kv   = hd + i;
    len += kv->key.len + 1 + kv->value.len + delen;
  }
  
  str = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!str) return NULL;
  
  buf = str->data = ngx_pcalloc(r->pool, len);
  if (!buf) return NULL;
  
  for (i = 0; i < a->nelts; i++) {
    kv = hd + i;
    if (buf > str->data) buf = ngx_copy(buf, de, delen);
    buf = ngx_copy(buf, kv->key.data,   kv->key.len);
    *buf++ = '=';
    buf = ngx_copy(buf, kv->value.data, kv->value.len);
  }
  
  str->len = buf - str->data;
  return str;
}

ngx_int_t
ngx_http_google_debug(ngx_pool_t * pool, const char * fmt, ...)
{
  u_char   * buf;
  ngx_uint_t len = 4096;
  
  buf = ngx_pcalloc(pool, len + 1);
  if (!buf) {
    return fprintf(stderr, "ngx_pcalloc(%lu) failed\n", (unsigned long)len);
  }
  
  va_list args;
  va_start(args, fmt);
  ngx_vsnprintf(buf, len, fmt, args);
  va_end(args);
  
  return fprintf(stdout, "%s", buf);
}

