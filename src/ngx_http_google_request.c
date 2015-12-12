//
//  ngx_http_google_request.c
//  nginx
//
//  Created by Cube on 14/12/15.
//  Copyright (c) 2014年 Cube. All rights reserved.
//

#include "ngx_http_google_util.h"
#include "ngx_http_google_request.h"

static ngx_http_google_ctx_t *
ngx_http_google_create_ctx(ngx_http_request_t * r)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_google_ctx_t));
  
  if (!ctx)         return NULL;
  ctx->host   = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!ctx->host)   return NULL;
  ctx->conf   = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!ctx->conf)   return NULL;
  ctx->pass   = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!ctx->pass)   return NULL;
  ctx->arg    = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!ctx->arg)    return NULL;
  ctx->domain = ngx_pcalloc(r->pool, sizeof(ngx_str_t));
  if (!ctx->domain) return NULL;
  
  // default conf key
  ngx_str_set(ctx->conf, "PREF");
  
  ctx->uri  = &r->unparsed_uri;
  ctx->host = &r->headers_in.host->value;
  ctx->lang = &glcf->language;
  ctx->type = ngx_http_google_type_main;
  
  ngx_str_t domain = *ctx->host;
  
  u_char * last = domain.data + domain.len, * find;
  if ((find = ngx_strlchr(domain.data, last, ':'))) {
    domain.len = find - domain.data;
  }
  
  ctx->domain->len  = domain.len + 1;
  ctx->domain->data = ngx_pcalloc(r->pool, ctx->domain->len);
  if (!ctx->domain->data) return NULL;
  
  ngx_snprintf(ctx->domain->data, ctx->domain->len, ".%V", &domain);
  
  if (ngx_inet_addr(domain.data, domain.len) != INADDR_NONE) {
    ctx->domain->data++;
    ctx->domain->len --;
  }
  
  // default language
  if (!ctx->lang->len) {
    ngx_str_set(ctx->lang, "zh-CN");
  }
  
  ctx->robots = (ctx->uri->len == 11 &&
                 !ngx_strncmp(ctx->uri->data, "/robots.txt", 11));
  
#if (NGX_HTTP_SSL)
  ngx_http_ssl_srv_conf_t * sscf;
  sscf = ngx_http_get_module_srv_conf(r, ngx_http_ssl_module);
  if (sscf->enable || r->http_connection->addr_conf->ssl) ctx->ssl = 1;
#endif
  
  return ctx;
}

