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

static char *
ngx_http_google_filter_language(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);

static void *
ngx_http_google_filter_create_main_conf(ngx_conf_t * cf);

static void *
ngx_http_google_filter_create_loc_conf(ngx_conf_t* cf);

static char *
ngx_http_google_filter_merge_loc_conf(ngx_conf_t * cf, void * parent,
                                                       void * child);

static ngx_int_t
ngx_http_google_filter_pre_config(ngx_conf_t * cf);

static ngx_int_t
ngx_http_google_filter_post_config(ngx_conf_t * cf);

static ngx_int_t
ngx_http_google_filter_google_var(ngx_http_request_t        * r,
                                  ngx_http_variable_value_t * v,
                                  uintptr_t data);

static ngx_int_t
ngx_http_google_filter_google_host_var(ngx_http_request_t        * r,
                                       ngx_http_variable_value_t * v,
                                       uintptr_t data);

static ngx_int_t
ngx_http_google_filter_google_schema_var(ngx_http_request_t        * r,
                                         ngx_http_variable_value_t * v,
                                         uintptr_t data);

static ngx_int_t
ngx_http_google_filter_google_schema_reverse_var(ngx_http_request_t        * r,
                                                 ngx_http_variable_value_t * v,
                                                 uintptr_t data);

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
    ngx_conf_set_flag_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, scholar),
    NULL
  },
  {
    ngx_string("google_language"),
    NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
    ngx_http_google_filter_language,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, language),
    NULL
  },
  {
    ngx_string("google_ssl_off"),
    NGX_HTTP_LOC_CONF | NGX_CONF_1MORE,
    ngx_conf_set_str_array_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, ssloff),
    NULL
  },
  {
    ngx_string("google_robots_allow"),
    NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_google_loc_conf_t, robots),
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t
ngx_http_google_filter_module_ctx = {
  ngx_http_google_filter_pre_config,
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

static ngx_http_variable_t ngx_http_google_vars[] = {
  { ngx_string("google"),
    NULL, ngx_http_google_filter_google_var,
    0,
    NGX_HTTP_VAR_CHANGEABLE | NGX_HTTP_VAR_NOCACHEABLE | NGX_HTTP_VAR_NOHASH,
    0 },
  { ngx_string("google_host"),
    NULL, ngx_http_google_filter_google_host_var,
    0,
    NGX_HTTP_VAR_CHANGEABLE | NGX_HTTP_VAR_NOCACHEABLE | NGX_HTTP_VAR_NOHASH,
    0 },
  { ngx_string("google_schema"),
    NULL, ngx_http_google_filter_google_schema_var,
    0,
    NGX_HTTP_VAR_CHANGEABLE | NGX_HTTP_VAR_NOCACHEABLE | NGX_HTTP_VAR_NOHASH,
    0 },
  { ngx_string("google_schema_reverse"),
    NULL, ngx_http_google_filter_google_schema_reverse_var,
    0,
    NGX_HTTP_VAR_CHANGEABLE | NGX_HTTP_VAR_NOCACHEABLE | NGX_HTTP_VAR_NOHASH,
    0 },
  { ngx_null_string, NULL, NULL, 0, 0, 0 }
};

static ngx_str_t ngx_http_google_language[] = {
  ngx_string("ar"),    // Arabic
  ngx_string("bg"),    // Bulgarian
  ngx_string("ca"),    // Catalan
  ngx_string("zh-CN"), // Chinese (Simplified)
  ngx_string("zh-TW"), // Chinese (Traditional)
  ngx_string("hr"),    // Croatian
  ngx_string("cs"),    // Czech
  ngx_string("da"),    // Danish
  ngx_string("nl"),    // Dutch
  ngx_string("en"),    // English
  ngx_string("tl"),    // Filipino
  ngx_string("fi"),    // Finnish
  ngx_string("fr"),    // French
  ngx_string("de"),    // German
  ngx_string("el"),    // Greek
  ngx_string("iw"),    // Hebrew
  ngx_string("hi"),    // Hindi
  ngx_string("hu"),    // Hungarian
  ngx_string("id"),    // Indonesian
  ngx_string("it"),    // Italian
  ngx_string("ja"),    // Japanese
  ngx_string("ko"),    // Korean
  ngx_string("lv"),    // Latvian
  ngx_string("lt"),    // Lithuanian
  ngx_string("no"),    // Norwegian
  ngx_string("fa"),    // Persian
  ngx_string("pl"),    // Polish
  ngx_string("pt-BR"), // Portuguese (Brazil)
  ngx_string("pt-PT"), // Portuguese (Portugal)
  ngx_string("ro"),    // Romanian
  ngx_string("ru"),    // Russian
  ngx_string("sr"),    // Serbian
  ngx_string("sk"),    // Slovak
  ngx_string("sl"),    // Slovenian
  ngx_string("es"),    // Spanish
  ngx_string("sv"),    // Swedish
  ngx_string("th"),    // Thai
  ngx_string("tr"),    // Turkish
  ngx_string("uk"),    // Ukrainian
  ngx_string("vi"),    // Vietnamese
  ngx_null_string
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
  
  if (ngx_http_google_inject_subs (cf)) return NGX_CONF_ERROR;
  if (ngx_http_google_inject_proxy(cf)) return NGX_CONF_ERROR;
  
  return NGX_CONF_OK;
}

static char *
ngx_http_google_filter_language(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
  ngx_http_google_loc_conf_t * glcf = conf;
  
  ngx_str_t * v   = ((ngx_str_t *)cf->args->elts) + 1;
  ngx_str_t * def = ngx_http_google_language;
  
  for (; def->len; def++) {
    if (v->len != def->len)                        continue;
    if (ngx_strncmp(v->data, def->data, def->len)) continue;
    glcf->language = *v;
  }
  
  if (!glcf->language.len) return "language not support";
  
  
  return NGX_CONF_OK;
}

static void *
ngx_http_google_filter_create_main_conf(ngx_conf_t * cf)
{
  ngx_http_google_main_conf_t  *conf;
  
  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_google_main_conf_t));
  if (conf == NULL) return NULL;
  
  return conf;
}

