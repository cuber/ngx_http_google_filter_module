Nginx Module for Google
=======================

[![Build Status](https://travis-ci.org/cuber/ngx_http_google_filter_module.svg?branch=dev)](https://travis-ci.org/cuber/ngx_http_google_filter_module)
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/cuber/ngx_http_google_filter_module?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

#### Description ####
`ngx_http_google_filter_module` is a filter module which makes google mirror much easier to deploy.    
Regular expressions, uri locations and other complex configurations have been built-in already.    
The native nginx module ensure the efficiency of handling cookies, gstatic scoures and redirections.   
Let's see how `easy` it is to setup a google mirror.
```nginx
location / {
  google on;
}
```
> _What? Are you kidding me?_   
> _Yes, it's just that simple!_
  
#### Demo site [https://g2.wen.lu](https://g2.wen.lu) ####
![Demo Site](http://ww2.sinaimg.cn/large/68bd1777gw1f13naidonmj212i0najsy.jpg)
  
#### Dependency ####
  1. [`pcre`](http://www.pcre.org/) *regular expression support*
  1. [`ngx_http_proxy_module`](http://nginx.org/en/docs/http/ngx_http_proxy_module.html) *backend proxy support*
  1. [`ngx_http_substitutions_filter_module`](https://github.com/yaoweibin/ngx_http_substitutions_filter_module) *mutiple substitutions support*

#### Installation ####
##### Download sources first #####
```
#
# download the newest source
# @see http://nginx.org/en/download.html
#
wget http://nginx.org/download/nginx-1.7.8.tar.gz

#
# clone ngx_http_google_filter_module
# @see https://github.com/cuber/ngx_http_google_filter_module
#
git clone https://github.com/cuber/ngx_http_google_filter_module

#
# clone ngx_http_substitutions_filter_module
# @see https://github.com/yaoweibin/ngx_http_substitutions_filter_module
#
git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module
```
##### Brand new installation #####
``` bash
#
# configure nginx customly
# replace </path/to/> with your real path
#
./configure \
  <your configuration> \
  --add-module=</path/to/>ngx_http_google_filter_module \
  --add-module=</path/to/>ngx_http_substitutions_filter_module
```

##### Migrate from existed distribution #####
```bash
#
# get the configuration of existed nginx
# replace </path/to/> with your real path
#
</path/to/>nginx -V
> nginx version: nginx/ <version>
> built by gcc 4.x.x
> configure arguments: <configuration>

#
# download the same version of nginx source
# @see http://nginx.org/en/download.html
# replace <version> with your nginx version
#
wget http://nginx.org/download/nginx-<version>.tar.gz
  
#
# configure nginx
# replace <configuration> with your nginx configuration
# replace </path/to/> with your real path
#
./configure \
  <configuration> \
  --add-module=</path/to/>ngx_http_google_filter_module \
  --add-module=</path/to/>ngx_http_substitutions_filter_module
#
# if some libraries were missing, you should install them with the package manager
#   eg. apt-get, pacman, yum ...
#
```

#### Usage ####
##### Basic Configuration #####
  `resolver` is needed to resolve domains.
```nginx
server {
  # ... part of server configuration
  resolver 8.8.8.8;
  location / {
    google on;
  }
  # ...
}
```

##### Google Scholar #####
`google_scholar` depends on `google`, so `google_scholar` cannot be used independently.    
Nowadays google scholar has migrate from `http` to `https`, and `ncr` is supported, so the `tld` of google scholar is no more needed.     
``` nginx
location / {
  google on;
  google_scholar on;
}
```

##### Google Language #####
The default language can be set through `google_language`, if it is not setup, `zh-CN` will be the default language.
```nginx
location / {
  google on;
  google_scholar on;
  # set language to German
  google_language de; 
}
```

Supported languages are listed below.
```txt
ar    -> Arabic
bg    -> Bulgarian
ca    -> Catalan
zh-CN -> Chinese (Simplified)
zh-TW -> Chinese (Traditional)
hr    -> Croatian
cs    -> Czech
da    -> Danish
nl    -> Dutch
en    -> English
tl    -> Filipino
fi    -> Finnish
fr    -> French
de    -> German
el    -> Greek
iw    -> Hebrew
hi    -> Hindi
hu    -> Hungarian
id    -> Indonesian
it    -> Italian
ja    -> Japanese
ko    -> Korean
lv    -> Latvian
lt    -> Lithuanian
no    -> Norwegian
fa    -> Persian
pl    -> Polish
pt-BR -> Portuguese (Brazil)
pt-PT -> Portuguese (Portugal)
ro    -> Romanian
ru    -> Russian
sr    -> Serbian
sk    -> Slovak
sl    -> Slovenian
es    -> Spanish
sv    -> Swedish
th    -> Thai
tr    -> Turkish
uk    -> Ukrainian
vi    -> Vietnamese
```

##### Spider Exclusion #####
The spiders of any search engines are not allowed to crawl google mirror.    
Default `robots.txt` listed below was build-in aleady.
```txt
User-agent: *
Disallow: /
```     
If `google_robots_allow` set to `on`, the `robots.txt` will be replaced with the version of google itself.   
```nginx
  #...
  location / {
    google on;
    google_robots_allow on;
  }
  #...
```

##### Upstreaming #####
`upstream` can help you to avoid name resolving cost, decrease the possibility of google robot detection and proxy through some specific servers.   
``` nginx
upstream www.google.com {
  server 173.194.38.1:443;
  server 173.194.38.2:443;
  server 173.194.38.3:443;
  server 173.194.38.4:443;
}
```

##### Proxy Protocol #####
By default, the proxy will use `https` to communicate with backend servers.      
You can use `google_ssl_off` to force some domains to fall back to `http` protocol.      
It is useful, if you want to proxy some domains through another gateway without ssl certificate.
```nginx
#
# eg. 
# i want to proxy the domain 'www.google.com' like this
# vps(hk) -> vps(us) -> google
#

#
# configuration of vps(hk)
#
server {
  # ...
  location / {
    google on;
    google_ssl_off "www.google.com";
  }
  # ...
}

upstream www.google.com {
  server < ip of vps(us) >:80;
}

#
# configuration of vps(us)
#
server {
  listen 80;
  server_name www.google.com;
  # ...
  location / {
    proxy_pass https://www.google.com;
  }
  # ...
}
```


#### Copyright & License ####
  All codes are under the same [LICENCE](http://nginx.org/LICENSE) with [Nginx](http://nginx.org)    
  Copyright (C) 2014 by Cube.




