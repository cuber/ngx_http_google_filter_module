//
//  ngx_http_google_response.c
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#include "ngx_http_google_util.h"
#include "ngx_http_google_response.h"

static ngx_int_t
ngx_http_google_response_header_location(ngx_http_request_t    * r,
                                         ngx_http_google_ctx_t * ctx,
                                         ngx_str_t             * v)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  
  u_char *  last = v->data + v->len;
  ngx_uint_t add = 0;
  
  if        (!ngx_strncasecmp(v->data, (u_char *)"http://", 7)) {
    add = 7;
  } else if (!ngx_strncasecmp(v->data, (u_char *)"https://", 8)) {
    add = 8;
  } else {
    return NGX_OK;
  }
  
  ngx_str_t host;
  host.data = v->data + add;
  host.len  = last - host.data;
  
  ngx_str_t uri;
  uri.data = ngx_strlchr(host.data, last, '/');
  if (uri.data) {
    uri .len = last - uri.data;
    host.len = uri.data - host.data;
  } else {
    uri.len = 0;
  }
  
  if (!ngx_strlcasestrn(host.data, host.data + host.len,
                        (u_char *)"google", 6 - 1))
  {
    // none google domains
    // just return back
    return NGX_OK;
  }
  
  if (!ngx_strncasecmp(host.data, (u_char *)"ipv", 3)) {
    ngx_str_t nuri;
    nuri.len  = uri.len + 5;
    nuri.data = ngx_pcalloc(r->pool, nuri.len);
    if (!nuri.data) return NGX_ERROR;
    ngx_snprintf(nuri.data, nuri.len, "/ipv%c%V", host.data[3], &uri);
    uri = nuri;
  } else if (glcf->scholar == 1 &&
             !ngx_strncasecmp(host.data, (u_char *)"scholar", 7))
  {
    if (uri.len &&
        ngx_strncasecmp(uri.data, (u_char *)"/scholar", 8))
    {
      ngx_str_t nuri;
      nuri.len  = uri.len + 8;
      nuri.data = ngx_pcalloc(r->pool, nuri.len);
      if (!nuri.data) return NGX_ERROR;
      ngx_snprintf(nuri.data, nuri.len, "/scholar%V", &uri);
      uri = nuri;
    }
  }
  
  ngx_str_t nv;
  nv.len  = (ctx->ssl ? 8 : 7) + ctx->host->len + uri.len;
  nv.data = ngx_pcalloc(r->pool, nv.len);
  
  if (!nv.data) return NGX_ERROR;
  
  ngx_snprintf(nv.data, nv.len, "%s%V%V",
               ctx->ssl ? "https://" : "http://",
               ctx->host, &uri);
  *v = nv;
  
  return NGX_OK;
}


static ngx_int_t
ngx_http_google_response_header_set_cookie_exempt(ngx_http_request_t    * r,
                                                  ngx_http_google_ctx_t * ctx,
                                                  ngx_array_t           * kvs)
{
  ngx_uid_t i;
  ngx_keyval_t * kv, * hd = kvs->elts;
  
  for (i = 0; i < kvs->nelts; i++) {
    kv = hd + i;
    
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"GOOGLE_ABUSE_EXEMPTION", 22)) {
      if (!kv->value.len) {
        kvs->nelts = 0;
        return NGX_OK;
      }
    }
    
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"expires", 7)) {
      ngx_str_set(&kv->value, "Fri, 01-Jan-2028 00:00:00 GMT");
    }
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_response_header_sort_cookie_conf(const void * a, const void * b)
{
  const ngx_keyval_t * kva = a, * kvb = b;
  if (kva->key.len < kvb->key.len) return  1;
  if (kva->key.len > kvb->key.len) return -1;
  return 0;
}