static ngx_int_t
ngx_http_google_request_parse_cookie_gz(ngx_http_request_t    * r,
                                        ngx_http_google_ctx_t * ctx)
{
  ngx_uint_t i;
  ngx_keyval_t * kv, * hd = ctx->cookies->elts;
  
  for (i = 0; i < ctx->cookies->nelts; i++) {
    kv = hd + i;
    if (ngx_strncasecmp(kv->key.data, (u_char *)"GZ", 2)) continue;
    ngx_str_set(&kv->value, "Z=0");
    break;
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_cookie_conf(ngx_http_request_t    * r,
                                          ngx_http_google_ctx_t * ctx)
{
  ngx_uint_t i;
  ngx_keyval_t * kv, * hd = ctx->cookies->elts;
  
  for (i = 0; i < ctx->cookies->nelts; i++)
  {
    kv = hd + i;
    if (ngx_strncasecmp(kv->key.data, ctx->conf->data, ctx->conf->len)) continue;
    // find cr
    u_char * last = kv->value.data + kv->value.len;
    if (ngx_strlcasestrn(kv->value.data, last, (u_char *)"CR=", 2)) ctx->ncr = 1;
    break;
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_redirect(ngx_http_request_t    * r,
                                       ngx_http_google_ctx_t * ctx)
{
  ctx->uri->data += 2;
  ctx->uri->len  -= 2;
  
  u_char * last   = ctx->uri->data + ctx->uri->len;
  u_char * slash  = ngx_strlchr(ctx->uri->data, last, '/');
  
  ctx->pass->data = ctx->uri->data;
  ctx->pass->len  = ctx->uri->len;
  
  if (slash) {
    ctx->uri->data = slash;
    ctx->uri->len  = last - slash;
    ctx->pass->len = slash - ctx->pass->data;
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_scholar(ngx_http_request_t    * r,
                                      ngx_http_google_ctx_t * ctx)
{
  if (ngx_http_google_request_parse_cookie_conf(r, ctx)) return NGX_ERROR;
  
  if (ctx->uri->len == 12 && ctx->args && ctx->args->nelts) {
    
    u_char * refer = ctx->uri->data + 9;
    
    if (!ngx_strncmp(refer, "bib", 3) ||
        !ngx_strncmp(refer, "enw", 3) ||
        !ngx_strncmp(refer, "ris", 3) ||
        !ngx_strncmp(refer, "rfw", 3))
    {
      ngx_uint_t i;
      ngx_keyval_t * kv, * hd = ctx->args->elts;
      
      for (i = 0; i < ctx->args->nelts; i++)
      {
        kv = hd + i;
        if (!kv->key.len || *kv->key.data != 'q') continue;
        
        u_char * last = kv->value.data + kv->value.len;
        u_char * find = ngx_strlcasestrn(kv->value.data, last,
                                         ctx->host->data, ctx->host->len - 1);
        
        if (!find) break;
        kv->value.len = find - kv->value.data;
        
        ngx_str_t nval;
        nval.len  = kv->value.len + sizeof("scholar.google.com");
        nval.data = ngx_pcalloc(r->pool, nval.len);
        
        if (!nval.data) return NGX_ERROR;
        ngx_snprintf(nval.data, nval.len, "%Vscholar.google.com/", &kv->value);
        kv->value = nval;
        
        break;
      } // end of for args
    } // end of if refer
  } else {
    
    if (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/schhp", 6) ||
        !ctx->args->nelts)
    {
      ngx_str_set(ctx->uri, "/");
    }
    
    if (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/scholar", 8))
    {
      ngx_str_t uri = *ctx->uri;
      
      // strip "/scholar" uri
      uri.data += 8;
      uri.len  -= 8;
      
      ngx_uint_t i, strip = 1;
      ngx_keyval_t * kv, * hd = ctx->args->elts;
      
      // traverse args
      for (i = 0; i < ctx->args->nelts; i++)
      {
        kv = hd + i;
        if (!kv->key.len) continue;
        if (kv->key.len == 1 && *kv->key.data == 'q')
        {
          strip = 0; break;
        }
        if (kv->key.len == 5 && !ngx_strncasecmp(kv->key.data,
                                                (u_char *)"cites", 5))
        {
          strip = 0; break;
        }
        if (kv->key.len == 7 && !ngx_strncasecmp(kv->key.data,
                                                (u_char *)"cluster", 7))
        {
          strip = 0; break;
        }
        
      }
      
      if (uri.len) switch (*uri.data) {
        case '?': case '/': break;
        default: strip = 0; break;
      }
      
      if (strip) *ctx->uri = uri;
    }
  }
  
  if (!ctx->ncr && ctx->uri->len == 1) {
    ngx_str_set(ctx->uri, "/ncr");
  }
  
  ngx_str_set(ctx->pass, "scholar.google.com");
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_books(ngx_http_request_t    * r,
                                      ngx_http_google_ctx_t * ctx)
{
  if (ngx_http_google_request_parse_cookie_conf(r, ctx)) return NGX_ERROR;

  if (ctx->uri->len == 12 && ctx->args && ctx->args->nelts) {

    u_char * refer = ctx->uri->data + 9;

    if (!ngx_strncmp(refer, "bib", 3) ||
        !ngx_strncmp(refer, "enw", 3) ||
        !ngx_strncmp(refer, "ris", 3) ||
        !ngx_strncmp(refer, "rfw", 3))
    {
      ngx_uint_t i;
      ngx_keyval_t * kv, * hd = ctx->args->elts;

      for (i = 0; i < ctx->args->nelts; i++)
      {
        kv = hd + i;
        if (!kv->key.len || *kv->key.data != 'q') continue;

        u_char * last = kv->value.data + kv->value.len;
        u_char * find = ngx_strlcasestrn(kv->value.data, last,
                                         ctx->host->data, ctx->host->len - 1);

        if (!find) break;
        kv->value.len = find - kv->value.data;

        ngx_str_t nval;
        nval.len  = kv->value.len + sizeof("books.google.com");
        nval.data = ngx_pcalloc(r->pool, nval.len);

        if (!nval.data) return NGX_ERROR;
        ngx_snprintf(nval.data, nval.len, "%Vbooks.google.com/", &kv->value);
        kv->value = nval;

        break;
      } // end of for args
    } // end of if refer
  } else {

    if (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/books", 6))
    {
      ngx_uint_t i, strip = 1;
      ngx_keyval_t * kv, * hd = ctx->args->elts;

      for (i = 0; i < ctx->args->nelts; i++) {
        kv = hd + i;
        if (!kv->key.len) continue;
        if (kv->key.len == 2 && *kv->key.data == 'id')
        {
          strip = 0; break;
        }
      }

      if (strip) {
        ctx->uri->data += 6;
        ctx->uri->len  -= 6;
      }
    }
  }

  /* ncr does not work at the porting moment
  if (!ctx->ncr && ctx->uri->len == 1) {
    ngx_str_set(ctx->uri, "/ncr");
  }
  */

  ngx_str_set(ctx->pass, "books.google.com");

  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_verify(ngx_http_request_t    * r,
                                     ngx_http_google_ctx_t * ctx)
{
  ctx->pass->len  = sizeof("ipvX.google.com") - 1;
  ctx->pass->data = ngx_pcalloc(r->pool, ctx->pass->len);
  
  ngx_snprintf(ctx->pass->data, ctx->pass->len,
               "ipv%c.google.com", ctx->uri->data[4]);
  
  ctx->uri->data += 5;
  ctx->uri->len  -= 5;
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_main(ngx_http_request_t    * r,
                                   ngx_http_google_ctx_t * ctx)
{
  ngx_str_set(ctx->pass, "www.google.com");
  
  if (ngx_http_google_request_parse_cookie_gz  (r, ctx)) return NGX_ERROR;
  if (ctx->robots) return NGX_OK;
  if (ngx_http_google_request_parse_cookie_conf(r, ctx)) return NGX_ERROR;
  
  if (ctx->ncr) return NGX_OK;
  
  if ((ctx->uri->len == 1 && *ctx->uri->data == '/') ||
      (ctx->uri->len == 6 &&
       !ngx_strncasecmp(ctx->uri->data, (u_char *)"/webhp", 6)))
  {
    ngx_str_set(ctx->uri, "/ncr");
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parse_host(ngx_http_request_t    * r,
                                   ngx_http_google_ctx_t * ctx)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  
  // redirect
  if (ctx->uri->len > 2 && !ngx_strncmp(ctx->uri->data, "/!", 2))
  {
    ctx->type = ngx_http_google_type_redirect;
    if (ngx_http_google_request_parse_redirect(r, ctx)) return NGX_ERROR;
  }
  
  // scholar
  if (glcf->scholar == 1 &&
      ctx->uri->len > 7   &&
      (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/scholar", 8) ||
       !ngx_strncasecmp(ctx->uri->data, (u_char *)"/schhp",   6)))
  {
    ctx->type = ngx_http_google_type_scholar;
    // conf key change
    ngx_str_set(ctx->conf, "GSP");
    if (ngx_http_google_request_parse_scholar(r, ctx)) return NGX_ERROR;
  }
  
  // books
  if (glcf->books == 1 &&
      ctx->uri->len > 5   &&
      (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/books", 6)))
  {
    ctx->type = ngx_http_google_type_books;
    if (ngx_http_google_request_parse_books(r, ctx)) return NGX_ERROR;
  }

  // verify
  if (ctx->uri->len > 4 &&
      (!ngx_strncasecmp(ctx->uri->data, (u_char *)"/ipv4", 5) ||
       !ngx_strncasecmp(ctx->uri->data, (u_char *)"/ipv6", 5)))
  {
    ctx->type = ngx_http_google_type_verify;
    if (ngx_http_google_request_parse_verify(r, ctx)) return NGX_ERROR;
  }
  
  // www
  if (ctx->type == ngx_http_google_type_main)
  {
    if (ngx_http_google_request_parse_main(r, ctx)) return NGX_ERROR;
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_parser(ngx_http_request_t    * r,
                               ngx_http_google_ctx_t * ctx)
{
  // parse arg
  u_char * last  = r->unparsed_uri.data + r->unparsed_uri.len;
  ctx->arg->data = ngx_strlchr(ctx->uri->data, last, '?');
  
  // parse uri
  if (ctx->arg->data) {
    ctx->arg->len  = last - ++ctx->arg->data;
    ctx->uri->len -= ctx->arg->len + 1;
  }
  
  // parse args
  if (ctx->arg->len) {
    ctx->args = ngx_http_google_explode_kv(r, ctx->arg, "&");
  } else {
    ctx->args = ngx_array_create(r->pool, 4, sizeof(ngx_keyval_t));
  }
  if (!ctx->args) return NGX_ERROR;
  
  // parse cookies
  if (r->headers_in.cookies.nelts) {
    ngx_table_elt_t ** ck = r->headers_in.cookies.elts;
    ctx->cookies = ngx_http_google_explode_kv(r, &(*ck)->value, ";");
  } else {
    ctx->cookies = ngx_array_create(r->pool, 4, sizeof(ngx_keyval_t));
  }
  if (!ctx->cookies) return NGX_ERROR;
  
  // parse host
  if (ngx_http_google_request_parse_host(r, ctx)) return NGX_ERROR;
  
  // traverse headers
  ngx_uint_t i, acl = 0;
  ngx_list_part_t * pt = &r->headers_in.headers.part;
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
    if (!ngx_strncasecmp(tb->key.data, (u_char *)"Accept-Language", 15)) {
      acl = 1; tb->value = *ctx->lang;
    }
  }
  
  if (!acl) {
    tb = ngx_list_push(&r->headers_in.headers);
    if (!tb) return NGX_ERROR;
    
    ngx_str_set(&tb->key, "Accept-Language");
    tb->value = *ctx->lang;
    tb->hash  = ngx_hash_key_lc(tb->key.data, tb->key.len);
  }
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_request_generater(ngx_http_request_t    * r,
                                  ngx_http_google_ctx_t * ctx)
{
  ngx_uint_t i;
  ngx_table_elt_t *  tb;
  ngx_table_elt_t ** ptr;
  
  ngx_str_t * cookie = ngx_http_google_implode_kv(r, ctx->cookies, "; ");
  if (!cookie) return NGX_ERROR;
  
  if (!r->headers_in.cookies.nelts)
  {
    tb = ngx_list_push(&r->headers_in.headers);
    if (!tb) return NGX_ERROR;
    
    ngx_str_set(&tb->key, "Cookie");
    tb->value = *cookie;
    tb->hash  = ngx_hash_key_lc(tb->key.data, tb->key.len);
    
  } else {
    
    ptr = r->headers_in.cookies.elts;
    for (i = 0; i < r->headers_in.cookies.nelts; i++) {
      ptr[i]->value = *cookie;
    }
    
  }
  
  ngx_str_t * arg = ngx_http_google_implode_kv(r, ctx->args, "&");
  if (!arg) return NGX_ERROR;
  ctx->arg = arg;
  
  if (!ctx->arg->len)
  {
    r->unparsed_uri = *ctx->uri;
    
  } else {
    
    ngx_str_t uri;
    uri.len  = ctx->uri->len + 1 + ctx->arg->len;
    uri.data = ngx_pcalloc(r->pool, uri.len);
    
    if (!uri.data) return NGX_ERROR;
    ngx_snprintf(uri.data, uri.len, "%V?%V", ctx->uri, ctx->arg);
    
    r->unparsed_uri = uri;
  }
  
  return NGX_OK;
}

ngx_int_t
ngx_http_google_request_handler(ngx_http_request_t * r)
{
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  if (glcf->enable != 1) return NGX_DECLINED;
  
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_google_create_ctx(r);
  
  if (!ctx) return NGX_ERROR;
  ngx_http_set_ctx(r, ctx, ngx_http_google_filter_module);
  
  if (ngx_http_google_request_parser   (r, ctx)) return NGX_ERROR;
  if (ngx_http_google_request_generater(r, ctx)) return NGX_ERROR;

  return NGX_DECLINED;
}