static void *
ngx_http_google_filter_create_loc_conf(ngx_conf_t * cf)
{
  ngx_http_google_loc_conf_t  *conf;
  
  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_google_loc_conf_t));
  if (conf == NULL) return NULL;
  
  conf->robots  = NGX_CONF_UNSET;
  conf->enable  = NGX_CONF_UNSET;
  conf->scholar = NGX_CONF_UNSET;
  conf->ssloff  = NGX_CONF_UNSET_PTR;
  
  return conf;
}

static char *
ngx_http_google_filter_merge_loc_conf(ngx_conf_t * cf, void * parent,
                                                       void * child)
{
  ngx_http_google_loc_conf_t * prev = parent;
  ngx_http_google_loc_conf_t * conf = child;
  
  ngx_conf_merge_value    (conf->robots,   prev->robots,   NGX_CONF_UNSET);
  ngx_conf_merge_value    (conf->enable,   prev->enable,   NGX_CONF_UNSET);
  ngx_conf_merge_value    (conf->scholar,  prev->scholar,  NGX_CONF_UNSET);
  ngx_conf_merge_ptr_value(conf->ssloff,   prev->ssloff,   NGX_CONF_UNSET_PTR);
  ngx_conf_merge_str_value(conf->language, prev->language, "zh-CN");
  
  return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_google_filter_pre_config(ngx_conf_t * cf)
{
  ngx_http_variable_t * var, * v;
  
  for (v = ngx_http_google_vars; v->name.len; v++)
  {
    var = ngx_http_add_variable(cf, &v->name, v->flags);
    if (var == NULL) {
      return NGX_ERROR;
    }
    
    var->get_handler = v->get_handler;
    var->data = v->data;
  }
  
  return NGX_OK;
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
  
  // body filter chain
  gmcf->next_body_filter     = ngx_http_top_body_filter;
  ngx_http_top_body_filter   = ngx_http_google_response_body_filter;
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_filter_google_var(ngx_http_request_t        * r,
                                  ngx_http_variable_value_t * v, uintptr_t data)
{
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  if (ctx == NULL) {
    v->not_found = 1;
    return NGX_OK;
  }
  
  ngx_http_google_loc_conf_t * glcf;
  glcf = ngx_http_get_module_loc_conf(r, ngx_http_google_filter_module);
  
  ngx_int_t ssl = 1;
  
  if (glcf->ssloff != NGX_CONF_UNSET_PTR) {
    
    ngx_uint_t   i;
    ngx_str_t * hd = glcf->ssloff->elts, * domain;
    
    for (i = 0; i < glcf->ssloff->nelts; i++) {
      domain = hd + i;
      if (ctx->pass->len != domain->len)                           continue;
      if (ngx_strncmp(ctx->pass->data, domain->data, domain->len)) continue;
      ssl = 0; break;
    }
  }
  
#if ! (NGX_HTTP_SSL)
  ssl = 0;
#endif
  
  v->len = 7 + (unsigned)ctx->pass->len;
  if (ssl) v->len++;
  
  v->data = ngx_pcalloc(r->pool, v->len);
  ngx_snprintf(v->data, v->len, "%s%V", ssl ? "https://" : "http://", ctx->pass);
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_filter_google_host_var(ngx_http_request_t        * r,
                                       ngx_http_variable_value_t * v,
                                       uintptr_t data)
{
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  if (ctx == NULL) {
    v->not_found = 1;
    return NGX_OK;
  }
  
  v->len  = (unsigned)ctx->host->len;
  v->data = ctx->host->data;
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_filter_google_schema_var(ngx_http_request_t        * r,
                                         ngx_http_variable_value_t * v,
                                         uintptr_t data)
{
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  if (ctx == NULL) {
    v->not_found = 1;
    return NGX_OK;
  }
  
  v->len  = 5;
  v->data = (u_char *)"https";
  
  if (!ctx->ssl) v->len--;
  
  return NGX_OK;
}

static ngx_int_t
ngx_http_google_filter_google_schema_reverse_var(ngx_http_request_t        * r,
                                                 ngx_http_variable_value_t * v,
                                                 uintptr_t data)
{
  ngx_http_google_ctx_t * ctx;
  ctx = ngx_http_get_module_ctx(r, ngx_http_google_filter_module);
  
  if (ctx == NULL) {
    v->not_found = 1;
    return NGX_OK;
  }
  
  v->len  = 5;
  v->data = (u_char *)"https";
  
  if (ctx->ssl) v->len--;
  
  return NGX_OK;
}