static ngx_int_t
ngx_http_google_response_header_set_cookie_conf(ngx_http_request_t    * r,
                                                ngx_http_google_ctx_t * ctx,
                                                ngx_str_t             * v)
{
  if (ctx->type != ngx_http_google_type_main &&
      ctx->type != ngx_http_google_type_scholar) return NGX_OK;
  
  ngx_uint_t i;
  ngx_array_t * kvs  = ngx_http_google_explode_kv(r, v, ":");
  if (!kvs) return NGX_ERROR;
  
  ngx_int_t nw = 0;
  ngx_keyval_t * kv, * hd;
  
  hd = kvs->elts;
  for (i = 0; i < kvs->nelts; i++) {
    kv = hd + i;
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"LD", 2)) {
      if (ctx->lang->len) kv->value = *ctx->lang;
      else {
        ngx_str_set(&kv->value, "zh-CN");
      }
    }
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"NW", 2)) nw = 1;
  }
  
  if (!nw) {
    kv = ngx_array_push(kvs);
    if (!kv) return NGX_ERROR;
    ngx_str_set(&kv->key,   "NW");
    ngx_str_set(&kv->value, "1");
  }

  // sort with length
  ngx_sort(kvs->elts, kvs->nelts, sizeof(ngx_keyval_t),
           ngx_http_google_response_header_sort_cookie_conf);
  
  ngx_str_t * nv = ngx_http_google_implode_kv(r, kvs, ":");
  if (!nv) return NGX_ERROR;
  
  *v = *nv;
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_response_header_set_cookie(ngx_http_request_t    * r,
                                           ngx_http_google_ctx_t * ctx,
                                           ngx_table_elt_t       * tb)
{
  ngx_array_t * kvs = ngx_http_google_explode_kv(r, &tb->value, ";");
  if (!kvs) return NGX_ERROR;
  
  ngx_uint_t i;
  ngx_keyval_t * kv, * hd = kvs->elts;
  
  for (i = 0; i < kvs->nelts; i++)
  {
    kv = hd + i;
    
    if (!ngx_strncasecmp(kv->key.data, ctx->conf->data, ctx->conf->len))
    {
      if (ngx_http_google_response_header_set_cookie_conf(r, ctx, &kv->value)) {
        return NGX_ERROR;
      }
    }
    
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"GOOGLE_ABUSE_EXEMPTION", 22))
    {
      if (ngx_http_google_response_header_set_cookie_exempt(r, ctx, kvs)) {
        return NGX_ERROR;
      }
    }
    
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"domain", 6))
    {
      kv->value = *ctx->domain;
    }
    
    if (!ngx_strncasecmp(kv->key.data, (u_char *)"path", 4)) {
      ngx_str_set(&kv->value, "/");
    }
  }
  
  // unset this key
  if (!kvs->nelts) {
    tb->hash = 0; return NGX_OK;
  }
  
  ngx_str_t * set_cookie = ngx_http_google_implode_kv(r, kvs, "; ");
  if (!set_cookie) return NGX_ERROR;
  
  // reset set cookie
  tb->value = *set_cookie;
  
  return NGX_OK;
}

ngx_int_t
ngx_http_google_response_header_filter(ngx_http_request_t * r)
{
  ngx_http_google_main_conf_t * gmcf;
  gmcf = ngx_http_get_module_main_conf(r, ngx_http_google_filter_module);
  
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  if (glcf->enable != 1) return gmcf->next_header_filter(r);
  
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  ngx_uint_t i;
  ngx_list_part_t * pt = &r->headers_out.headers.part;
  ngx_table_elt_t * hd = pt->elts, * tb;
  
  for (i = 0; /* void */; i++)
  {
    if (i >= pt->nelts) {
      
      if (pt->next == NULL) break;
      
      pt = pt->next;
      hd = pt->elts;
      i  = 0;
    }
    
    tb = hd + i;
    
    if (!ngx_strncasecmp(tb->key.data, (u_char *)"Location", 8)) {
      if (ngx_http_google_response_header_location(r, ctx, &tb->value)) {
        return NGX_ERROR;
      }
    }
    
    if (!ngx_strncasecmp(tb->key.data, (u_char *)"Set-Cookie", 10)) {
      if (ngx_http_google_response_header_set_cookie(r, ctx, tb)) {
        return NGX_ERROR;
      }
    }
  }
  
  tb = ngx_list_push(&r->headers_out.headers);
  if (!tb) return NGX_ERROR;
  
  // add server header
  ngx_str_set(&tb->key, "Server");
  tb->hash       = 1;
  tb->value.len  = ctx->host->len;
  tb->value.len += sizeof(NGX_HTTP_GOOGLE_FILTER_MODULE_VERSION);
  tb->value.data = ngx_pcalloc(r->pool, tb->value.len);
  // host / version
  ngx_snprintf(tb->value.data, tb->value.len,
               "%V/" NGX_HTTP_GOOGLE_FILTER_MODULE_VERSION,
               ctx->host);
  
  // replace with new headers
  r->headers_out.server = tb;
  
  return gmcf->next_header_filter(r);
}

ngx_int_t
ngx_http_google_response_body_filter(ngx_http_request_t * r, ngx_chain_t * in)
{
  ngx_http_google_main_conf_t * gmcf;
  gmcf = ngx_http_get_module_main_conf(r, ngx_http_google_filter_module);
  
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  if (glcf->enable != 1) return gmcf->next_body_filter(r, in);
  
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  if (!ctx->robots)      return gmcf->next_body_filter(r, in);
  if (glcf->robots == 1) return gmcf->next_body_filter(r, in);
  
  ngx_chain_t out;
  ngx_memzero(&out, sizeof(ngx_chain_t));
  
  ngx_str_t text;
  ngx_str_set(&text, "User-agent: *" CRLF
                     "Disallow: /"   CRLF);
  
  out.buf = ngx_create_temp_buf(r->pool, text.len);
  if (!out.buf) return NGX_ERROR;
  out.buf->last_buf = 1;
  
  out.buf->last = ngx_copy(out.buf->last, text.data, text.len);
  return gmcf->next_body_filter(r, &out);
}
